/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "sdict.hh"
#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include "chunkedstorage.hh"
#include "langcoder.hh"
#include "dprintf.hh"
#include "fsencoding.hh"

#include <zlib.h>
#include <bzlib.h>
#include <map>
#include <set>
#include <string>
#include <stdlib.h>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QString>
#include <QSemaphore>
#include <QThreadPool>
#include <QAtomicInt>
#include <QDomDocument>

#include "ufile.hh"

namespace Sdict {

using std::map;
using std::multimap;
using std::pair;
using std::set;
using std::string;
using gd::wstring;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

namespace {

DEF_EX_STR( exNotDctFile, "Not an Sdictionary file", Dictionary::Ex )
DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )
DEF_EX_STR( exWordIsTooLarge, "Enountered a word that is too large:", Dictionary::Ex )
DEF_EX_STR( exSuddenEndOfFile, "Sudden end of file", Dictionary::Ex )

#ifdef _MSC_VER
#pragma pack( push, 1 )
#endif

/// DCT file header
struct DCT_header
{
    char signature[4];
    char inputLang[3];
    char outputLang[3];
    uint8_t compression;
    uint32_t wordCount;
    uint32_t shortIndexLength;
    uint32_t titleOffset;
    uint32_t copyrightOffset;
    uint32_t versionOffset;
    uint32_t shortIndexOffset;
    uint32_t fullIndexOffset;
    uint32_t articlesOffset;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct IndexElement
{
    uint16_t nextWord;
    uint16_t previousWord;
    uint32_t articleOffset;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

enum
{
  Signature = 0x43494453, // SDIC on little-endian, CIDS on big-endian
  CurrentFormatVersion = 1 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, SDIC
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  uint32_t chunksOffset; // The offset to chunks' storage
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
  uint32_t wordCount;
  uint32_t articleCount;
  uint32_t compressionType; // Data compression in file. 0 - no compression, 1 - zip, 2 - bzip2
  uint32_t langFrom;  // Source language
  uint32_t langTo;    // Target language
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

#ifdef _MSC_VER
#pragma pack( pop, 1 )
#endif

bool indexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion;
}

string decompressZlib( char * bufptr, unsigned length )
{
z_stream zs;
char buf[2048];
string str;
int res;
    memset( &zs, 0, sizeof(zs) );
    zs.next_in = (Bytef *)bufptr;
    zs.avail_in = length;
    while( 1 )
    {
        res = inflateInit( &zs );
        if( res != Z_OK )
            break;
        while( res != Z_STREAM_END )
        {
            memset( buf, 0, sizeof(buf) );
            zs.next_out = (Bytef *)buf;
            zs.avail_out = 2047;
            res = inflate( &zs, Z_SYNC_FLUSH );
            str += buf;
            if( res != Z_OK && res != Z_STREAM_END )
                 break;
        }
        break;
    }
    inflateEnd( &zs );
    if( res != Z_STREAM_END )
        str.clear();
    return str;
}

string decompressBzip2( char * bufptr, unsigned length )
{
bz_stream zs;
char buf[2048];
string str;
int res;
    memset( &zs, 0, sizeof(zs) );
    zs.next_in = bufptr;
    zs.avail_in = length;
    zs.total_in_lo32 = length;
    while( 1 )
    {
        res = BZ2_bzDecompressInit( &zs, 0, 0 );
        if( res != BZ_OK )
            break;
        while( res != BZ_STREAM_END )
        {
            memset( buf, 0, sizeof(buf) );
            zs.next_out = buf;
            zs.avail_out = 2047;
            zs.total_out_lo32 = length;
            res = BZ2_bzDecompress( &zs );
            str += buf;
            if( res != BZ_OK && res != BZ_STREAM_END )
                break;
        }
        break;
    }
    BZ2_bzDecompressEnd( &zs );
    if( res != BZ_STREAM_END )
        str.clear();
    return str;
}

class SdictDictionary: public BtreeIndexing::BtreeDictionary
{
    Mutex idxMutex;
    File::Class idx;
    IdxHeader idxHeader;
    ChunkedStorage::Reader chunks;
    string dictionaryName;
    File::Class df;
    QIcon dictionaryIcon, dictionaryNativeIcon;
    bool dictionaryIconLoaded;

  public:

    SdictDictionary( string const & id, string const & indexFile,
                     vector< string > const & dictionaryFiles );

    ~SdictDictionary();

    virtual string getName() throw()
    { return dictionaryName; }

    virtual map< Dictionary::Property, string > getProperties() throw()
    { return map< Dictionary::Property, string >(); }

    virtual unsigned long getArticleCount() throw()
    { return idxHeader.articleCount; }

    virtual unsigned long getWordCount() throw()
    { return idxHeader.wordCount; }

    virtual QIcon getIcon() throw();

    virtual QIcon getNativeIcon() throw();

    inline virtual quint32 getLangFrom() const
    { return idxHeader.langFrom; }

    inline virtual quint32 getLangTo() const
    { return idxHeader.langTo; }

    virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                        vector< wstring > const & alts,
                                                        wstring const & )
      throw( std::exception );

private:

    void loadIcon();

    /// Loads the article.
    void loadArticle( uint32_t address,
                      string & articleText );
    string convert( string const & in_data );

    friend class SdictArticleRequest;
};

SdictDictionary::SdictDictionary( string const & id,
                                  string const & indexFile,
                                  vector< string > const & dictionaryFiles ):
    BtreeDictionary( id, dictionaryFiles ),
    idx( indexFile, "rb" ),
    idxHeader( idx.read< IdxHeader >() ),
    chunks( idx, idxHeader.chunksOffset ),
    df( dictionaryFiles[ 0 ], "rb" ),
    dictionaryIconLoaded( false )
{
    // Read dictionary name

    idx.seek( sizeof( idxHeader ) );
    vector< char > dName( idx.read< uint32_t >() );
    idx.read( &dName.front(), dName.size() );
    dictionaryName = string( &dName.front(), dName.size() );

    // Initialize the index

    openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                          idxHeader.indexRootOffset ),
               idx, idxMutex );
}

SdictDictionary::~SdictDictionary()
{
    df.close();
}

QIcon SdictDictionary::getNativeIcon() throw()
{
  loadIcon();
  return dictionaryNativeIcon;
}

QIcon SdictDictionary::getIcon() throw()
{
  loadIcon();
  return dictionaryIcon;
}

void SdictDictionary::loadIcon()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension

  fileName.chop( 3 );
  fileName += "bmp";
  QFileInfo info( fileName );

  if ( !info.exists() )
  {
      fileName.chop( 3 );
      fileName += "png";
      info = QFileInfo( fileName );
  }

  if ( info.exists() )
  {
    QImage img( fileName );

    if ( !img.isNull() )
    {
      // Load successful

      // Apply the color key

      img.setAlphaChannel( img.createMaskFromColor( QColor( 192, 192, 192 ).rgb(),
                                                    Qt::MaskOutColor ) );

      dictionaryNativeIcon = QIcon( QPixmap::fromImage( img ) );

      // Transform it to be square
      int max = img.width() > img.height() ? img.width() : img.height();

      QImage result( max, max, QImage::Format_ARGB32 );
      result.fill( 0 ); // Black transparent

      QPainter painter( &result );

      painter.drawImage( QPoint( img.width() == max ? 0 : ( max - img.width() ) / 2,
                                 img.height() == max ? 0 : ( max - img.height() ) / 2 ),
                         img );

      painter.end();

      dictionaryIcon = QIcon( QPixmap::fromImage( result ) );
    }
  }

  if ( dictionaryIcon.isNull() )
  {
    // Load failed -- use default icons
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/icon32_sdict.png");
  }

  dictionaryIconLoaded = true;
}

string SdictDictionary::convert( string const & in )
{
//    DPRINTF( "Source>>>>>>>>>>: %s\n\n\n", in.c_str() );

    string inConverted;

    inConverted.reserve( in.size() );

    bool afterEol = false;

    for( string::const_iterator i = in.begin(), j = in.end(); i != j; ++i )
    {
      switch( *i )
      {
        case '\n':
          afterEol = true;
          inConverted.append( "<br/>" );
        break;

        case ' ':
          if ( afterEol )
          {
            inConverted.append( "&nbsp;" );
            break;
          }
          // Fall-through

        default:
          inConverted.push_back( *i );
          afterEol = false;
      }
    }

    QDomDocument dd;
    QString errorStr;
    int errorLine, errorColumn;

    if( !dd.setContent( QByteArray( inConverted.c_str() ), false, &errorStr, &errorLine, &errorColumn ) )
    {
        FDPRINTF( stderr, "Sdictionary article parse failed: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
        FDPRINTF( stderr, "The input was: %s\n", in.c_str() );
        return in;
    }

    QDomNodeList nodes = dd.elementsByTagName( "r" ); // Reference to another word

    while( nodes.size() )
    {
      QDomElement el = nodes.at( 0 ).toElement();

      el.setTagName( "a" );
      el.setAttribute( "href", QString( "bword:" ) + el.text() );
      el.setAttribute( "class", "sdict_wordref" );
    }

    nodes = dd.elementsByTagName( "t" ); // Transcription

    while( nodes.size() )
    {
      QDomElement el = nodes.at( 0 ).toElement();

      el.setTagName( "span" );
      el.setAttribute( "class", "sdict_tr" );
    }

    nodes = dd.elementsByTagName( "l" ); // List

    while( nodes.size() )
    {
      QDomElement el = nodes.at( 0 ).toElement();

      el.setTagName( "ul" );
    }

    nodes = dd.elementsByTagName( "f" ); // Word forms

    while( nodes.size() )
    {
      QDomElement el = nodes.at( 0 ).toElement();

      el.setTagName( "span" );
      el.setAttribute( "class", "sdict_forms" );
    }

//    DPRINTF( "Result>>>>>>>>>>: %s\n\n\n", dd.toByteArray().data() );

    return dd.toByteArray().data();
}

void SdictDictionary::loadArticle( uint32_t address,
                                   string & articleText )
{
    uint32_t articleOffset = address;
    uint32_t articleSize;

    vector< char > articleBody;

    df.seek( articleOffset );
    df.read( &articleSize, sizeof(articleSize) );
    articleBody.resize( articleSize );
    df.read( &articleBody.front(), articleSize );

    if ( articleBody.empty() )
      throw exCantReadFile( getDictionaryFilenames()[ 0 ] );

    if( idxHeader.compressionType == 1 )
        articleText = decompressZlib( articleBody.data(), articleSize );
    else if( idxHeader.compressionType == 2 )
        articleText = decompressBzip2( articleBody.data(), articleSize );
    else
        articleText = string( articleBody.data(), articleSize );

    articleText = "<div class=\"sdict\">" + articleText + "</div>";
    articleText = convert( articleText );
}

/// SdictDictionary::getArticle()

class SdictArticleRequest;

class SdictArticleRequestRunnable: public QRunnable
{
  SdictArticleRequest & r;
  QSemaphore & hasExited;

public:

  SdictArticleRequestRunnable( SdictArticleRequest & r_,
                               QSemaphore & hasExited_ ): r( r_ ),
                                                          hasExited( hasExited_ )
  {}

  ~SdictArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class SdictArticleRequest: public Dictionary::DataRequest
{
  friend class SdictArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  SdictDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  SdictArticleRequest( wstring const & word_,
                       vector< wstring > const & alts_,
                       SdictDictionary & dict_ ):
    word( word_ ), alts( alts_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new SdictArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by DslArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~SdictArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void SdictArticleRequestRunnable::run()
{
  r.run();
}

void SdictArticleRequest::run()
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

  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
  {
      result += "<h3>";
      result += i->second.first;
      result += "</h3>";
      result += i->second.second;
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
      result += "<h3>";
      result += i->second.first;
      result += "</h3>";
      result += i->second.second;
  }

  Mutex::Lock _( dataMutex );

  data.resize( result.size() );

  memcpy( &data.front(), result.data(), result.size() );

  hasAnyData = true;

  finish();
}

sptr< Dictionary::DataRequest > SdictDictionary::getArticle( wstring const & word,
                                                             vector< wstring > const & alts,
                                                             wstring const & )
  throw( std::exception )
{
  return new SdictArticleRequest( word, alts, *this );
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
      // Skip files with the extensions different to .dct to speed up the
      // scanning
      if ( i->size() < 4 ||
          strcasecmp( i->c_str() + ( i->size() - 4 ), ".dct" ) != 0 )
        continue;

      // Got the file -- check if we need to rebuid the index

      vector< string > dictFiles( 1, *i );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        try
        {
          File::Class df( *i, "rb" );

          DCT_header dictHeader;

          df.read( &dictHeader, sizeof(dictHeader) );
          if( strncmp( dictHeader.signature, "sdct", 4 ) )
          {
              DPRINTF( "File %s is not valid sdictionary file", i->c_str() );
              continue;
          }
          int compression = dictHeader.compression & 0x0F;

          vector< char > data;
          uint32_t size;

          df.seek( dictHeader.titleOffset );
          df.read( &size, sizeof( size ) );
          data.resize( size );
          df.read( &data.front(), size );

          string dictName;

          if( compression == 1 )
              dictName = decompressZlib( data.data(), size );
          else if( compression == 2 )
              dictName = decompressBzip2( data.data(), size );
          else
              dictName = string( data.data(), size );

          initializing.indexingDictionary( dictName );

          File::Class idx( indexFile, "wb" );
          IdxHeader idxHeader;
          memset( &idxHeader, 0, sizeof( idxHeader ) );

          // We write a dummy header first. At the end of the process the header
          // will be rewritten with the right values.

          idx.write( idxHeader );

          idx.write( (uint32_t) dictName.size() );
          idx.write( dictName.data(), dictName.size() );

          IndexedWords indexedWords;

          ChunkedStorage::Writer chunks( idx );

          uint32_t wordCount = 0;
          set< uint32_t > articleOffsets;
          uint32_t pos = dictHeader.fullIndexOffset;

          for( uint32_t j = 0; j < dictHeader.wordCount; j++ )
          {
            IndexElement el;
            df.seek( pos );
            df.read( &el, sizeof(el) );
            uint32_t articleOffset = dictHeader.articlesOffset + el.articleOffset;
            size = el.nextWord - sizeof(el);
            if( size < 0 )
                break;
            wordCount++;
            data.resize( size );
            df.read( &data.front(), size );

            if( articleOffsets.find( articleOffset ) == articleOffsets.end() )
                articleOffsets.insert( articleOffset );

            // Insert new entry

            indexedWords.addWord( Utf8::decode( string( data.data(), size ) ), articleOffset);

            pos += el.nextWord;
          }
          // Finish with the chunks

          idxHeader.chunksOffset = chunks.finish();

          // Build index

          IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

          idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
          idxHeader.indexRootOffset = idxInfo.rootOffset;

          indexedWords.clear(); // Release memory -- no need for this data

          // That concludes it. Update the header.

          idxHeader.signature = Signature;
          idxHeader.formatVersion = CurrentFormatVersion;

          idxHeader.articleCount = articleOffsets.size();
          idxHeader.wordCount = wordCount;

          idxHeader.langFrom = LangCoder::code2toInt( dictHeader.inputLang );
          idxHeader.langTo = LangCoder::code2toInt( dictHeader.outputLang );
          idxHeader.compressionType = compression;

          idx.rewind();

          idx.write( &idxHeader, sizeof( idxHeader ) );
        }
        catch( std::exception & e )
        {
          FDPRINTF( stderr, "Sdictionary dictionary indexing failed: %s, error: %s\n",
            i->c_str(), e.what() );
          continue;
        }
        catch( ... )
        {
          FDPRINTF( stderr, "Sdictionary dictionary indexing failed\n" );
          continue;
        }
      } // if need to rebuild
      dictionaries.push_back( new SdictDictionary( dictId,
                                                   indexFile,
                                                   dictFiles ) );
  }
  return dictionaries;
}

}
