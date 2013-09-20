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
#include <wctype.h>
#include <stdlib.h>
#include "dprintf.hh"
#include "wstring_qt.hh"
#include "xdxf2html.hh"
#include "ufile.hh"
#include "dictzip.h"
#include "langcoder.hh"
#include "indexedzip.hh"
#include "filetype.hh"

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QIODevice>
#include <QXmlStreamReader>
#include <QTextDocument>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QDebug>

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
  CurrentFormatVersion = 4 + BtreeIndexing::FormatVersion + Folding::Version
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
  uint32_t langFrom; // Source language
  uint32_t langTo;   // Target language
  uint32_t articleCount; // Total number of articles
  uint32_t wordCount; // Total number of words
  uint32_t nameAddress; // Address of an utf8 name string, in chunks
  uint32_t nameSize; // And its size
  uint32_t descriptionAddress; // Address of an utf8 description string, in chunks
  uint32_t descriptionSize; // And its size
  uint32_t hasAbrv; // Non-zero means file has abrvs at abrvAddress
  uint32_t abrvAddress; // Address of abrv map in the chunked storage
  uint32_t chunksOffset; // The offset to chunks' storage
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
  uint32_t hasZipFile; // Non-zero means there's a zip file with resources
                       // present
  uint32_t zipIndexBtreeMaxElements; // Two fields from IndexInfo of the zip
                                     // resource index.
  uint32_t zipIndexRootOffset;
  uint32_t revisionNumber; // Format revision 
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

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
  sptr< ChunkedStorage::Reader > chunks;
  Mutex dzMutex;
  dictData * dz;
  Mutex resourceZipMutex;
  IndexedZip resourceZip;
  string dictionaryName;
  map< string, string > abrv;

public:

  XdxfDictionary( string const & id, string const & indexFile,
                   vector< string > const & dictionaryFiles );

  ~XdxfDictionary();

  virtual string getName() throw()
  { return dictionaryName; }

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return idxHeader.articleCount; }

  virtual unsigned long getWordCount() throw()
  { return idxHeader.wordCount; }

  inline virtual quint32 getLangFrom() const
  { return idxHeader.langFrom; }

  inline virtual quint32 getLangTo() const
  { return idxHeader.langTo; }

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const & alts,
                                                      wstring const & )
    throw( std::exception );

  virtual sptr< Dictionary::DataRequest > getResource( string const & name )
    throw( std::exception );

  virtual QString const& getDescription();

  virtual QString getMainFilename();

protected:

  void loadIcon() throw();

private:

  // Loads the article, storing its headword and formatting article's data into an html.
  void loadArticle( uint32_t address,
                    string & articleText );

  friend class XdxfArticleRequest;
  friend class XdxfResourceRequest;
};

XdxfDictionary::XdxfDictionary( string const & id,
                                string const & indexFile,
                                vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() )
{
  // Read the dictionary name

  chunks = new ChunkedStorage::Reader( idx, idxHeader.chunksOffset );

  if ( idxHeader.nameSize )
  {
    vector< char > chunk;

    dictionaryName = string( chunks->getBlock( idxHeader.nameAddress, chunk ),
                             idxHeader.nameSize );
  }

  // Open the file

  dz = dict_data_open( dictionaryFiles[ 0 ].c_str(), 0 );

  if ( !dz )
    throw exCantReadFile( dictionaryFiles[ 0 ] );

  // Read the abrv, if any

  if ( idxHeader.hasAbrv )
  {
    vector< char > chunk;

    char * abrvBlock = chunks->getBlock( idxHeader.abrvAddress, chunk );

    uint32_t total;
    memcpy( &total, abrvBlock, sizeof( uint32_t ) );
    abrvBlock += sizeof( uint32_t );

    while( total-- )
    {
      uint32_t keySz;
      memcpy( &keySz, abrvBlock, sizeof( uint32_t ) );
      abrvBlock += sizeof( uint32_t );

      char * key = abrvBlock;

      abrvBlock += keySz;

      uint32_t valueSz;
      memcpy( &valueSz, abrvBlock, sizeof( uint32_t ) );
      abrvBlock += sizeof( uint32_t );

      abrv[ string( key, keySz ) ] = string( abrvBlock, valueSz );

      abrvBlock += valueSz;
    }

    // Open a resource zip file, if there's one

    if ( idxHeader.hasZipFile &&
         ( idxHeader.zipIndexBtreeMaxElements ||
           idxHeader.zipIndexRootOffset ) )
    {
      resourceZip.openIndex( IndexInfo( idxHeader.zipIndexBtreeMaxElements,
                                        idxHeader.zipIndexRootOffset ),
                             idx, idxMutex );

      QString zipName = QDir::fromNativeSeparators(
          FsEncoding::decode( getDictionaryFilenames().back().c_str() ) );

      if ( zipName.endsWith( ".zip", Qt::CaseInsensitive ) ) // Sanity check
        resourceZip.openZipFile( zipName );
    }
  }

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

void XdxfDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  QFileInfo baseInfo( fileName );

  fileName = baseInfo.absoluteDir().absoluteFilePath( "icon32.png" );
  QFileInfo info( fileName );

  if( !info.isFile() )
  {
      fileName = baseInfo.absoluteDir().absoluteFilePath( "icon16.png" );
      info = QFileInfo( fileName );
  }

  if ( info.isFile() )
    loadIconFromFile( fileName, true );

  if ( dictionaryIcon.isNull() )
  {
    // Load failed -- use default icons

    dictionaryIcon = QIcon(":/icons/icon32_xdxf.png");
    dictionaryNativeIcon = QIcon(":/icons/icon32_xdxf.png");
  }

  dictionaryIconLoaded = true;
}

QString const& XdxfDictionary::getDescription()
{
    if( !dictionaryDescription.isEmpty() )
        return dictionaryDescription;

    if( idxHeader.descriptionAddress == 0 )
        dictionaryDescription = "NONE";
    else
    {
        try
        {
            vector< char > chunk;
            char * descr;
            {
              Mutex::Lock _( idxMutex );
              descr = chunks->getBlock( idxHeader.descriptionAddress, chunk );
            }
            dictionaryDescription = QString::fromUtf8( descr );
        }
        catch(...)
        {
        }
    }
    return dictionaryDescription;
}

QString XdxfDictionary::getMainFilename()
{
  return FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() );
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

    headword = chain[ x ].word;
    dict.loadArticle( chain[ x ].articleOffset, articleText );

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
//      result += "<h3>";
//      result += i->second.first;
//      result += "</h3>";
      result += i->second.second;
      result += cleaner;
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
//      result += "<h3>";
//      result += i->second.first;
//      result += "</h3>";
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
                                                            vector< wstring > const & alts,
                                                            wstring const & )
  throw( std::exception )
{
  return new XdxfArticleRequest( word, alts, *this );
}

void XdxfDictionary::loadArticle( uint32_t address,
                                  string & articleText )
{
  // Read the properties

  vector< char > chunk;

  char * propertiesData;

  {
    Mutex::Lock _( idxMutex );
  
    propertiesData = chunks->getBlock( address, chunk );
  }

  if ( &chunk.front() + chunk.size() - propertiesData < 9 )
  {
    articleText = string( "<div class=\"xdxf\">Index seems corrupted</div>" );
    return;
  }

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
  {
//    throw exCantReadFile( getDictionaryFilenames()[ 0 ] );
      articleText = string( "<div class=\"xdxf\">DICTZIP error: " ) + dict_error_str( dz ) + "</div>";
    return;
  }

  articleText = Xdxf2Html::convert( string( articleBody ), Xdxf2Html::XDXF, idxHeader.hasAbrv ? &abrv : NULL, this,
                                    fType == Logical, idxHeader.revisionNumber );

  free( articleBody );
}

class GzippedFile: public QIODevice
{
  gzFile gz;

public:

  GzippedFile( char const * fileName ) throw( exCantReadFile );

  ~GzippedFile();

  size_t gzTell();

  char * readDataArray( unsigned long startPos, unsigned long size );

protected:

  dictData *dz;

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
  gz = gd_gzopen( fileName );
  if ( !gz )
    throw exCantReadFile( fileName );

  dz = dict_data_open( fileName, 0 );

}

GzippedFile::~GzippedFile()
{
  gzclose( gz );
  if( dz )
      dict_data_close( dz );
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

char * GzippedFile::readDataArray( unsigned long startPos, unsigned long size )
{
  if( dz == NULL )
      return NULL;
  return dict_data_read_( dz, startPos, size, 0, 0 );
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

namespace {

/// Deal with Qt 4.5 incompatibility
QString readElementText( QXmlStreamReader & stream )
{
#if QT_VERSION >= 0x040600
    return stream.readElementText( QXmlStreamReader::SkipChildElements );
#else
    return stream.readElementText();
#endif
}

}


void addAllKeyTags( QXmlStreamReader & stream, list< QString > & words )
{
  // todo implement support for tag <srt>, that overrides the article sorting order 
  if ( stream.name() == "k" )
  {
    words.push_back( readElementText( stream ) );
    return;
  }

  while( !stream.atEnd() )
  {
    stream.readNext();
  
    if ( stream.isStartElement() )
      addAllKeyTags( stream, words );
    else
    if ( stream.isEndElement() )
      return;
  }
}

void checkArticlePosition( GzippedFile & gzFile,
                           uint32_t *pOffset,
                           uint32_t *pSize )
{
    char * data = gzFile.readDataArray( *pOffset, *pSize );
    if( data == NULL )
        return;
    QString s = QString::fromUtf8( data );
    free( data );
    int n = s.lastIndexOf( "</ar" );
    if( n > 0 )
        *pSize -= s.size() - n;
    if( s.at( 0 ) == '>')
    {
        *pOffset += 1;
        *pSize -= 1;
    }
}

void indexArticle( GzippedFile & gzFile,
                   QXmlStreamReader & stream,
                   IndexedWords & indexedWords,
                   ChunkedStorage::Writer & chunks,
                   unsigned & articleCount,
                   unsigned & wordCount,
                   ArticleFormat defaultFormat )
{
  ArticleFormat format( Default );

  QStringRef formatValue = stream.attributes().value( "f" );

  if ( formatValue == "v" )
    format = Visual;
  else
  if ( formatValue == "l" )
    format = Logical;
  if( format == Default )
    format = defaultFormat; 
  size_t articleOffset = gzFile.pos() - 1; // stream.characterOffset() is loony

  // uint32_t lineNumber = stream.lineNumber();
  // uint32_t columnNumber = stream.columnNumber();

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
        qWarning( "Warning: no <k> tags found in an article at offset 0x%x, article skipped.\n",
                  (unsigned) articleOffset );
      }
      else
      {
        // Add an entry

        uint32_t offset = chunks.startNewBlock();

        uint32_t offs = articleOffset;
        uint32_t size = gzFile.pos() - 1 - articleOffset;

        checkArticlePosition( gzFile, &offs, &size );

        unsigned char f = format;
        chunks.addToBlock( &f, 1 );
        chunks.addToBlock( &offs, sizeof( offs ) );
        chunks.addToBlock( &size, sizeof( size ) );

//        DPRINTF( "%x: %s\n", articleOffset, words.begin()->toUtf8().data() );

        // Add words to index

        for( list< QString >::const_iterator i = words.begin(); i != words.end();
             ++i )
            indexedWords.addWord( gd::toWString( *i ), offset );

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

//// XdxfDictionary::getResource()

class XdxfResourceRequest;

class XdxfResourceRequestRunnable: public QRunnable
{
  XdxfResourceRequest & r;
  QSemaphore & hasExited;

public:

  XdxfResourceRequestRunnable( XdxfResourceRequest & r_,
                               QSemaphore & hasExited_ ): r( r_ ),
                                                          hasExited( hasExited_ )
  {}

  ~XdxfResourceRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class XdxfResourceRequest: public Dictionary::DataRequest
{
  friend class XdxfResourceRequestRunnable;

  XdxfDictionary & dict;

  string resourceName;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  XdxfResourceRequest( XdxfDictionary & dict_,
                      string const & resourceName_ ):
    dict( dict_ ),
    resourceName( resourceName_ )
  {
    QThreadPool::globalInstance()->start(
      new XdxfResourceRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by XdxfResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~XdxfResourceRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void XdxfResourceRequestRunnable::run()
{
  r.run();
}

void XdxfResourceRequest::run()
{
  // Some runnables linger enough that they are cancelled before they start
  if ( isCancelled )
  {
    finish();
    return;
  }

  if ( dict.ensureInitDone().size() )
  {
    setErrorString( QString::fromUtf8( dict.ensureInitDone().c_str() ) );
    finish();
    return;
  }

  string n =
    FsEncoding::dirname( dict.getDictionaryFilenames()[ 0 ] ) +
    FsEncoding::separator() +
    FsEncoding::encode( resourceName );

  DPRINTF( "n is %s\n", n.c_str() );

  try
  {
    try
    {
      Mutex::Lock _( dataMutex );

      File::loadFromFile( n, data );
    }
    catch( File::exCantOpen & )
    {
      n = dict.getDictionaryFilenames()[ 0 ] + ".files" +
          FsEncoding::separator() +
          FsEncoding::encode( resourceName );

      try
      {
        Mutex::Lock _( dataMutex );

        File::loadFromFile( n, data );
      }
      catch( File::exCantOpen & )
      {
        // Try reading from zip file

        if ( dict.resourceZip.isOpen() )
        {
          Mutex::Lock _( dict.resourceZipMutex );

          Mutex::Lock __( dataMutex );

          if ( !dict.resourceZip.loadFile( Utf8::decode( resourceName ), data ) )
            throw; // Make it fail since we couldn't read the archive
        }
        else
          throw;
      }
    }

    if ( Filetype::isNameOfTiff( resourceName ) )
    {
      // Convert it

      dataMutex.lock();

      QImage img = QImage::fromData( (unsigned char *) &data.front(),
                                     data.size() );

      dataMutex.unlock();

      if ( !img.isNull() )
      {
        // Managed to load -- now store it back as BMP

        QByteArray ba;
        QBuffer buffer( &ba );
        buffer.open( QIODevice::WriteOnly );
        img.save( &buffer, "BMP" );

        Mutex::Lock _( dataMutex );

        data.resize( buffer.size() );

        memcpy( &data.front(), buffer.data(), data.size() );
      }
    }

    Mutex::Lock _( dataMutex );

    hasAnyData = true;
  }
  catch( File::Ex & )
  {
    // No such resource -- we don't set the hasAnyData flag then
  }
  catch( Utf8::exCantDecode )
  {
    // Failed to decode some utf8 -- probably the resource name is no good
  }

  finish();
}

sptr< Dictionary::DataRequest > XdxfDictionary::getResource( string const & name )
  throw( std::exception )
{
  return new XdxfResourceRequest( *this, name );
}

}
// anonymous namespace - this section of file is devoted to rebuilding of dictionary articles index

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

      string baseName = ( (*i)[ i->size() - 5 ] == '.' ) ?
               string( *i, 0, i->size() - 5 ) : string( *i, 0, i->size() - 8 );

      // See if there's a zip file with resources present. If so, include it.

      string zipFileName;

      if ( File::tryPossibleName( baseName + ".xdxf.files.zip", zipFileName ) ||
           File::tryPossibleName( baseName + ".xdxf.dz.files.zip", zipFileName ) ||
           File::tryPossibleName( baseName + ".XDXF.FILES.ZIP", zipFileName ) ||
           File::tryPossibleName( baseName + ".XDXF.DZ.FILES.ZIP", zipFileName ) )
        dictFiles.push_back( zipFileName );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        // Building the index

        qDebug( "Xdxf: Building the index for dictionary: %s\n", i->c_str() );

        //initializing.indexingDictionary( nameFromFileName( dictFiles[ 0 ] ) );

        File::Class idx( indexFile, "wb" );

        IdxHeader idxHeader;
        map< string, string > abrv;

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

              string str = stream.attributes().value( "lang_from" ).toString().toLatin1().data();

              if ( str.size() > 3 )
                str.resize( 3 );

              idxHeader.langFrom = LangCoder::findIdForLanguageCode3( str.c_str() );

              str = stream.attributes().value( "lang_to" ).toString().toLatin1().data();

              if ( str.size() > 3 )
                str.resize( 3 );

              idxHeader.langTo = LangCoder::findIdForLanguageCode3( str.c_str() );

              bool isLogical = ( stream.attributes().value( "format" ) == "logical" );
              idxHeader.revisionNumber = stream.attributes().value( "revision" ).toString().toUInt();

              idxHeader.articleFormat = isLogical ? Logical : Visual;

              unsigned articleCount = 0, wordCount = 0;

              while( !stream.atEnd() )
              {
                stream.readNext();

                if ( stream.isStartElement() )
                {
                  // todo implement using short <title> for denoting the dictionary in settings or dict list toolbar
                  if ( stream.name() == "full_name" || stream.name() == "full_title" )
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
                    {
                      DPRINTF( "Warning: duplicate full_name in %s\n", dictFiles[ 0 ].c_str() );
                    }
                  }
                  else
                  if ( stream.name() == "description" )
                  {
                    // todo implement adding other information to the description like <publisher>, <authors>, <file_ver>, <creation_date>, <last_edited_date>, <dict_edition>, <publishing_date>, <dict_src_url> 
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
                    {
                      DPRINTF( "Warning: duplicate description in %s\n", dictFiles[ 0 ].c_str() );
                    }
                  }
                  else
                  if ( stream.name() == "abbreviations" )
                  {
                    QString s;
                    string value;
                    list < wstring > keys;
                    while( !( stream.isEndElement() && stream.name() == "abbreviations" ) && !stream.atEnd() )
                    {
                      stream.readNext();
                      // abbreviations tag set switch at format revision = 30 
                      if( idxHeader.revisionNumber >= 30 )
                      {
                        while ( !( stream.isEndElement() && stream.name() == "abbr_def" ) || !stream.atEnd() )
                        {
                          stream.readNext();
                          if ( stream.isStartElement() && stream.name() == "abbr_k" )
                          {
                            s = readElementText( stream );
                            keys.push_back( gd::toWString( s ) );
                          }
                          else if ( stream.isStartElement() && stream.name() == "abbr_v" )
                          {
                            s =  readElementText( stream );
                              value = Utf8::encode( Folding::trimWhitespace( gd::toWString( s ) ) );
                              for( list< wstring >::iterator i = keys.begin(); i != keys.end(); ++i )
                              {
                                abrv[ Utf8::encode( Folding::trimWhitespace( *i ) ) ] = value;
                              }
                              keys.clear();
                          }
                          else if ( stream.isEndElement() && stream.name() == "abbreviations" )
                            break;
                        }
                      }
                      else
                      {
                        while ( !( stream.isEndElement() && stream.name() == "abr_def" ) || !stream.atEnd() )
                        {
                          stream.readNext();
                          if ( stream.isStartElement() && stream.name() == "k" )
                          {
                            s = readElementText( stream );
                            keys.push_back( gd::toWString( s ) );
                          }
                          else if ( stream.isStartElement() && stream.name() == "v" )
                          {
                            s =  readElementText( stream );
                              value = Utf8::encode( Folding::trimWhitespace( gd::toWString( s ) ) );
                              for( list< wstring >::iterator i = keys.begin(); i != keys.end(); ++i )
                              {
                                abrv[ Utf8::encode( Folding::trimWhitespace( *i ) ) ] = value;
                              }
                              keys.clear();
                          }
                          else if ( stream.isEndElement() && stream.name() == "abbreviations" )
                            break;
                        }
                      }
                    }
                  }
                  else
                  if ( stream.name() == "ar" )
                  {
                    indexArticle( gzFile, stream, indexedWords, chunks,
                                  articleCount, wordCount, isLogical ? Logical : Visual );
                  }
                }
              }

              // Write abbreviations if presented

              if( !abrv.empty() )
              {
                idxHeader.hasAbrv = 1;
                idxHeader.abrvAddress = chunks.startNewBlock();

                uint32_t sz = abrv.size();

                chunks.addToBlock( &sz, sizeof( uint32_t ) );

                for( map< string, string >::const_iterator i = abrv.begin();  i != abrv.end(); ++i )
                {
                  sz = i->first.size();
                  chunks.addToBlock( &sz, sizeof( uint32_t ) );
                  chunks.addToBlock( i->first.data(), sz );
                  sz = i->second.size();
                  chunks.addToBlock( &sz, sizeof( uint32_t ) );
                  chunks.addToBlock( i->second.data(), sz );
                }
              }

              // Finish with the chunks

              idxHeader.chunksOffset = chunks.finish();

              // Build index

              IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

              idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
              idxHeader.indexRootOffset = idxInfo.rootOffset;

              indexedWords.clear(); // Release memory -- no need for this data

              // If there was a zip file, index it too

              if ( zipFileName.size() )
              {
                DPRINTF( "Indexing zip file\n" );

                idxHeader.hasZipFile = 1;

                IndexedWords zipFileNames;
                IndexedZip zipFile;
                if( zipFile.openZipFile( QDir::fromNativeSeparators(
                                         FsEncoding::decode( zipFileName.c_str() ) ) ) )
                    zipFile.indexFile( zipFileNames );

                if( !zipFileNames.empty() )
                {
                  // Build the resulting zip file index

                  IndexInfo idxInfo = BtreeIndexing::buildIndex( zipFileNames, idx );

                  idxHeader.zipIndexBtreeMaxElements = idxInfo.btreeMaxElements;
                  idxHeader.zipIndexRootOffset = idxInfo.rootOffset;
                }
                else
                {
                  // Bad zip file -- no index (though the mark that we have one
                  // remains)
                  idxHeader.zipIndexBtreeMaxElements = 0;
                  idxHeader.zipIndexRootOffset = 0;
                }
              }
              else
                idxHeader.hasZipFile = 0;
              // That concludes it. Update the header.

              idxHeader.signature = Signature;
              idxHeader.formatVersion = CurrentFormatVersion;

              idxHeader.articleCount = articleCount;
              idxHeader.wordCount = wordCount;

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
          qWarning( "Warning: %s had a parse error %ls at line %lu, and therefore was indexed only up to the point of error.",
                     dictFiles[ 0 ].c_str(), stream.errorString().toStdWString().c_str(),
                     (unsigned long) stream.lineNumber() );
        }
      }

      dictionaries.push_back( new XdxfDictionary( dictId,
                                                  indexFile,
                                                  dictFiles ) );
    }
    catch( std::exception & e )
    {
      qWarning( "Xdxf dictionary reading failed: %s, error: %s\n",
                i->c_str(), e.what() );
    }
  }

  return dictionaries;
}

}
