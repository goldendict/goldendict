/* This file is (c) 2008-2009 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "xdxf.hh"
#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include "chunkedstorage.hh"
#include "dictzip.h"
#include "htmlescape.hh"
#include "fsencoding.hh"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <zlib.h>
#include <wctype.h>
#include <stdlib.h>

#include <QIODevice>
#include <QXmlStreamReader>
#include <QTextDocument>

#include <QSemaphore>
#include <QThreadPool>
#include <QAtomicInt>

namespace Xdxf {

using std::map;
using std::multimap;
using std::pair;
using std::set;
using std::string;
using gd::wstring;
using std::vector;
using std::list;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

namespace {

DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )
DEF_EX_STR( exNotXdxfFile, "The file is not an XDXF file:", Dictionary::Ex )
DEF_EX( exCorruptedIndex, "The index file is corrupted", Dictionary::Ex )

enum
{
  Signature = 0x46584458, // XDXF on little-endian, FXDX on big-endian
  CurrentFormatVersion = 1 + BtreeIndexing::FormatVersion + Folding::Version
};

enum ArticleFormat
{
  Default = 0,
  Visual = 1,
  Logical = 2
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, XDXF
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  uint32_t articleFormat; // ArticleFormat value, except that 0 = bad file
  char fromLang[ 4 ]; // 3-letter ISO-639.2 language code
  char toLang[ 4 ]; // 3-letter ISO-639.2 language code
  uint32_t articleCount; // Total number of articles
  uint32_t wordCount; // Total number of words
  uint32_t nameAddress; // Address of an utf8 name string, in chunks
  uint32_t nameSize; // And its size
  uint32_t descriptionAddress; // Address of an utf8 description string, in chunks
  uint32_t descriptionSize; // And its size
  uint32_t abbreviationsCount; // Number of abbreviations
  uint32_t chunksOffset; // The offset to chunks' storage
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
} __attribute__((packed));

bool indexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion ||
         !header.articleFormat;
}



class XdxfDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;
  ChunkedStorage::Reader chunks;
  Mutex dzMutex;
  dictData * dz;
  string dictionaryName;

public:

  XdxfDictionary( string const & id, string const & indexFile,
                   vector< string > const & dictionaryFiles );

  ~XdxfDictionary();

  virtual string getName() throw()
  { return dictionaryName; }

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return idxHeader.wordCount; }

  virtual unsigned long getWordCount() throw()
  { return idxHeader.wordCount; }

  virtual QIcon getIcon() throw()
  { return QIcon(":/icons/icon32_xdxf.png"); }

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const & alts )
    throw( std::exception );

private:

  /// Loads the article, storing its headword and formatting the data it has
  /// into an html.
  void loadArticle( uint32_t address,
                    string & headword,
                    string & articleText );

  friend class XdxfArticleRequest;
};

XdxfDictionary::XdxfDictionary( string const & id,
                                string const & indexFile,
                                vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() ),
  chunks( idx, idxHeader.chunksOffset )
{
  // Read the dictionary name

  if ( idxHeader.nameSize )
  {
    vector< char > chunk;

    dictionaryName = string( chunks.getBlock( idxHeader.nameAddress, chunk ),
                             idxHeader.nameSize );
  }

  // Open the file

  dz = dict_data_open( dictionaryFiles[ 0 ].c_str(), 0 );

  if ( !dz )
    throw exCantReadFile( dictionaryFiles[ 0 ] );

  // Initialize the index

  openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
             idx, idxMutex );
}

XdxfDictionary::~XdxfDictionary()
{
  if ( dz )
    dict_data_close( dz );
}

/// XdxfDictionary::getArticle()

class XdxfArticleRequest;

class XdxfArticleRequestRunnable: public QRunnable
{
  XdxfArticleRequest & r;
  QSemaphore & hasExited;

public:

  XdxfArticleRequestRunnable( XdxfArticleRequest & r_,
                                  QSemaphore & hasExited_ ): r( r_ ),
                                                             hasExited( hasExited_ )
  {}

  ~XdxfArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class XdxfArticleRequest: public Dictionary::DataRequest
{
  friend class XdxfArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  XdxfDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  XdxfArticleRequest( wstring const & word_,
                     vector< wstring > const & alts_,
                     XdxfDictionary & dict_ ):
    word( word_ ), alts( alts_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new XdxfArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by XdxfArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~XdxfArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void XdxfArticleRequestRunnable::run()
{
  r.run();
}

void XdxfArticleRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  vector< WordArticleLink > chain = dict.findArticles( word );

  for( unsigned x = 0; x < alts.size(); ++x )
  {
    /// Make an additional query for each alt

    vector< WordArticleLink > altChain = dict.findArticles( alts[ x ] );

    chain.insert( chain.end(), altChain.begin(), altChain.end() );
  }

  multimap< wstring, pair< string, string > > mainArticles, alternateArticles;

  set< uint32_t > articlesIncluded; // Some synonims make it that the articles
                                    // appear several times. We combat this
                                    // by only allowing them to appear once.

  wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    if ( isCancelled )
    {
      finish();
      return;
    }

    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    // Now grab that article

    string headword, articleText;

    dict.loadArticle( chain[ x ].articleOffset, headword, articleText );

    // Ok. Now, does it go to main articles, or to alternate ones? We list
    // main ones first, and alternates after.

    // We do the case-folded comparison here.

    wstring headwordStripped =
      Folding::applySimpleCaseOnly( Utf8::decode( headword ) );

    multimap< wstring, pair< string, string > > & mapToUse =
      ( wordCaseFolded == headwordStripped ) ?
        mainArticles : alternateArticles;

    mapToUse.insert( pair< wstring, pair< string, string > >(
      Folding::applySimpleCaseOnly( Utf8::decode( headword ) ),
      pair< string, string >( headword, articleText ) ) );

    articlesIncluded.insert( chain[ x ].articleOffset );
  }

  if ( mainArticles.empty() && alternateArticles.empty() )
  {
    // No such word
    finish();
    return;
  }

  string result;

  multimap< wstring, pair< string, string > >::const_iterator i;

  string cleaner = "</font>""</font>""</font>""</font>""</font>""</font>"
                   "</font>""</font>""</font>""</font>""</font>""</font>"
                   "</b></b></b></b></b></b></b></b>"
                   "</i></i></i></i></i></i></i></i>";

  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
  {
      result += "<h3>";
      result += i->second.first;
      result += "</h3>";
      result += i->second.second;
      result += cleaner;
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
      result += "<h3>";
      result += i->second.first;
      result += "</h3>";
      result += i->second.second;
      result += cleaner;
  }

  Mutex::Lock _( dataMutex );

  data.resize( result.size() );

  memcpy( &data.front(), result.data(), result.size() );

  hasAnyData = true;

  finish();
}

sptr< Dictionary::DataRequest > XdxfDictionary::getArticle( wstring const & word,
                                                            vector< wstring > const & alts )
  throw( std::exception )
{
  return new XdxfArticleRequest( word, alts, *this );
}

void XdxfDictionary::loadArticle( uint32_t address,
                                  string & headword,
                                  string & articleText )
{
  // Read the properties

  vector< char > chunk;

  char * propertiesData;

  {
    Mutex::Lock _( idxMutex );
  
    propertiesData = chunks.getBlock( address, chunk );
  }

  if ( &chunk.front() + chunk.size() - propertiesData < 9 )
    throw exCorruptedIndex();

  unsigned char fType = (unsigned char) *propertiesData;

  uint32_t articleOffset, articleSize;

  memcpy( &articleOffset, propertiesData + 1, sizeof( uint32_t ) );
  memcpy( &articleSize, propertiesData + 5, sizeof( uint32_t ) );

  // Load the article

  char * articleBody;

  {
    Mutex::Lock _( dzMutex );

    // Note that the function always zero-pads the result.
    articleBody = dict_data_read_( dz, articleOffset, articleSize, 0, 0 );
  }

  if ( !articleBody )
    throw exCantReadFile( getDictionaryFilenames()[ 0 ] );

  articleText = Html::escape( articleBody );

  headword = "foo";

  free( articleBody );
}

class GzippedFile: public QIODevice
{
  gzFile gz;

public:

  GzippedFile( char const * fileName ) throw( exCantReadFile );

  ~GzippedFile();

  size_t gzTell();

protected:

  virtual bool isSequential () const
  { return false; } // Which is a lie, but else pos() won't work

  bool waitForReadyRead ( int )
  { return !gzeof( gz ); }

 qint64 bytesAvailable() const
 {
     return ( gzeof( gz ) ? 0 : 1 ) + QIODevice::bytesAvailable();
 }

  virtual qint64 readData( char * data, qint64 maxSize );

  virtual bool atEnd();

  virtual qint64 writeData ( const char * /*data*/, qint64 /*maxSize*/ )
  { return -1; }
};

GzippedFile::GzippedFile( char const * fileName ) throw( exCantReadFile )
{
  gz = gzopen( fileName, "rb" );

  if ( !gz )
    throw exCantReadFile( fileName );
}

GzippedFile::~GzippedFile()
{
  gzclose( gz );
}

bool GzippedFile::atEnd()
{
  return gzeof( gz );
}

size_t GzippedFile::gzTell()
{
  return gztell( gz );
}

qint64 GzippedFile::readData( char * data, qint64 maxSize )
{
  if ( maxSize > 1 )
    maxSize = 1;

  // The returning value translates directly to QIODevice semantics
  return gzread( gz, data, maxSize );
}

QString readXhtmlData( QXmlStreamReader & stream )
{
  QString result;

  while( !stream.atEnd() )
  {
    stream.readNext();

    if ( stream.isStartElement() )
    {
      QString name = stream.name().toString();

      result += "<" + Qt::escape( name ) + " ";

      QXmlStreamAttributes attrs = stream.attributes();

      for( int x = 0; x < attrs.size(); ++x )
      {
        result += Qt::escape( attrs[ x ].name().toString() );
        result += "=\"" + Qt::escape( attrs[ x ].value().toString() ) + "\"";
      }

      result += ">";

      result += readXhtmlData( stream );

      result += "</" + Qt::escape( name ) + ">";
    }
    else
    if ( stream.isCharacters() || stream.isWhitespace() || stream.isCDATA() )
    {
      result += stream.text();
    }
    else
    if ( stream.isEndElement() )
      break;
  }

  return result;
}

void addAllKeyTags( QXmlStreamReader & stream, list< QString > & words )
{
  if ( stream.name() == "k" )
  {
    words.push_back( stream.readElementText() );
    return;
  }

  for( ; ; )
  {
    stream.readNext();
  
    if ( stream.isStartElement() )
      addAllKeyTags( stream, words );
    else
    if ( stream.isEndElement() )
      return;
  }
}

void indexArticle( GzippedFile & gzFile,
                   QXmlStreamReader & stream,
                   IndexedWords & indexedWords,
                   ChunkedStorage::Writer & chunks,
                   unsigned & articleCount,
                   unsigned & wordCount )
{
  ArticleFormat format( Default );

  QStringRef formatValue = stream.attributes().value( "f" );

  if ( formatValue == "v" )
    format = Visual;
  else
  if ( formatValue == "l" )
    format = Logical;

  size_t articleOffset = gzFile.pos() - 1; // stream.characterOffset() is loony

  uint32_t lineNumber = stream.lineNumber();
  uint32_t columnNumber = stream.columnNumber();

  list< QString > words;

  while( !stream.atEnd() )
  {
    stream.readNext();

    // Find any <k> tags and index them
    if ( stream.isEndElement() )
    {
      // End of the <ar> tag

      if ( words.empty() )
      {
        // Nothing to index, this article didn't have any tags
        printf( "Warning: no <k> tags found in an article at offset 0x%x, article skipped.\n",
                (unsigned) articleOffset );
      }
      else
      {
        // Add an entry

        uint32_t offset = chunks.startNewBlock();

        uint32_t value = articleOffset;

        unsigned char f = format;
        chunks.addToBlock( &f, 1 );
        chunks.addToBlock( &value, sizeof( value ) );
        value = gzFile.pos() - 1 - articleOffset;
        chunks.addToBlock( &value, sizeof( value ) );

        printf( "%x: %s\n", articleOffset, words.begin()->toUtf8().data() );

        // Add words to index

        for( list< QString >::const_iterator i = words.begin(); i != words.end();
             ++i )
          indexedWords.addWord( i->toStdWString(), offset );

        ++articleCount;

        wordCount += words.size();
      }

      return;
    }
    else
    if ( stream.isStartElement() )
    {
      addAllKeyTags( stream, words );
    }
  }
}

} // anonymous namespace

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & initializing )
  throw( std::exception )
{
  vector< sptr< Dictionary::Class > > dictionaries;

  for( vector< string >::const_iterator i = fileNames.begin(); i != fileNames.end();
       ++i )
  {
    // Only allow .xdxf and .xdxf.dz suffixes

    if ( ( i->size() < 5  || strcasecmp( i->c_str() + ( i->size() - 5 ), ".xdxf" ) != 0 ) &&
         ( i->size() < 8 ||
           strcasecmp( i->c_str() + ( i->size() - 8 ), ".xdxf.dz" ) != 0 ) )
      continue;

    try
    {
      vector< string > dictFiles( 1, *i );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        // Building the index

        //initializing.indexingDictionary( nameFromFileName( dictFiles[ 0 ] ) );

        File::Class idx( indexFile, "wb" );

        IdxHeader idxHeader;

        memset( &idxHeader, 0, sizeof( idxHeader ) );

        // We write a dummy header first. At the end of the process the header
        // will be rewritten with the right values.

        idx.write( idxHeader );

        IndexedWords indexedWords;

        GzippedFile gzFile( dictFiles[ 0 ].c_str() );

        if ( !gzFile.open( QIODevice::ReadOnly ) )
          throw exCantReadFile( dictFiles[ 0 ] );

        QXmlStreamReader stream( &gzFile );

        QString dictionaryName, dictionaryDescription;

        ChunkedStorage::Writer chunks( idx );

        // Wait for the first element, which must be xdxf

        bool hadXdxf = false;

        while( !stream.atEnd() )
        {
          stream.readNext();

          if ( stream.isStartElement() )
          {
            if ( stream.name() != "xdxf" )
              throw exNotXdxfFile( dictFiles[ 0 ] );
            else
            {
              // Read the xdxf

              string str = stream.attributes().value( "lang_from" ).toString().toAscii().data();

              if ( str.size() > 3 )
                str.resize( 3 );

              strcpy( idxHeader.fromLang, str.c_str() );

              str = stream.attributes().value( "lang_to" ).toString().toAscii().data();

              if ( str.size() > 3 )
                str.resize( 3 );

              strcpy( idxHeader.toLang, str.c_str() );

              bool isLogical = ( stream.attributes().value( "format" ) == "logical" );

              idxHeader.articleFormat = isLogical ? Logical : Visual;

              unsigned articleCount = 0, wordCount = 0;

              while( !stream.atEnd() )
              {
                stream.readNext();

                if ( stream.isStartElement() )
                {
                  if ( stream.name() == "full_name" )
                  {
                    // That's our name

                    QString name = stream.readElementText();

                    if ( dictionaryName.isEmpty() )
                    {
                      dictionaryName = name;

                      initializing.indexingDictionary( dictionaryName.toUtf8().data() );

                      idxHeader.nameAddress = chunks.startNewBlock();

                      QByteArray n = dictionaryName.toUtf8();

                      idxHeader.nameSize = n.size();

                      chunks.addToBlock( n.data(), n.size() );
                    }
                    else
                      printf( "Warning: duplicate full_name in %s\n", dictFiles[ 0 ].c_str() );
                  }
                  else
                  if ( stream.name() == "description" )
                  {
                    QString desc = readXhtmlData( stream );

                    if ( dictionaryDescription.isEmpty() )
                    {
                      dictionaryDescription = desc;
                      idxHeader.descriptionAddress = chunks.startNewBlock();

                      QByteArray n = dictionaryDescription.toUtf8();

                      idxHeader.descriptionSize = n.size();

                      chunks.addToBlock( n.data(), n.size() );
                    }
                    else
                      printf( "Warning: duplicate description in %s\n", dictFiles[ 0 ].c_str() );
                  }
                  else
                  if ( stream.name() == "ar" )
                  {
                    indexArticle( gzFile, stream, indexedWords, chunks,
                                  articleCount, wordCount );
                  }
                }
              }

              // Finish with the chunks

              idxHeader.chunksOffset = chunks.finish();

              // Build index

              IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

              idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
              idxHeader.indexRootOffset = idxInfo.rootOffset;

              // That concludes it. Update the header.

              idxHeader.signature = Signature;
              idxHeader.formatVersion = CurrentFormatVersion;

              idx.rewind();

              idx.write( &idxHeader, sizeof( idxHeader ) );

              hadXdxf = true;
            }
            break;
          }
        }

        if ( !hadXdxf )
          throw exNotXdxfFile( dictFiles[ 0 ] );

        if ( stream.hasError() )
        {
          printf( "Warning: %s had a parse error %ls, and therefore was indexed only up to the point of error.\n",
                  dictFiles[ 0 ].c_str(), stream.errorString().toStdWString().c_str() );
        }
      }

      dictionaries.push_back( new XdxfDictionary( dictId,
                                                  indexFile,
                                                  dictFiles ) );
    }
    catch( std::exception & e )
    {
      fprintf( stderr, "Xdxf dictionary reading failed: %s, error: %s\n",
        i->c_str(), e.what() );
    }
  }

  return dictionaries;
}

}
