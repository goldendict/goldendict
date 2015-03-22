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
#include "tiff.hh"
#include "ftshelpers.hh"
#include "htmlescape.hh"

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
DEF_EX( exUserAbort, "User abort", Dictionary::Ex )


//namespace {

class ZimFile;

#pragma pack( push, 1 )

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

#pragma pack( pop )

// Class for support of split zim files

class ZimFile
{
  QVector< QFile * > files;
  QVector< quint64 > offsets;
  int currentFile;

public:

  ZimFile();
  ZimFile( const QString & name );
  ~ZimFile();

  void setFileName( const QString & name );
  void getFilenames( vector< string > & names );
  bool open( QFile::OpenMode mode );
  void close();
  bool seek( quint64 pos );
  qint64 read(  char * data, qint64 maxSize );
  QByteArray read( qint64 maxSize );
  bool getChar( char * c );
  qint64 size()
  { return files.isEmpty() ? 0 : offsets.last() + files.last()->size(); }
};

ZimFile::ZimFile() :
  currentFile( 0 )
{
}

ZimFile::ZimFile( const QString & name ) :
  currentFile( 0 )
{
  setFileName( name );
}

ZimFile::~ZimFile()
{
  close();
}

void ZimFile::setFileName( const QString & name )
{
  close();

  files.append( new QFile( name ) );
  offsets.append( 0 );

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

        quint64 offset = offsets.last() + files.last()->size();

        files.append( new QFile( fname ) );
        offsets.append( offset );
      }

      if( j < 26 )
        break;
    }
  }
}

void ZimFile::close()
{
  for( QVector< QFile * >::const_iterator i = files.begin(); i != files.end(); ++i )
  {
    (*i)->close();
    delete (*i);
  }

  files.clear();
  offsets.clear();

  currentFile = 0;
}

void ZimFile::getFilenames( vector< string > &names )
{
  for( QVector< QFile const * >::const_iterator i = files.begin(); i != files.end(); ++i )
    names.push_back( FsEncoding::encode( (*i)->fileName() ) );
}

bool ZimFile::open( QFile::OpenMode mode )
{
  for( QVector< QFile * >::iterator i = files.begin(); i != files.end(); ++i )
    if( !(*i)->open( mode ) )
    {
      close();
      return false;
    }

  return true;
}

bool ZimFile::seek( quint64 pos )
{
  int fileNom;

  for( fileNom = 0; fileNom < offsets.size() - 1; fileNom++ )
    if( pos < offsets.at( fileNom + 1 ) )
      break;

  pos -= offsets.at( fileNom );

  currentFile = fileNom;
  return files.at( fileNom )->seek( pos );
}

qint64 ZimFile::read( char *data, qint64 maxSize )
{
  quint64 bytesReaded = 0;
  for( int i = currentFile; i < files.size(); i++ )
  {
    if( i != currentFile )
      files.at( i )->seek( 0 );

    qint64 ret = files.at( i )->read( data + bytesReaded, maxSize );
    if( ret < 0 )
      break;

    bytesReaded += ret;
    maxSize -= ret;

    if( maxSize <= 0 )
      break;
  }
  return bytesReaded;
}

QByteArray ZimFile::read( qint64 maxSize )
{
  QByteArray data;
  data.resize( maxSize );

  qint64 ret = read( data.data(), maxSize );

  if( ret != maxSize )
    data.resize( ret );

  return data;
}

bool ZimFile::getChar( char *c )
{
  char ch;
  return read( c ? c : &ch, 1 ) == 1;
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

quint32 readArticle( ZimFile & file, ZIM_header & header, quint32 articleNumber, string & result,
                     set< quint32 > * loadedArticles = NULL )
{
  result.clear();

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

    result.append( decompressedData, offsets[ 0 ], size );

    return articleNumber;
  }
  return 0xFFFFFFFF;
}

// ZimDictionary

class ZimDictionary: public BtreeIndexing::BtreeDictionary
{
    Mutex idxMutex;
    Mutex zimMutex, idxResourceMutex;
    File::Class idx;
    BtreeIndex resourceIndex;
    IdxHeader idxHeader;
    string dictionaryName;
    ZimFile df;
    ZIM_header zimHeader;
    set< quint32 > articlesIndexedForFTS;

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

    virtual sptr< Dictionary::DataRequest > getSearchResults( QString const & searchString,
                                                              int searchMode, bool matchCase,
                                                              int distanceBetweenWords,
                                                              int maxResults );
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
      QString name = QDir::fromNativeSeparators( FsEncoding::decode( dictionaryFiles[ 0 ].c_str() ) );
      int n = name.lastIndexOf( '/' );
      dictionaryName = string( name.mid( n + 1 ).toUtf8().constData() );
    }
    else
    {
      readArticle( df, zimHeader, idxHeader.namePtr, dictionaryName );
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
    ret = readArticle( df, zimHeader, address, articleText, loadedArticles );
  }
  if( !rawText )
    articleText = convert( articleText );
  return ret;
}

string ZimDictionary::convert( const string & in )
{
  QString text = QString::fromUtf8( in.c_str() );

  // replace background
  text.replace( QRegExp( "<\\s*body\\s*([^>]*)(background(|-color)):([^;\"]*(|;))" ),
                QString( "<body \\1" ) );

  // pattern of img and script
  text.replace( QRegExp( "<\\s*(img|script)\\s*([^>]*)src=(\"|)(\\.\\.|)/" ),
                QString( "<\\1 \\2src=\\3bres://%1/").arg( getId().c_str() ) );

  // Fix links without '"'
  text.replace( QRegExp( "href=(\\.\\.|)/([^\\s>]+)" ), QString( "href=\"\\1/\\2\"" ) );

  // pattern <link... href="..." ...>
  text.replace( QRegExp( "<\\s*link\\s*([^>]*)href=\"(\\.\\.|)/" ),
                QString( "<link \\1href=\"bres://%1/").arg( getId().c_str() ) );

  // localize the http://en.wiki***.com|org/wiki/<key> series links
  // excluding those keywords that have ":" in it
  QString urlWiki = "\"http(s|)://en\\.(wiki(pedia|books|news|quote|source|voyage|versity)|wiktionary)\\.(org|com)/wiki/([^:\"]*)\"";
  text.replace( QRegExp( "<\\s*a\\s+(class=\"external\"\\s+|)href=" + urlWiki ),
                QString( "<a href=\"gdlookup://localhost/\\6\"" ) );

  // pattern <a href="..." ...>, excluding any known protocols such as http://, mailto:, #(comment)
  // these links will be translated into local definitions
  QRegExp rxLink( "<\\s*a\\s+([^>]*)href=\"(?!(\\w+://|#|mailto:|tel:))(/|)([^\"]*)\"\\s*(title=\"[^\"]*\")?[^>]*>",
                       Qt::CaseSensitive,
                       QRegExp::RegExp2 );
  int pos = 0;
  while( (pos = rxLink.indexIn( text, pos )) >= 0 )
  {
    QStringList list = rxLink.capturedTexts();
    QString tag = list[3];     // a url, ex: Precambrian_Chaotian.html
    if ( !list[4].isEmpty() )  // a title, ex: title="Precambrian/Chaotian"
      tag = list[4].split("\"")[1];

    tag.remove( QRegExp(".*/") ).
        remove( QRegExp( "\\.(s|)htm(l|)$", Qt::CaseInsensitive ) ).
        replace( "_", "%20" ).
        prepend( "<a href=\"gdlookup://localhost/" ).
        append( "\" " + list[4] + ">" );

    text.replace( pos, list[0].length(), tag );
    pos += tag.length() + 1;
  }

  // Occassionally words needs to be displayed in vertical, but <br/> were changed to <br\> somewhere
  // proper style: <a href="gdlookup://localhost/Neoptera" ... >N<br/>e<br/>o<br/>p<br/>t<br/>e<br/>r<br/>a</a>
  QRegExp rxBR( "(<a href=\"gdlookup://localhost/[^\"]*\"\\s*[^>]*>)\\s*((\\w\\s*&lt;br(\\\\|/|)&gt;\\s*)+\\w)\\s*</a>",
                       Qt::CaseSensitive,
                       QRegExp::RegExp2 );
  pos = 0;
  while( (pos = rxBR.indexIn( text, pos )) >= 0 )
  {
    QStringList list = rxBR.capturedTexts();
    QString tag = list[2];
    tag.replace( QRegExp( "&lt;br( |)(\\\\|/|)&gt;", Qt::CaseInsensitive ) , "<br/>" ).
        prepend( list[1] ).
        append( "</a>" );

    text.replace( pos, list[0].length(), tag );
    pos += tag.length() + 1;
  }

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

    findArticleLinks( 0, &setOfOffsets, 0, &isCancelled );

    if( isCancelled )
      throw exUserAbort();

    QVector< uint32_t > offsets;
    offsets.resize( setOfOffsets.size() );
    uint32_t * ptr = &offsets.front();

    for( QSet< uint32_t >::ConstIterator it = setOfOffsets.constBegin();
         it != setOfOffsets.constEnd(); ++it )
    {
      *ptr = *it;
      ptr++;
    }

    // Free memory
    setOfOffsets.clear();

    if( isCancelled )
      throw exUserAbort();

    qSort( offsets );

    if( isCancelled )
      throw exUserAbort();

    QMap< QString, QVector< uint32_t > > ftsWords;

    set< quint32 > indexedArticles;
    quint32 articleNumber;

    // index articles for full-text search
    for( int i = 0; i < offsets.size(); i++ )
    {
      if( isCancelled )
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
      if( isCancelled )
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

    if( isCancelled )
      throw exUserAbort();

    ftsIdxHeader.chunksOffset = chunks.finish();
    ftsIdxHeader.wordCount = indexedWords.size();

    if( isCancelled )
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
                                                                 int maxResults )
{
  return new FtsHelpers::FTSResultsRequest( *this, searchString,searchMode, matchCase, distanceBetweenWords, maxResults );
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

#ifdef MAKE_EXTRA_TIFF_HANDLER
      if( img.isNull() )
        GdTiff::tiffToQImage( &data.front(), data.size(), img );
#endif

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
  catch( std::exception &ex )
  {
    gdWarning( "ZIM: Failed loading resource \"%s\" from \"%s\", reason: %s\n",
               resourceName.c_str(), dict.getName().c_str(), ex.what() );
    // Resource not loaded -- we don't set the hasAnyData flag then
  }

  finish();
}

sptr< Dictionary::DataRequest > ZimDictionary::getResource( string const & name )
  throw( std::exception )
{
  return new ZimResourceRequest( *this, name );
}

//} // anonymous namespace

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

          ZIM_header zh;

          unsigned articleCount = 0;
          unsigned wordCount = 0;

          df.open( QFile::ReadOnly );

          qint64 ret = df.read( reinterpret_cast< char * >( &zh ), sizeof( zh ) );
          if( ret != sizeof( zh ) )
            throw exCantReadFile( i->c_str() );

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
