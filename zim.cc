/* This file is (c) 2012 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifdef MAKE_ZIM_SUPPORT

#include "zim.hh"
#include "btreeidx.hh"
#include "fsencoding.hh"
#include "folding.hh"
#include "dprintf.hh"
#include "utf8.hh"
#include "decompress.hh"
#include "langcoder.hh"
#include "wstring_qt.hh"
#include "filetype.hh"
#include "file.hh"

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QRunnable>
#include <QSemaphore>
#include <QAtomicInt>
#include <QImage>

#include <string>
#include <set>
#include <map>

namespace Zim {

using std::string;
using std::map;
using std::vector;
using std::multimap;
using std::pair;
using std::set;
using gd::wstring;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

DEF_EX_STR( exNotZimFile, "Not an Zim file", Dictionary::Ex )
DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )

namespace {

#ifdef _MSC_VER
#pragma pack( push, 1 )
#endif

enum CompressionType
{
  Default = 0, None, Zlib, Bzip2, Lzma2
};

/// Zim file header
struct ZIM_header
{
    quint32 magicNumber;
    quint32 version;
    quint8 uuid[ 16 ];
    quint32 articleCount;
    quint32 clusterCount;
    quint64 urlPtrPos;
    quint64 titlePtrPos;
    quint64 clusterPtrPos;
    quint64 mimeListPos;
    quint32 mainPage;
    quint32 layoutPage;
    quint64 checksumPos;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct ArticleEntry
{
    quint16 mimetype;
    quint8 parameterLen;
    char nameSpace;
    quint32 revision;
    quint32 clusterNumber;
    quint32 blobNumber;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct RedirectEntry
{
    quint16 mimetype;
    quint8 parameterLen;
    char nameSpace;
    quint32 revision;
    quint32 redirectIndex;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

enum
{
  Signature = 0x584D495A, // ZIMX on little-endian, XMIZ on big-endian
  CurrentFormatVersion = 1 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  quint32 signature; // First comes the signature, ZIMX
  quint32 formatVersion; // File format version (CurrentFormatVersion)
  quint32 indexBtreeMaxElements; // Two fields from IndexInfo
  quint32 indexRootOffset;
  quint32 resourceIndexBtreeMaxElements; // Two fields from IndexInfo
  quint32 resourceIndexRootOffset;
  quint32 wordCount;
  quint32 articleCount;
  quint32 namePtr;
  quint32 descriptionPtr;
  quint32 langFrom;  // Source language
  quint32 langTo;    // Target language
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

quint32 readArticle( QFile & file, ZIM_header & header, uint32_t articleNumber, string & result,
                     set< quint32 > * loadedArticles = NULL )
{
  while( 1 )
  {
    if( articleNumber >= header.articleCount )
      break;

    file.seek( header.urlPtrPos + (quint64)articleNumber * 8 );
    quint64 pos;
    if( file.read( reinterpret_cast< char * >( &pos ), sizeof(pos) ) != sizeof(pos) )
      break;

    // Read article info

    quint16 mimetype;

    file.seek( pos );
    if( file.read( reinterpret_cast< char * >( &mimetype ), sizeof(mimetype) ) != sizeof(mimetype) )
      break;

    if( mimetype == 0xFFFF ) // Redirect to other article
    {
      RedirectEntry redEntry;
      if( file.read( reinterpret_cast< char * >( &redEntry ) + 2, sizeof(redEntry) - 2 ) != sizeof(redEntry) - 2 )
        break;
      if( articleNumber == redEntry.redirectIndex )
        break;
      articleNumber = redEntry.redirectIndex;
      continue;
    }

    if( loadedArticles && loadedArticles->find( articleNumber ) != loadedArticles->end() )
      break;

    ArticleEntry artEntry;
    artEntry.mimetype = mimetype;
    if( file.read( reinterpret_cast< char * >( &artEntry ) + 2, sizeof(artEntry) - 2 ) != sizeof(artEntry) - 2 )
      break;

    // Read cluster pointers

    quint64 clusters[ 2 ];
    file.seek( header.clusterPtrPos + (quint64)artEntry.clusterNumber * 8 );
    if( file.read( reinterpret_cast< char * >( clusters ), sizeof(clusters) ) != sizeof(clusters) )
      break;

    // Calculate cluster size

    quint64 clusterSize;
    if( artEntry.clusterNumber < header.clusterCount - 1 )
      clusterSize = clusters[ 1 ] - clusters[ 0 ];
    else
      clusterSize = file.size() - clusters[ 0 ];

    // Read cluster data

    file.seek( clusters[ 0 ] );

    char compressionType;
    if( !file.getChar( &compressionType ) )
      break;

    string decompressedData;

    QByteArray data = file.read( clusterSize );
    if( compressionType == Default || compressionType == None )
      decompressedData = string( data.data(), data.size() );
    else
    if( compressionType == Zlib )
      decompressedData = decompressZlib( data.constData(), data.size() );
    else
    if( compressionType == Bzip2 )
      decompressedData = decompressBzip2( data.constData(), data.size() );
    else
    if( compressionType == Lzma2 )
      decompressedData = decompressLzma2( data.constData(), data.size() );
    else
      break;

    // Take article data from cluster

    quint32 blobCount;
    memcpy( &blobCount, decompressedData.data(), sizeof(blobCount) );
    if( artEntry.blobNumber > blobCount )
      break;

    quint32 offsets[ 2 ];
    memcpy( offsets, decompressedData.data() + artEntry.blobNumber * 4, sizeof(offsets) );
    quint32 size = offsets[ 1 ] - offsets[ 0 ];

    result.clear();
    result.append( decompressedData, offsets[ 0 ], size );

    return articleNumber;
  }
  return 0xFFFFFFFF;
}

class ZimDictionary: public BtreeIndexing::BtreeDictionary
{
    Mutex idxMutex;
    Mutex zimMutex, idxResourceMutex;
    File::Class idx;
    BtreeIndex resourceIndex;
    IdxHeader idxHeader;
    string dictionaryName;
    QFile df;
    ZIM_header zimHeader;

  public:

    ZimDictionary( string const & id, string const & indexFile,
                     vector< string > const & dictionaryFiles );

    ~ZimDictionary();

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

    /// Loads the resource.
    void loadResource( std::string &resourceName, string & data );

protected:

    virtual void loadIcon() throw();

private:

    /// Loads the article.
    quint32 loadArticle( quint32 address,
                         string & articleText,
                         set< quint32 > * loadedArticles );

    string convert( string const & in_data );
    friend class ZimArticleRequest;
    friend class ZimResourceRequest;
};

ZimDictionary::ZimDictionary( string const & id,
                              string const & indexFile,
                              vector< string > const & dictionaryFiles ):
    BtreeDictionary( id, dictionaryFiles ),
    idx( indexFile, "rb" ),
    idxHeader( idx.read< IdxHeader >() ),
    df( FsEncoding::decode( dictionaryFiles[ 0 ].c_str() ) )
{
    // Open data file

    df.open( QFile::ReadOnly );
    memset( &zimHeader, 0, sizeof(zimHeader) );
    df.read( reinterpret_cast< char * >( &zimHeader ), sizeof( zimHeader ) );

    // Initialize the indexes

    openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                          idxHeader.indexRootOffset ),
               idx, idxMutex );

    resourceIndex.openIndex( IndexInfo( idxHeader.resourceIndexBtreeMaxElements,
                                        idxHeader.resourceIndexRootOffset ),
                             idx, idxResourceMutex );

    // Read dictionary name

    if( idxHeader.namePtr == 0xFFFFFFFF )
    {
      int n = df.fileName().lastIndexOf( '/' );
      dictionaryName = string( df.fileName().mid( n + 1 ).toUtf8().constData() );
    }
    else
    {
      readArticle( df, zimHeader, idxHeader.namePtr, dictionaryName );
    }
}

ZimDictionary::~ZimDictionary()
{
    df.close();
}

void ZimDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension
  fileName.chop( 3 );

  if( !loadIconFromFile( fileName ) )
  {
    // Load failed -- use default icons
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/icon32_zim.png");
  }

  dictionaryIconLoaded = true;
}

quint32 ZimDictionary::loadArticle( quint32 address,
                                    string & articleText,
                                    set< quint32 > * loadedArticles )
{
quint32 ret;
  {
    Mutex::Lock _( zimMutex );
    ret = readArticle( df, zimHeader, address, articleText, loadedArticles );
  }
  articleText = convert( articleText );
  return ret;
}

string ZimDictionary::convert( const string & in )
{
  QString text = QString::fromUtf8( in.c_str() );

  text.replace( QRegExp( "<\\s*body\\s*([^>]*)background:([^;\"]*)" ),
                QString( "<body \\1background: inherited;" ) );

  text.replace( QRegExp( "<\\s*(img|script)\\s*([^>]*)src=\"/" ),
                QString( "<\\1 \\2src=\"bres://%1/").arg( getId().c_str() ) );

  text.replace( QRegExp( "<\\s*link\\s*([^>]*)href=\"/" ),
                QString( "<link \\1href=\"bres://%1/").arg( getId().c_str() ) );

  text.replace( QRegExp( "<\\s*a\\s*([^>]*)href=\"/[^\"]*\"\\s*title=\"([^\"]*)\"" ),
                QString( "<a href=\"gdlookup://localhost/\\2\" title=\"\\2\"") );

  text.replace( QRegExp( "<\\s*a\\s*([^>]*)href=\"/A/([^\"]*)\"" ),
                QString( "<a \\1href=\"gdlookup://localhost/\\2\"") );

  // Fix outstanding elements
  text += "<br style=\"clear:both;\" />";

  return text.toUtf8().data();
}

void ZimDictionary::loadResource( std::string & resourceName, string & data )
{
  vector< WordArticleLink > link;
  string resData;

  link = resourceIndex.findArticles( Utf8::decode( resourceName ) );

  if( link.empty() )
    return;

  {
    Mutex::Lock _( zimMutex );
    readArticle( df, zimHeader, link[ 0 ].articleOffset, data );
  }
}

QString const& ZimDictionary::getDescription()
{
    if( !dictionaryDescription.isEmpty() || idxHeader.descriptionPtr == 0xFFFFFFFF )
        return dictionaryDescription;

    string str;
    {
      Mutex::Lock _( zimMutex );
      readArticle( df, zimHeader, idxHeader.descriptionPtr, str );
    }

    if( !str.empty() )
      dictionaryDescription = QString::fromUtf8( str.c_str(), str.size() );

    return dictionaryDescription;
}

/// ZimDictionary::getArticle()

class ZimArticleRequest;

class ZimArticleRequestRunnable: public QRunnable
{
  ZimArticleRequest & r;
  QSemaphore & hasExited;

public:

  ZimArticleRequestRunnable( ZimArticleRequest & r_,
                             QSemaphore & hasExited_ ): r( r_ ),
                                                        hasExited( hasExited_ )
  {}

  ~ZimArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class ZimArticleRequest: public Dictionary::DataRequest
{
  friend class ZimArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  ZimDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  ZimArticleRequest( wstring const & word_,
                     vector< wstring > const & alts_,
                     ZimDictionary & dict_ ):
    word( word_ ), alts( alts_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new ZimArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by ZimArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~ZimArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void ZimArticleRequestRunnable::run()
{
  r.run();
}

void ZimArticleRequest::run()
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

  set< quint32 > articlesIncluded; // Some synonims make it that the articles
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

    // Now grab that article

    string headword, articleText;

    headword = chain[ x ].word;

    quint32 articleNumber;
    try
    {
      articleNumber = dict.loadArticle( chain[ x ].articleOffset, articleText, &articlesIncluded );
    }
    catch(...)
    {
    }

    if( articleNumber == 0xFFFFFFFF )
      continue; // No article loaded

    if ( articlesIncluded.find( articleNumber ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

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

    articlesIncluded.insert( articleNumber );
  }

  if ( mainArticles.empty() && alternateArticles.empty() )
  {
    // No such word
    finish();
    return;
  }

  string result;

  // See Issue #271: A mechanism to clean-up invalid HTML cards.
  string cleaner = "</font>""</font>""</font>""</font>""</font>""</font>"
                   "</font>""</font>""</font>""</font>""</font>""</font>"
                   "</b></b></b></b></b></b></b></b>"
                   "</i></i></i></i></i></i></i></i>"
                   "</a></a></a></a></a></a></a></a>";

  multimap< wstring, pair< string, string > >::const_iterator i;


  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
  {
      result += "<div class=\"zimdict\">";
      result += "<h2 class=\"zimdict_headword\">";
      result += i->second.first;
      result += "</h2>";
      result += i->second.second;
      result += cleaner + "</div>";
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
      result += "<div class=\"zimdict\">";
      result += "<h2 class=\"zimdict_headword\">";
      result += i->second.first;
      result += "</h2>";
      result += i->second.second;
      result += cleaner + "</div>";
  }

  Mutex::Lock _( dataMutex );

  data.resize( result.size() );

  memcpy( &data.front(), result.data(), result.size() );

  hasAnyData = true;

  finish();
}

sptr< Dictionary::DataRequest > ZimDictionary::getArticle( wstring const & word,
                                                           vector< wstring > const & alts,
                                                           wstring const & )
  throw( std::exception )
{
  return new ZimArticleRequest( word, alts, *this );
}

//// ZimDictionary::getResource()

class ZimResourceRequest;

class ZimResourceRequestRunnable: public QRunnable
{
  ZimResourceRequest & r;
  QSemaphore & hasExited;

public:

  ZimResourceRequestRunnable( ZimResourceRequest & r_,
                              QSemaphore & hasExited_ ): r( r_ ),
                                                         hasExited( hasExited_ )
  {}

  ~ZimResourceRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class ZimResourceRequest: public Dictionary::DataRequest
{
  friend class ZimResourceRequestRunnable;

  ZimDictionary & dict;

  string resourceName;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  ZimResourceRequest( ZimDictionary & dict_,
                      string const & resourceName_ ):
    dict( dict_ ),
    resourceName( resourceName_ )
  {
    QThreadPool::globalInstance()->start(
      new ZimResourceRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by ZimResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~ZimResourceRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void ZimResourceRequestRunnable::run()
{
  r.run();
}

void ZimResourceRequest::run()
{
  // Some runnables linger enough that they are cancelled before they start
  if ( isCancelled )
  {
    finish();
    return;
  }

  try
  {
    string resource;
    dict.loadResource( resourceName, resource );
    if( resource.empty() )
      throw File::Ex();

    if( Filetype::isNameOfCSS( resourceName ) )
    {
      QString css = QString::fromUtf8( resource.data(), resource.size() );
      dict.isolateCSS( css, ".zimdict" );
      QByteArray bytes = css.toUtf8();
      data.resize( bytes.size() );
      memcpy( &data.front(), bytes.constData(), bytes.size() );
    }
    else
    if ( Filetype::isNameOfTiff( resourceName ) )
    {
      // Convert it

      dataMutex.lock();

      QImage img = QImage::fromData( reinterpret_cast< const uchar * >( resource.data() ), resource.size() );

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
    else
    {
      Mutex::Lock _( dataMutex );
      data.resize( resource.size() );
      memcpy( &data.front(), resource.data(), data.size() );
    }

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

sptr< Dictionary::DataRequest > ZimDictionary::getResource( string const & name )
  throw( std::exception )
{
  return new ZimResourceRequest( *this, name );
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
      // Skip files with the extensions different to .zim to speed up the
      // scanning
      if ( i->size() < 4 ||
          strcasecmp( i->c_str() + ( i->size() - 4 ), ".zim" ) != 0 )
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
          ZIM_header zh;

          unsigned articleCount = 0;
          unsigned wordCount = 0;

          QFile df( FsEncoding::decode( i->c_str() ) );
          df.open( QFile::ReadOnly );

          qint64 ret = df.read( reinterpret_cast< char * >( &zh ), sizeof( zh ) );
          if( ret != sizeof( zh ) )
            throw exCantReadFile( i->c_str() );

          if( zh.magicNumber != 0x44D495A )
            throw exNotZimFile( i->c_str() );

          {
            int n = df.fileName().lastIndexOf( '/' );
            initializing.indexingDictionary( df.fileName().mid( n + 1 ).toUtf8().constData() );
          }

          File::Class idx( indexFile, "wb" );
          IdxHeader idxHeader;
          memset( &idxHeader, 0, sizeof( idxHeader ) );
          idxHeader.namePtr = 0xFFFFFFFF;
          idxHeader.descriptionPtr = 0xFFFFFFFF;

          // We write a dummy header first. At the end of the process the header
          // will be rewritten with the right values.

          idx.write( idxHeader );

          IndexedWords indexedWords, indexedResources;

          QByteArray artEntries;
          df.seek( zh.urlPtrPos );
          artEntries = df.read( (quint64)zh.articleCount * 8 );

          QVector< quint64 > clusters;
          clusters.reserve( zh.clusterCount );
          df.seek( zh.clusterPtrPos );
          {
            QByteArray data = df.read( (quint64)zh.clusterCount * 8 );
            for( unsigned n = 0; n < zh.clusterCount; n++ )
              clusters.append( *( reinterpret_cast< const quint64 * >( data.constData() ) + n ) );
          }

          const quint64 * ptr;
          quint16 mimetype;
          ArticleEntry artEntry;
          RedirectEntry redEntry;
          string url, title;
          char nameSpace;
          for( unsigned n = 0; n < zh.articleCount; n++ )
          {
            ptr = reinterpret_cast< const quint64 * >( artEntries.constData() ) + n;
            df.seek( *ptr );
            df.read( reinterpret_cast< char * >( &mimetype ), sizeof(mimetype) );
            if( mimetype == 0xFFFF )
            {
              redEntry.mimetype = mimetype;
              ret = df.read( reinterpret_cast< char * >( &redEntry ) + 2, sizeof(RedirectEntry) - 2 );
              if( ret != sizeof(RedirectEntry) - 2 )
                throw exCantReadFile( i->c_str() );

              nameSpace = redEntry.nameSpace;
            }
            else
            {
              artEntry.mimetype = mimetype;
              ret = df.read( reinterpret_cast< char * >( &artEntry ) + 2, sizeof(ArticleEntry) - 2 );
              if( ret != sizeof(ArticleEntry) - 2 )
                throw exCantReadFile( i->c_str() );

              nameSpace = artEntry.nameSpace;

              if( nameSpace == 'A' )
                articleCount++;
            }

            // Read article url and title
            char ch;

            url.clear();
            while( df.getChar( &ch ) )
            {
              if( ch == 0 )
                break;
              url.push_back( ch );
            }

            title.clear();
            while( df.getChar( &ch ) )
            {
              if( ch == 0 )
                break;
              title.push_back( ch );
            }

            if( nameSpace == 'A' )
            {
              if( !title.empty() )
                indexedWords.addWord( Utf8::decode( title ), n );
              else
                indexedWords.addWord( Utf8::decode( url ), n );
              wordCount++;
            }
            else
            if( nameSpace == 'M' )
            {
              if( url.compare( "Title") == 0 )
              {
                idxHeader.namePtr = n;
                string name;
                readArticle( df, zh, n, name );
                initializing.indexingDictionary( name );
              }
              else
              if( url.compare( "Description") == 0 )
                idxHeader.descriptionPtr = n;
              else
              if( url.compare( "Language") == 0 )
              {
                string lang;
                readArticle( df, zh, n, lang );
                if( lang.size() == 2 )
                  idxHeader.langFrom = LangCoder::code2toInt( lang.c_str() );
                else
                if( lang.size() == 3 )
                  idxHeader.langFrom = LangCoder::code3toInt( lang.c_str() );
                idxHeader.langTo = idxHeader.langFrom;
              }
            }
            else
            {
              url.insert( url.begin(), '/' );
              url.insert( url.begin(), nameSpace );
              indexedResources.addSingleWord( Utf8::decode( url ), n );
            }
          }

          // Build index

          {
            IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

            idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
            idxHeader.indexRootOffset = idxInfo.rootOffset;

            indexedWords.clear(); // Release memory -- no need for this data
          }

          {
            IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedResources, idx );

            idxHeader.resourceIndexBtreeMaxElements = idxInfo.btreeMaxElements;
            idxHeader.resourceIndexRootOffset = idxInfo.rootOffset;

            indexedResources.clear(); // Release memory -- no need for this data
          }

          idxHeader.signature = Signature;
          idxHeader.formatVersion = CurrentFormatVersion;

          idxHeader.articleCount = articleCount;
          idxHeader.wordCount = wordCount;

          idx.rewind();

          idx.write( &idxHeader, sizeof( idxHeader ) );
        }
        catch( std::exception & e )
        {
          FDPRINTF( stderr, "Zim dictionary indexing failed: %s, error: %s\n",
            i->c_str(), e.what() );
          continue;
        }
        catch( ... )
        {
          FDPRINTF( stderr, "Zim dictionary indexing failed\n" );
          continue;
        }
      }
      dictionaries.push_back( new ZimDictionary( dictId,
                                                 indexFile,
                                                 dictFiles ) );
  }
  return dictionaries;
}

} // namespace Zim

#endif
