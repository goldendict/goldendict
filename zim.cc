/* This file is (c) 2012 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifdef MAKE_ZIM_SUPPORT

#include "zim.hh"
#include "btreeidx.hh"
#include "fsencoding.hh"
#include "folding.hh"
#include "gddebug.hh"
#include "utf8.hh"
#include "decompress.hh"
#include "langcoder.hh"
#include "wstring_qt.hh"
#include "filetype.hh"
#include "file.hh"
#include "utils.hh"
#include "tiff.hh"
#include "ftshelpers.hh"
#include "htmlescape.hh"
#include "splitfile.hh"

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QRunnable>
#include <QSemaphore>
#include <QAtomicInt>
#include <QImage>
#include <QDir>
#include <QDebug>

#include <QRegularExpression>

#include <string>
#include <set>
#include <map>
#include <algorithm>

namespace Zim {

#define CACHE_SIZE 3

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
DEF_EX( exUserAbort, "User abort", Dictionary::Ex )


//namespace {

class ZimFile;

#pragma pack( push, 1 )

enum CompressionType
{
  Default = 0, None, Zlib, Bzip2, Lzma2, Zstd
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

#pragma pack( pop )

// Class for support of split zim files

struct Cache
{
  char * data;
  quint32 clusterNumber;
  int stamp;
  int count, size;

  Cache() :
    data( 0 ),
    clusterNumber( 0 ),
    stamp( -1 ),
    count( 0 ),
    size( 0 )
  {}
};

class ZimFile : public SplitFile::SplitFile
{
public:
  ZimFile();
  ZimFile( const QString & name );
  ~ZimFile();

  virtual void setFileName( const QString & name );
  bool open();
  void close()
  {
    SplitFile::close();
    clearCache();
  }
  const ZIM_header & header() const
  { return zimHeader; }
  string getClusterData( quint32 cluster_nom );

private:
  ZIM_header zimHeader;
  Cache cache[ CACHE_SIZE ];
  int stamp;
  QVector< QPair< quint64, quint32 > > clusterOffsets;

  void clearCache();
};

ZimFile::ZimFile() :
  stamp( 0 )
{
  memset( &zimHeader, 0, sizeof( zimHeader ) );
}

ZimFile::ZimFile( const QString & name )
{
  setFileName( name );
}

ZimFile::~ZimFile()
{
  clearCache();
}

void ZimFile::setFileName( const QString & name )
{
  close();
  memset( &zimHeader, 0, sizeof( zimHeader ) );
  clearCache();

  appendFile( name );

  if( name.endsWith( ".zimaa", Qt::CaseInsensitive ) )
  {
    QString fname = name;

    for( int i = 0; i < 26; i++ )
    {
      fname[ fname.size() - 2 ] = (char)( 'a' + i );

      int j;
      for( j = 1; j < 26; j++ )
      {
        fname[ fname.size() - 1 ] = (char)( 'a' + j );
        if( !QFileInfo( fname ).isFile() )
          break;

        appendFile( fname );
      }

      if( j < 26 )
        break;
    }
  }
}

void ZimFile::clearCache()
{
  for( int i = 0; i < CACHE_SIZE; i++ )
  {
    if( cache[ i ].data )
    {
      free( cache[ i ].data );
      cache[ i ].data = 0;
    }
    cache[ i ].clusterNumber = 0;
    cache[ i ].stamp = -1;
    cache[ i ].count = 0;
    cache[ i ].size = 0;
  }
  stamp = 0;
}

bool ZimFile::open()
{
  if( !SplitFile::open( QIODevice::ReadOnly ) )
    return false;

  memset( &zimHeader, 0, sizeof( zimHeader ) );

  if( read( reinterpret_cast< char * >( &zimHeader ), sizeof( zimHeader ) ) != sizeof( zimHeader ) )
    return false;

// Clusters in zim file may be placed in random order.
// We create sorted offsets list to calculate clusters size.

  clusterOffsets.resize( zimHeader.clusterCount );
  QVector< quint64 > offs;
  offs.resize( zimHeader.clusterCount );

  seek( zimHeader.clusterPtrPos );
  qint64 size = zimHeader.clusterCount * sizeof( quint64 );
  if( read( reinterpret_cast< char * >( offs.data() ), size) != size )
  {
    vector< string > names;
    getFilenames( names );
    throw exCantReadFile( names[ 0 ] );
  }

  for( quint32 i = 0; i < zimHeader.clusterCount; i++ )
    clusterOffsets[ i ] = QPair< quint64, quint32 >( offs.at( i ), i );

  std::sort( clusterOffsets.begin(), clusterOffsets.end() );

  return true;
}

string ZimFile::getClusterData( quint32 cluster_nom )
{
  // Check cache
  int target = 0;
  bool found = false;
  int lastStamp = INT_MAX;

  for( int i = 0; i < CACHE_SIZE; i++ )
  {
    if( cache[ i ].clusterNumber == cluster_nom && cache[ i ].count )
    {
      found = true;
      target = i;
      break;
    }

    if( cache[ i ].stamp < lastStamp )
    {
      lastStamp = cache[ i ].stamp;
      target = i;
    }
  }

  cache[ target ].stamp = ++stamp;
  if( stamp < 0 )
  {
     stamp = 0;
     for (int i = 0; i < CACHE_SIZE; i++)
       cache[ i ].stamp = -1;
  }

  if( found )
  {
    // Cache hit
    return string( cache[ target ].data, cache[ target ].count );
  }

  // Cache miss, read data from file

  // Calculate cluster size

  quint64 clusterSize;
  quint32 nom;
  for( nom = 0; nom < zimHeader.clusterCount; nom++ )
    if( clusterOffsets.at( nom ).second == cluster_nom )
      break;

  if( nom >= zimHeader.clusterCount ) // Invalid cluster nom
    return string();

  if( nom < zimHeader.clusterCount - 1 )
    clusterSize = clusterOffsets.at( nom + 1 ).first - clusterOffsets.at( nom ).first;
  else
    clusterSize = size() - clusterOffsets.at( nom ).first;

  // Read cluster data

  seek( clusterOffsets.at( nom ).first );

  char compressionType;
  if( !getChar( &compressionType ) )
    return string();

  string decompressedData;

  QByteArray data = read( clusterSize );

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
  if( compressionType == Zstd )
    decompressedData = decompressZstd( data.constData(), data.size() );
  else
    return string();

  if( decompressedData.empty() )
    return string();

  // Check BLOBs number in the cluster
  // We cache multi-element clusters only

  quint32 firstOffset;
  memcpy( &firstOffset, decompressedData.data(), sizeof(firstOffset) );
  quint32 blobCount = ( firstOffset - 4 ) / 4;

  if( blobCount > 1 )
  {
    // Fill cache
    int size = decompressedData.size();
    if( cache[ target ].count < size )
    {
      if( cache[ target ].data )
        free( cache[ target ].data );
      cache[ target ].data = ( char * )malloc( size );
      if( cache[ target ].data )
        cache[ target ].size = size;
      else
      {
        cache[ target ].size = 0;
        cache[ target ].count = 0;
      }
    }
    if( cache[ target ].size )
    {
      memcpy( cache[ target ].data, decompressedData.c_str(), size );
      cache[ target ].count = size;
      cache[ target ].clusterNumber = cluster_nom;
    }
  }

  return decompressedData;
}

// Some supporting functions

bool indexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion;
}

quint32 getArticleCluster( ZimFile & file, quint32 articleNumber )
{
  while( 1 )
  {
    ZIM_header const & header = file.header();
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

    ArticleEntry artEntry;
    artEntry.mimetype = mimetype;
    if( file.read( reinterpret_cast< char * >( &artEntry ) + 2, sizeof(artEntry) - 2 ) != sizeof(artEntry) - 2 )
      break;

    return artEntry.clusterNumber;
  }
  return 0xFFFFFFFF;
}

quint32 readArticle( ZimFile & file, quint32 articleNumber, string & result,
                     set< quint32 > * loadedArticles = NULL )
{
  result.clear();

  while( 1 )
  {
    ZIM_header const & header = file.header();
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

    // Read cluster data

    string decompressedData = file.getClusterData( artEntry.clusterNumber );
    if( decompressedData.empty() )
      break;

    // Take article data from cluster

    quint32 firstOffset;
    memcpy( &firstOffset, decompressedData.data(), sizeof(firstOffset) );
    quint32 blobCount = ( firstOffset - 4 ) / 4;
    if( artEntry.blobNumber > blobCount )
      break;

    quint32 offsets[ 2 ];
    memcpy( offsets, decompressedData.data() + artEntry.blobNumber * 4, sizeof(offsets) );
    quint32 size = offsets[ 1 ] - offsets[ 0 ];

    result.append( decompressedData, offsets[ 0 ], size );

    return articleNumber;
  }
  return 0xFFFFFFFF;
}

// ZimDictionary

class ZimDictionary: public BtreeIndexing::BtreeDictionary
{
    enum LINKS_TYPE { UNKNOWN, SLASH, NO_SLASH };

    Mutex idxMutex;
    Mutex zimMutex, idxResourceMutex;
    File::Class idx;
    BtreeIndex resourceIndex;
    IdxHeader idxHeader;
    string dictionaryName;
    ZimFile df;
    set< quint32 > articlesIndexedForFTS;
    LINKS_TYPE linksType;

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
                                                        wstring const &,
                                                        bool ignoreDiacritics )
      ;

    virtual sptr< Dictionary::DataRequest > getResource( string const & name )
      ;

    virtual QString const& getDescription();

    /// Loads the resource.
    void loadResource( std::string &resourceName, string & data );

    virtual sptr< Dictionary::DataRequest > getSearchResults( QString const & searchString,
                                                              int searchMode, bool matchCase,
                                                              int distanceBetweenWords,
                                                              int maxResults,
                                                              bool ignoreWordsOrder,
                                                              bool ignoreDiacritics );
    virtual void getArticleText( uint32_t articleAddress, QString & headword, QString & text );

    quint32 getArticleText( uint32_t articleAddress, QString & headword, QString & text,
                            set< quint32 > * loadedArticles );

    virtual void makeFTSIndex(QAtomicInt & isCancelled, bool firstIteration );

    virtual void setFTSParameters( Config::FullTextSearch const & fts )
    {
      can_FTS = fts.enabled
                && !fts.disabledTypes.contains( "ZIM", Qt::CaseInsensitive )
                && ( fts.maxDictionarySize == 0 || getArticleCount() <= fts.maxDictionarySize );
    }

    virtual void sortArticlesOffsetsForFTS( QVector< uint32_t > & offsets, QAtomicInt & isCancelled );

protected:

    virtual void loadIcon() throw();

private:

    /// Loads the article.
    quint32 loadArticle( quint32 address,
                         string & articleText,
                         set< quint32 > * loadedArticles,
                         bool rawText = false );

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
    df( FsEncoding::decode( dictionaryFiles[ 0 ].c_str() ) ),
    linksType( UNKNOWN )
{
    // Open data file

    df.open();

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
      QString name = QDir::fromNativeSeparators( FsEncoding::decode( dictionaryFiles[ 0 ].c_str() ) );
      int n = name.lastIndexOf( '/' );
      dictionaryName = string( name.mid( n + 1 ).toUtf8().constData() );
    }
    else
    {
      readArticle( df, idxHeader.namePtr, dictionaryName );
    }

    // Full-text search parameters

    can_FTS = true;

    ftsIdxName = indexFile + "_FTS";

    if( !Dictionary::needToRebuildIndex( dictionaryFiles, ftsIdxName )
        && !FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName, this ) )
      FTS_index_completed.ref();
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
                                    set< quint32 > * loadedArticles,
                                    bool rawText )
{
quint32 ret;
  {
    Mutex::Lock _( zimMutex );
    ret = readArticle( df, address, articleText, loadedArticles );
  }
  if( !rawText )
    articleText = convert( articleText );

  return ret;
}

string ZimDictionary::convert( const string & in )
{
  QString text = QString::fromUtf8( in.c_str() );

  // replace background
  text.replace( QRegularExpression( "<\\s*body\\s+([^>]*)(background(|-color)):([^;\"]*(;|))" ),
                QString( "<body \\1" ) );

  // pattern of img and script
  text.replace( QRegularExpression( "<\\s*(img|script)\\s+([^>]*)src=(\"|)(\\.\\.|)/" ),
                QString( "<\\1 \\2src=\\3bres://%1/").arg( getId().c_str() ) );

  // Fix links without '"'
  text.replace( QRegularExpression( "href=(\\.\\.|)/([^\\s>]+)" ),
                QString( "href=\"\\1/\\2\"" ) );

  // pattern <link... href="..." ...>
  text.replace( QRegularExpression( "<\\s*link\\s+([^>]*)href=\"(\\.\\.|)/" ),
                QString( "<link \\1href=\"bres://%1/").arg( getId().c_str() ) );

  // localize the http://en.wiki***.com|org/wiki/<key> series links
  // excluding those keywords that have ":" in it
  QString urlWiki = "\"http(s|)://en\\.(wiki(pedia|books|news|quote|source|voyage|versity)|wiktionary)\\.(org|com)/wiki/([^:\"]*)\"";
  text.replace( QRegularExpression( "<\\s*a\\s+(class=\"external\"\\s+|)href=" + urlWiki ),
                QString( "<a href=\"gdlookup://localhost/\\6\"" ) );

  // pattern <a href="..." ...>, excluding any known protocols such as http://, mailto:, #(comment)
  // these links will be translated into local definitions
  // <meta http-equiv="Refresh" content="0;url=../dsalsrv02.uchicago.edu/cgi-bin/0994.html">
  QRegularExpression rxLink( "<\\s*(?:a|meta)\\s+([^>]*)(?:href|url)=\"?(?!(?:\\w+://|#|mailto:|tel:))(/|)([^\"]*)\"\\s*(title=\"[^\"]*\")?[^>]*>" );
  QRegularExpressionMatchIterator it = rxLink.globalMatch( text );
  int pos = 0;
  QString newText;
  while( it.hasNext() )
  {
    QRegularExpressionMatch match = it.next();

    newText += text.mid( pos, match.capturedStart() - pos );
    pos = match.capturedEnd();

    QStringList list = match.capturedTexts();
    // Add empty strings for compatibility with QRegExp behaviour
    for( int i = list.size(); i < 5; i++ )
      list.append( QString() );

    QString tag = list[3];     // a url, ex: Precambrian_Chaotian.html
    if ( !list[4].isEmpty() )  // a title, ex: title="Precambrian/Chaotian"
      tag = list[4].split("\"")[1];

    // Check type of links inside articles
    if( linksType == UNKNOWN && tag.indexOf( '/' ) >= 0 )
    {
      QString word = QUrl::fromPercentEncoding( tag.toLatin1() );
      QRegularExpression htmlRx( "\\.(s|)htm(l|)$", QRegularExpression::CaseInsensitiveOption );
      word.remove( htmlRx ).
           replace( "_", " " );

      vector< WordArticleLink > links;
      links = findArticles( gd::toWString( word ) );

      if( !links.empty() )
      {
        linksType = SLASH;
      }
      else
      {
        word.remove( QRegularExpression(".*/") );
        links = findArticles( gd::toWString( word ) );
        if( !links.empty() )
        {
          linksType = NO_SLASH;
          links.clear();
        }
      }
    }

    if( linksType == SLASH || linksType == UNKNOWN )
    {
      tag.remove( QRegularExpression( "\\.(s|)htm(l|)$", QRegularExpression::PatternOption::CaseInsensitiveOption ) ).
          replace( "_", "%20" ).
          prepend( "<a href=\"gdlookup://localhost/" ).
          append( "\" " + list[4] + ">" );
    }
    else
    {
      tag.remove( QRegularExpression(".*/") ).
          remove( QRegularExpression( "\\.(s|)htm(l|)$", QRegularExpression::PatternOption::CaseInsensitiveOption ) ).
          replace( "_", "%20" ).
          prepend( "<a href=\"gdlookup://localhost/" ).
          append( "\" " + list[4] + ">" );
    }

    newText += tag;
  }
  if( pos )
  {
    newText += text.mid( pos );
    text = newText;
  }
  newText.clear();

  // Occasionally words needs to be displayed in vertical, but <br/> were changed to <br\> somewhere
  // proper style: <a href="gdlookup://localhost/Neoptera" ... >N<br/>e<br/>o<br/>p<br/>t<br/>e<br/>r<br/>a</a>
  QRegularExpression rxBR( "(<a href=\"gdlookup://localhost/[^\"]*\"\\s*[^>]*>)\\s*((\\w\\s*&lt;br(\\\\|/|)&gt;\\s*)+\\w)\\s*</a>",
                           QRegularExpression::UseUnicodePropertiesOption );
  pos = 0;
  QRegularExpressionMatchIterator it2 = rxBR.globalMatch( text );
  while( it2.hasNext() )
  {
    QRegularExpressionMatch match = it2.next();

    newText += text.mid( pos, match.capturedStart() - pos );
    pos = match.capturedEnd();

    QStringList list = match.capturedTexts();
    // Add empty strings for compatibility with QRegExp behaviour
    for( int i = match.lastCapturedIndex() + 1; i < 3; i++ )
      list.append( QString() );

    QString tag = list[2];
    tag.replace( QRegularExpression( "&lt;br( |)(\\\\|/|)&gt;", QRegularExpression::PatternOption::CaseInsensitiveOption ) , "<br/>" ).
        prepend( list[1] ).
        append( "</a>" );

    newText += tag;
  }
  if( pos )
  {
    newText += text.mid( pos );
    text = newText;
  }
  newText.clear();


  // // output all links in the page - only for analysis
  // QRegExp rxPrintAllLinks( "<\\s*a\\s+[^>]*href=\"[^\"]*\"[^>]*>",
  //                         Qt::CaseSensitive,
  //                         QRegExp::RegExp2 );
  // pos = 0;
  // while( (pos = rxPrintAllLinks.indexIn( text, pos )) >= 0 )
  // {
  //   QStringList list = rxPrintAllLinks.capturedTexts();
  //   qDebug() << "\n--Alllinks--" << list[0];
  //   pos += list[0].length() + 1;
  // }

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
    readArticle( df, link[ 0 ].articleOffset, data );
  }
}

QString const& ZimDictionary::getDescription()
{
    if( !dictionaryDescription.isEmpty() || idxHeader.descriptionPtr == 0xFFFFFFFF )
        return dictionaryDescription;

    string str;
    {
      Mutex::Lock _( zimMutex );
      readArticle( df, idxHeader.descriptionPtr, str );
    }

    if( !str.empty() )
      dictionaryDescription = QString::fromUtf8( str.c_str(), str.size() );

    return dictionaryDescription;
}

void ZimDictionary::makeFTSIndex( QAtomicInt & isCancelled, bool firstIteration )
{
  if( !( Dictionary::needToRebuildIndex( getDictionaryFilenames(), ftsIdxName )
         || FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName, this ) ) )
    FTS_index_completed.ref();

  if( haveFTSIndex() )
    return;

  if( ensureInitDone().size() )
    return;

  if( firstIteration )
    return;

  gdDebug( "Zim: Building the full-text index for dictionary: %s\n",
           getName().c_str() );

  try
  {
    Mutex::Lock _( getFtsMutex() );

    File::Class ftsIdx( ftsIndexName(), "wb" );

    FtsHelpers::FtsIdxHeader ftsIdxHeader;
    memset( &ftsIdxHeader, 0, sizeof( ftsIdxHeader ) );

    // We write a dummy header first. At the end of the process the header
    // will be rewritten with the right values.

    ftsIdx.write( ftsIdxHeader );

    ChunkedStorage::Writer chunks( ftsIdx );

    BtreeIndexing::IndexedWords indexedWords;

    QSet< uint32_t > setOfOffsets;
    setOfOffsets.reserve( getWordCount() );

    findArticleLinks( 0, &setOfOffsets, 0, &isCancelled );

    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    // We should sort articles order by cluster number
    // to effective use clusters data caching

    QVector< QPair< quint32, uint32_t > > offsetsWithClusters;
    offsetsWithClusters.reserve( setOfOffsets.size() );

    for( QSet< uint32_t >::ConstIterator it = setOfOffsets.constBegin();
         it != setOfOffsets.constEnd(); ++it )
    {
      if( Utils::AtomicInt::loadAcquire( isCancelled ) )
        throw exUserAbort();

      Mutex::Lock _( zimMutex );
      offsetsWithClusters.append( QPair< uint32_t, quint32 >( getArticleCluster( df, *it ), *it ) );
    }

    // Free memory
    setOfOffsets.clear();

    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    std::sort( offsetsWithClusters.begin(), offsetsWithClusters.end() );

    QVector< uint32_t > offsets;
    offsets.resize( offsetsWithClusters.size() );
    for( int i = 0; i < offsetsWithClusters.size(); i++ )
      offsets[ i ] = offsetsWithClusters.at( i ).second;

    // Free memory
    offsetsWithClusters.clear();

    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    QMap< QString, QVector< uint32_t > > ftsWords;

    set< quint32 > indexedArticles;
    quint32 articleNumber;

    // index articles for full-text search
    for( int i = 0; i < offsets.size(); i++ )
    {
      if( Utils::AtomicInt::loadAcquire( isCancelled ) )
        throw exUserAbort();

      QString headword, articleStr;

      articleNumber = getArticleText( offsets.at( i ), headword, articleStr,
                                      &indexedArticles );
      if( articleNumber == 0xFFFFFFFF )
        continue;

      indexedArticles.insert( articleNumber );

      FtsHelpers::parseArticleForFts( offsets.at( i ), articleStr, ftsWords );
    }

    // Free memory
    offsets.clear();

    QMap< QString, QVector< uint32_t > >::iterator it = ftsWords.begin();
    while( it != ftsWords.end() )
    {
      if( Utils::AtomicInt::loadAcquire( isCancelled ) )
        throw exUserAbort();

      uint32_t offset = chunks.startNewBlock();
      uint32_t size = it.value().size();

      chunks.addToBlock( &size, sizeof(uint32_t) );
      chunks.addToBlock( it.value().data(), size * sizeof(uint32_t) );

      indexedWords.addSingleWord( gd::toWString( it.key() ), offset );

      it = ftsWords.erase( it );
    }

    // Free memory
    ftsWords.clear();

    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    ftsIdxHeader.chunksOffset = chunks.finish();
    ftsIdxHeader.wordCount = indexedWords.size();

    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    BtreeIndexing::IndexInfo ftsIdxInfo = BtreeIndexing::buildIndex( indexedWords, ftsIdx );

    // Free memory
    indexedWords.clear();

    ftsIdxHeader.indexBtreeMaxElements = ftsIdxInfo.btreeMaxElements;
    ftsIdxHeader.indexRootOffset = ftsIdxInfo.rootOffset;

    ftsIdxHeader.signature = FtsHelpers::FtsSignature;
    ftsIdxHeader.formatVersion = FtsHelpers::CurrentFtsFormatVersion + getFtsIndexVersion();

    ftsIdx.rewind();
    ftsIdx.writeRecords( &ftsIdxHeader, sizeof(ftsIdxHeader), 1 );

    FTS_index_completed.ref();
  }
  catch( std::exception &ex )
  {
    gdWarning( "Zim: Failed building full-text search index for \"%s\", reason: %s\n", getName().c_str(), ex.what() );
    QFile::remove( FsEncoding::decode( ftsIdxName.c_str() ) );
  }
}

void ZimDictionary::sortArticlesOffsetsForFTS( QVector< uint32_t > & offsets,
                                               QAtomicInt & isCancelled )
{
  QVector< QPair< quint32, uint32_t > > offsetsWithClusters;
  offsetsWithClusters.reserve( offsets.size() );

  for( QVector< uint32_t >::ConstIterator it = offsets.constBegin();
       it != offsets.constEnd(); ++it )
  {
    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      return;

    Mutex::Lock _( zimMutex );
    offsetsWithClusters.append( QPair< uint32_t, quint32 >( getArticleCluster( df, *it ), *it ) );
  }

  std::sort( offsetsWithClusters.begin(), offsetsWithClusters.end() );

  for( int i = 0; i < offsetsWithClusters.size(); i++ )
    offsets[ i ] = offsetsWithClusters.at( i ).second;
}

void ZimDictionary::getArticleText( uint32_t articleAddress, QString & headword, QString & text )
{
  try
  {
    headword.clear();
    string articleText;

    loadArticle( articleAddress, articleText, 0, true );
    text = Html::unescape( QString::fromUtf8( articleText.data(), articleText.size() ) );
  }
  catch( std::exception &ex )
  {
    gdWarning( "Zim: Failed retrieving article from \"%s\", reason: %s\n", getName().c_str(), ex.what() );
  }
}

quint32 ZimDictionary::getArticleText( uint32_t articleAddress, QString & headword, QString & text,
                                    set< quint32 > * loadedArticles )
{
  quint32 articleNumber = 0xFFFFFFFF;
  try
  {
    headword.clear();
    string articleText;

    articleNumber = loadArticle( articleAddress, articleText, loadedArticles, true );
    text = Html::unescape( QString::fromUtf8( articleText.data(), articleText.size() ) );
  }
  catch( std::exception &ex )
  {
    gdWarning( "Zim: Failed retrieving article from \"%s\", reason: %s\n", getName().c_str(), ex.what() );
  }
  return articleNumber;
}

sptr< Dictionary::DataRequest > ZimDictionary::getSearchResults( QString const & searchString,
                                                                 int searchMode, bool matchCase,
                                                                 int distanceBetweenWords,
                                                                 int maxResults,
                                                                 bool ignoreWordsOrder,
                                                                 bool ignoreDiacritics )
{
  return new FtsHelpers::FTSResultsRequest( *this, searchString,searchMode, matchCase, distanceBetweenWords, maxResults, ignoreWordsOrder, ignoreDiacritics );
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
  bool ignoreDiacritics;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  ZimArticleRequest( wstring const & word_,
                     vector< wstring > const & alts_,
                     ZimDictionary & dict_, bool ignoreDiacritics_ ):
    word( word_ ), alts( alts_ ), dict( dict_ ), ignoreDiacritics( ignoreDiacritics_ )
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
    //hasExited.acquire();
  }
};

void ZimArticleRequestRunnable::run()
{
  r.run();
}

void ZimArticleRequest::run()
{
  if ( Utils::AtomicInt::loadAcquire( isCancelled ) )
  {
    finish();
    return;
  }

  vector< WordArticleLink > chain = dict.findArticles( word, ignoreDiacritics );

  for( unsigned x = 0; x < alts.size(); ++x )
  {
    /// Make an additional query for each alt

    vector< WordArticleLink > altChain = dict.findArticles( alts[ x ], ignoreDiacritics );

    chain.insert( chain.end(), altChain.begin(), altChain.end() );
  }

  multimap< wstring, pair< string, string > > mainArticles, alternateArticles;

  set< quint32 > articlesIncluded; // Some synonims make it that the articles
                                    // appear several times. We combat this
                                    // by only allowing them to appear once.

  wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );
  if( ignoreDiacritics )
    wordCaseFolded = Folding::applyDiacriticsOnly( wordCaseFolded );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    if ( Utils::AtomicInt::loadAcquire( isCancelled ) )
    {
      finish();
      return;
    }

    // Now grab that article

    string headword, articleText;

    headword = chain[ x ].word;

    quint32 articleNumber = 0xFFFFFFFF;
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
    if( ignoreDiacritics )
      headwordStripped = Folding::applyDiacriticsOnly( headwordStripped );

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

  // leave the invalid tags at the mercy of modern browsers.(webengine chrome)
  // https://html.spec.whatwg.org/#an-introduction-to-error-handling-and-strange-cases-in-the-parser
  // https://en.wikipedia.org/wiki/Tag_soup#HTML5
  string cleaner = "";

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
                                                           wstring const &,
                                                           bool ignoreDiacritics )
  
{
  return new ZimArticleRequest( word, alts, *this, ignoreDiacritics );
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
    //hasExited.release();
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
  ZimResourceRequest(ZimDictionary &dict_, string const &resourceName_)
      : dict(dict_), resourceName(resourceName_) {
      //(new ZimResourceRequestRunnable(*this, hasExited))->run();
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
    //hasExited.acquire();
  }
};

void ZimResourceRequestRunnable::run()
{
  r.run();
}

void ZimResourceRequest::run()
{
  // Some runnables linger enough that they are cancelled before they start
  if ( Utils::AtomicInt::loadAcquire( isCancelled ) )
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

      Mutex::Lock _( dataMutex );
      data.resize( bytes.size() );
      memcpy( &data.front(), bytes.constData(), bytes.size() );
    }
    else
    if ( Filetype::isNameOfTiff( resourceName ) )
    {
      // Convert it
      Mutex::Lock _( dataMutex );
      GdTiff::tiff2img( data );
    }
    else
    {
      Mutex::Lock _( dataMutex );
      data.resize( resource.size() );
      memcpy( &data.front(), resource.data(), data.size() );
    }

    Mutex::Lock _( dataMutex );
    hasAnyData = true;
  }
  catch( std::exception &ex )
  {
    gdWarning( "ZIM: Failed loading resource \"%s\" from \"%s\", reason: %s\n",
               resourceName.c_str(), dict.getName().c_str(), ex.what() );
    // Resource not loaded -- we don't set the hasAnyData flag then
  }

  finish();
}

sptr< Dictionary::DataRequest > ZimDictionary::getResource( string const & name )
  
{
  return new ZimResourceRequest( *this, name );
}

//} // anonymous namespace

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & initializing,
                                      unsigned maxHeadwordsToExpand )
  
{
  vector< sptr< Dictionary::Class > > dictionaries;

  for( vector< string >::const_iterator i = fileNames.begin(); i != fileNames.end();
       ++i )
  {
      // Skip files with the extensions different to .zim to speed up the
      // scanning

      QString firstName = QDir::fromNativeSeparators( FsEncoding::decode( i->c_str() ) );
      if( !firstName.endsWith( ".zim") && !firstName.endsWith( ".zimaa" ) )
        continue;

      // Got the file -- check if we need to rebuid the index
      ZimFile df( firstName );

      vector< string > dictFiles;
      df.getFilenames( dictFiles );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      try
      {
        if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
             indexIsOldOrBad( indexFile ) )
        {
          gdDebug( "Zim: Building the index for dictionary: %s\n", i->c_str() );

          unsigned articleCount = 0;
          unsigned wordCount = 0;

          df.open();
          ZIM_header const & zh = df.header();

          if( zh.magicNumber != 0x44D495A )
            throw exNotZimFile( i->c_str() );

          {
            int n = firstName.lastIndexOf( '/' );
            initializing.indexingDictionary( firstName.mid( n + 1 ).toUtf8().constData() );
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
              qint64 ret = df.read( reinterpret_cast< char * >( &redEntry ) + 2, sizeof(RedirectEntry) - 2 );
              if( ret != sizeof(RedirectEntry) - 2 )
                throw exCantReadFile( i->c_str() );

              nameSpace = redEntry.nameSpace;
            }
            else
            {
              artEntry.mimetype = mimetype;
              qint64 ret = df.read( reinterpret_cast< char * >( &artEntry ) + 2, sizeof(ArticleEntry) - 2 );
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
              wstring word;
              if( !title.empty() )
              {

                word = Utf8::decode( title );
                if( maxHeadwordsToExpand && zh.articleCount >= maxHeadwordsToExpand )
                  indexedWords.addSingleWord( word, n );
                else
                  indexedWords.addWord( word, n );
              }
              if( !url.empty() )
              {
                word          = Utf8::decode( url );

                // begin, the same process order as ZimDictionary::convert before findArticle's invocation
                QString qword = QString::fromStdU32String( word );
                QRegularExpression htmlRx( "\\.(s|)htm(l|)$", QRegularExpression::CaseInsensitiveOption );
                qword.remove( htmlRx ).replace( "_", " " ).remove( QRegularExpression( ".*/" ) );
                //end

                word = qword.toStdU32String();
                if( maxHeadwordsToExpand && zh.articleCount >= maxHeadwordsToExpand )
                  indexedWords.addSingleWord( word, n );
                else
                  indexedWords.addWord( word, n );
              }

              wordCount++;
            }
            else
            if( nameSpace == 'M' )
            {
              if( url.compare( "Title") == 0 )
              {
                idxHeader.namePtr = n;
                string name;
                readArticle( df, n, name );
                initializing.indexingDictionary( name );
              }
              else
              if( url.compare( "Description") == 0 )
                idxHeader.descriptionPtr = n;
              else
              if( url.compare( "Language") == 0 )
              {
                string lang;
                readArticle( df, n, lang );
                if( lang.size() == 2 )
                  idxHeader.langFrom = LangCoder::code2toInt( lang.c_str() );
                else
                if( lang.size() == 3 )
                  idxHeader.langFrom = LangCoder::findIdForLanguageCode3( lang.c_str() );
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

        dictionaries.push_back( new ZimDictionary( dictId,
                                                   indexFile,
                                                   dictFiles ) );
      }
      catch( std::exception & e )
      {
        gdWarning( "Zim dictionary initializing failed: %s, error: %s\n",
                   i->c_str(), e.what() );
        continue;
      }
      catch( ... )
      {
        qWarning( "Zim dictionary initializing failed\n" );
        continue;
      }
  }
  return dictionaries;
}

} // namespace Zim

#endif
