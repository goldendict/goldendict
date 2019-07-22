/* This file is (c) 2015 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifdef MAKE_ZIM_SUPPORT

#include "slob.hh"
#include "btreeidx.hh"
#include "fsencoding.hh"
#include "folding.hh"
#include "gddebug.hh"
#include "utf8.hh"
#include "decompress.hh"
#include "langcoder.hh"
#include "wstring.hh"
#include "wstring_qt.hh"
#include "ftshelpers.hh"
#include "htmlescape.hh"
#include "filetype.hh"
#include "tiff.hh"
#include "qt4x5.hh"

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextCodec>
#include <QMap>
#include <QPair>
#include <QRegExp>
#include <QProcess>

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
#include <QRegularExpression>
#endif

#include <string>
#include <vector>
#include <map>
#include <set>

namespace Slob {

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

DEF_EX_STR( exNotSlobFile, "Not an Slob file", Dictionary::Ex )
DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )
DEF_EX_STR( exCantDecodeFile, "Can't decode file", Dictionary::Ex )
DEF_EX_STR( exNoCodecFound, "No text codec found", Dictionary::Ex )
DEF_EX( exUserAbort, "User abort", Dictionary::Ex )
DEF_EX( exNoResource, "No resource found", Dictionary::Ex )

#pragma pack( push, 1 )

enum
{
  Signature = 0x58424C53, // SLBX on little-endian, XBLS on big-endian
  CurrentFormatVersion = 2 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  quint32 signature; // First comes the signature, SLBX
  quint32 formatVersion; // File format version (CurrentFormatVersion)
  quint32 indexBtreeMaxElements; // Two fields from IndexInfo
  quint32 indexRootOffset;
  quint32 resourceIndexBtreeMaxElements; // Two fields from IndexInfo
  quint32 resourceIndexRootOffset;
  quint32 wordCount;
  quint32 articleCount;
  quint32 langFrom;  // Source language
  quint32 langTo;    // Target language
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

#pragma pack( pop )

const char SLOB_MAGIC[ 8 ] = { 0x21, 0x2d, 0x31, 0x53, 0x4c, 0x4f, 0x42, 0x1f };

struct RefEntry
{
  QString key;
  quint32 itemIndex;
  quint16 binIndex;
  QString fragment;
};

bool indexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion;
}


class SlobFile
{
  enum Compressions
  { UNKNOWN = 0, NONE, ZLIB, BZ2, LZMA2 };

  QFile file;
  QString fileName, dictionaryName;
  Compressions compression;
  QString encoding;
  unsigned char uuid[ 16 ];
  QTextCodec *codec;
  QMap< QString, QString > tags;
  QVector< QString > contentTypes;
  quint32 blobCount;
  quint64 storeOffset, fileSize, refsOffset;
  quint32 refsCount, itemsCount;
  quint64 itemsOffset, itemsDataOffset;
  quint32 currentItem;
  quint32 contentTypesCount;
  string currentItemData;

  QString readTinyText();
  QString readText();
  QString readLargeText();
  QString readString( unsigned length );

public:
  SlobFile() :
    compression( UNKNOWN )
  , codec( 0 )
  , blobCount( 0 )
  , storeOffset( 0 )
  , fileSize( 0 )
  , refsOffset( 0 )
  , refsCount( 0 )
  , itemsCount( 0 )
  , itemsOffset( 0 )
  , itemsDataOffset( 0 )
  , currentItem( 0xFFFFFFFF )
  , contentTypesCount( 0 )
  {}

  ~SlobFile();

  Compressions getCompression() const
  { return compression; }

  QString const & getEncoding() const
  { return encoding; }

  QString const & getDictionaryName() const
  { return dictionaryName; }

  quint32 blobsCount() const
  { return blobCount; }

  quint64 dataOffset() const
  { return storeOffset; }

  quint32 getRefsCount() const
  { return refsCount; }

  quint32 getContentTypesCount() const
  { return contentTypesCount; }

  QTextCodec * getCodec() const
  { return codec; }

  QString getContentType( quint8 content_id ) const
  { return content_id < contentTypes.size() ? contentTypes[ content_id ] : QString(); }

  QMap< QString, QString > const & getTags() const
  { return tags; }

  void open( const QString & name );

  void getRefEntry( quint32 ref_nom, RefEntry & entry );

  quint8 getItem( RefEntry const & entry, string * data );
};

SlobFile::~SlobFile()
{
  file.close();
}

QString SlobFile::readString( unsigned length )
{
  QByteArray data = file.read( length );
  QString str;

  if( codec != 0 && !data.isEmpty() )
    str = codec->toUnicode( data );
  else
    str = QString( data );

  char term = 0;
  int n = str.indexOf( term );
  if( n >= 0 )
    str.resize( n );

  return str;
}

QString SlobFile::readTinyText()
{
  unsigned char len;
  if( !file.getChar( ( char * )&len ) )
  {
    QString error = fileName + ": " + file.errorString();
    throw exCantReadFile( string( error.toUtf8().data() ) );
  }
  return readString( len );
}

QString SlobFile::readText()
{
  quint16 len;
  if( file.read( ( char * )&len, sizeof( len ) ) != sizeof( len ) )
  {
    QString error = fileName + ": " + file.errorString();
    throw exCantReadFile( string( error.toUtf8().data() ) );
  }
  return readString( qFromBigEndian( len ) );
}

QString SlobFile::readLargeText()
{
  quint32 len;
  if( file.read( ( char * )&len, sizeof( len ) ) != sizeof( len ) )
  {
    QString error = fileName + ": " + file.errorString();
    throw exCantReadFile( string( error.toUtf8().data() ) );
  }
  return readString( qFromBigEndian( len ) );
}

void SlobFile::open( const QString & name )
{
QString error( name + ": " );

  if( file.isOpen() )
    file.close();

  fileName = name;

  file.setFileName( name );

  {
    QFileInfo fi( name );
    dictionaryName = fi.fileName();
  }

  for( ; ; )
  {

    if( !file.open( QFile::ReadOnly ) )
      break;

    char magic[ 8 ];
    if( file.read( magic, sizeof( magic ) ) != sizeof( magic ) )
      break;

    if( memcmp( magic, SLOB_MAGIC, sizeof( magic ) ) != 0 )
      throw exNotSlobFile( string( name.toUtf8().data() ) );

    if( file.read( ( char * )uuid, sizeof( uuid ) ) != sizeof( uuid ) )
      break;

    // Read encoding

    encoding = readTinyText();

    codec = QTextCodec::codecForName( encoding.toLatin1() );
    if( codec == 0 )
    {
      error = QString( "for encoding \"") + encoding + "\"";
      throw exNoCodecFound( string( error.toUtf8().data() ) );
    }

    // Read compression type

    QString compr = readTinyText();

    if( compr.compare( "zlib", Qt::CaseInsensitive ) == 0 )
      compression = ZLIB;
    else
    if( compr.compare( "bz2", Qt::CaseInsensitive ) == 0 )
      compression = BZ2;
    else
    if( compr.compare( "lzma2", Qt::CaseInsensitive ) == 0 )
      compression = LZMA2;
    else
    if( compr.isEmpty() || compr.compare( "none", Qt::CaseInsensitive ) == 0 )
      compression = NONE;

    // Read tags

    unsigned char count;
    if( !file.getChar( ( char * )&count ) )
      break;

    for( unsigned i = 0; i < count; i++ )
    {
      QString key = readTinyText();
      QString value = readTinyText();
      tags[ key ] = value;

      if( key.compare( "label", Qt::CaseInsensitive ) == 0
          || key.compare( "name", Qt::CaseInsensitive ) == 0)
        dictionaryName = value;
    }

    // Read content types

    if( !file.getChar( ( char * )&count ) )
      break;

    for( unsigned i = 0; i < count; i++ )
    {
      QString type = readText();
      contentTypes.append( type );
    }
    contentTypesCount = count;

    // Read data parameters

    quint32 cnt;
    if( file.read( ( char * )&cnt, sizeof( cnt ) ) != sizeof( cnt ) )
      break;
    blobCount = qFromBigEndian( cnt );

    quint64 tmp;
    if( file.read( ( char * )&tmp, sizeof( tmp ) ) != sizeof( tmp ) )
      break;
    storeOffset = qFromBigEndian( tmp );

    if( file.read( ( char * )&tmp, sizeof( tmp ) ) != sizeof( tmp ) )
      break;
    fileSize = qFromBigEndian( tmp );

    if( file.read( ( char * )&cnt, sizeof( cnt ) ) != sizeof( cnt ) )
      break;
    refsCount = qFromBigEndian( cnt );

    refsOffset = file.pos();

    if( !file.seek( storeOffset ) )
      break;

    if( file.read( ( char * )&cnt, sizeof( cnt ) ) != sizeof( cnt ) )
      break;
    itemsCount = qFromBigEndian( cnt );

    itemsOffset = storeOffset + sizeof( itemsCount );
    itemsDataOffset = itemsOffset + itemsCount * sizeof( quint64 );

    return;
  }
  error += file.errorString();
  throw exCantReadFile( string( error.toUtf8().data() ) );
}

void SlobFile::getRefEntry( quint32 ref_nom, RefEntry & entry )
{
  quint64 pos = refsOffset + ref_nom * sizeof( quint64 );
  quint64 offset, tmp;

  for( ; ; )
  {
    if( !file.seek( pos ) || file.read( ( char * )&tmp, sizeof( tmp ) ) != sizeof( tmp ) )
      break;
    offset = qFromBigEndian( tmp ) + refsOffset + refsCount * sizeof( quint64 );

    if( !file.seek( offset ) )
      break;

    entry.key = readText();

    quint32 index;
    if( file.read( ( char * )&index, sizeof( index ) ) != sizeof( index ) )
      break;
    entry.itemIndex = qFromBigEndian( index );

    quint16 binIndex;
    if( file.read( ( char * )&binIndex, sizeof( binIndex ) ) != sizeof( binIndex ) )
      break;
    entry.binIndex = qFromBigEndian( binIndex );

    entry.fragment = readTinyText();

    return;
  }
  QString error = fileName + ": " + file.errorString();
  throw exCantReadFile( string( error.toUtf8().data() ) );
}

quint8 SlobFile::getItem( RefEntry const & entry, string * data )
{
  quint64 pos = itemsOffset + entry.itemIndex * sizeof( quint64 );
  quint64 offset, tmp;

  for( ; ; )
  {
    // Read item data types

    if( !file.seek( pos ) || file.read( ( char * )&tmp, sizeof( tmp ) ) != sizeof( tmp ) )
      break;

    offset = qFromBigEndian( tmp ) + itemsDataOffset;

    if( !file.seek( offset ) )
      break;

    quint32 bins, bins_be;
    if( file.read( ( char * )&bins_be, sizeof( bins_be ) ) != sizeof( bins_be ) )
      break;
    bins = qFromBigEndian( bins_be );

    if( entry.binIndex >= bins )
      return 0xFF;

    QVector< quint8 > ids;
    ids.resize( bins );
    if( file.read( ( char * )ids.data(), bins ) != bins )
      break;

    quint8 id = ids[ entry.binIndex ];

    if( id >= (unsigned)contentTypes.size() )
      return 0xFF;

    if( data != 0 )
    {
      // Read item data
      if( currentItem != entry.itemIndex )
      {
        currentItemData.clear();
        quint32 length, length_be;
        if( file.read( ( char * )&length_be, sizeof( length_be ) ) != sizeof( length_be ) )
          break;
        length = qFromBigEndian( length_be );

        QByteArray compressedData = file.read( length );

        if( compression == NONE )
          currentItemData = compressedData.toStdString();
        else
        if( compression == ZLIB )
          currentItemData = decompressZlib( compressedData.data(), length );
        else
        if( compression == BZ2 )
          currentItemData = decompressBzip2( compressedData.data(), length );
        else
          currentItemData = decompressLzma2( compressedData.data(), length, true );

        if( currentItemData.empty() )
        {
          currentItem = 0xFFFFFFFF;
          return 0xFF;
        }
        currentItem = entry.itemIndex;
      }

      // Find bin data inside item

      const char * ptr = currentItemData.c_str();
      quint32 pos = entry.binIndex * sizeof( quint32 );

      if( pos >= currentItemData.length() - sizeof( quint32 ) )
        return 0xFF;

      quint32 offset, offset_be;
      memcpy( &offset_be, ptr + pos, sizeof( offset_be ) );
      offset = qFromBigEndian( offset_be );

      pos = bins * sizeof( quint32 ) + offset;

      if( pos >= currentItemData.length() - sizeof( quint32 ) )
        return 0xFF;

      quint32 length, len_be;
      memcpy( &len_be, ptr + pos, sizeof( len_be ) );
      length = qFromBigEndian( len_be );

      *data = currentItemData.substr( pos + sizeof( len_be ), length );
    }

    return ids[ entry.binIndex ];
  }
  QString error = fileName + ": " + file.errorString();
  throw exCantReadFile( string( error.toUtf8().data() ) );
}

// SlobDictionary

class SlobDictionary: public BtreeIndexing::BtreeDictionary
{
    Mutex idxMutex;
    Mutex slobMutex, idxResourceMutex;
    File::Class idx;
    BtreeIndex resourceIndex;
    IdxHeader idxHeader;
    string dictionaryName;
    SlobFile sf;
    QString texCgiPath, texCachePath;

  public:

    SlobDictionary( string const & id, string const & indexFile,
                    vector< string > const & dictionaryFiles );

    ~SlobDictionary();

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
      THROW_SPEC( std::exception );

    virtual sptr< Dictionary::DataRequest > getResource( string const & name )
      THROW_SPEC( std::exception );

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

    virtual void makeFTSIndex(QAtomicInt & isCancelled, bool firstIteration );

    virtual void setFTSParameters( Config::FullTextSearch const & fts )
    {
      can_FTS = fts.enabled
                && !fts.disabledTypes.contains( "SLOB", Qt::CaseInsensitive )
                && ( fts.maxDictionarySize == 0 || getArticleCount() <= fts.maxDictionarySize );
    }

    virtual uint32_t getFtsIndexVersion()
    { return 2; }

protected:

    virtual void loadIcon() throw();

private:

    /// Loads the article.
    void loadArticle( quint32 address,
                      string & articleText );

    quint32 readArticle( quint32 address,
                         string & articleText,
                         RefEntry & entry );

    string convert( string const & in_data, RefEntry const & entry );

    void removeDirectory( QString const & directory );

    friend class SlobArticleRequest;
    friend class SlobResourceRequest;
};

SlobDictionary::SlobDictionary( string const & id,
                                string const & indexFile,
                                vector< string > const & dictionaryFiles ):
    BtreeDictionary( id, dictionaryFiles ),
    idx( indexFile, "rb" ),
    idxHeader( idx.read< IdxHeader >() )
{
    // Open data file

    try
    {
      sf.open( FsEncoding::decode( dictionaryFiles[ 0 ].c_str() ) );
    }
    catch( std::exception & e )
    {
      gdWarning( "Slob dictionary initializing failed: %s, error: %s\n",
                 dictionaryFiles[ 0 ].c_str(), e.what() );
    }

    // Initialize the indexes

    openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                          idxHeader.indexRootOffset ),
               idx, idxMutex );

    resourceIndex.openIndex( IndexInfo( idxHeader.resourceIndexBtreeMaxElements,
                                        idxHeader.resourceIndexRootOffset ),
                             idx, idxResourceMutex );

    // Read dictionary name

    dictionaryName = string( sf.getDictionaryName().toUtf8().constData() );
    if( dictionaryName.empty() )
    {
      QString name = QDir::fromNativeSeparators( FsEncoding::decode( dictionaryFiles[ 0 ].c_str() ) );
      int n = name.lastIndexOf( '/' );
      dictionaryName = string( name.mid( n + 1 ).toUtf8().constData() );
    }

    // Full-text search parameters

    can_FTS = true;

    ftsIdxName = indexFile + "_FTS";

    if( !Dictionary::needToRebuildIndex( dictionaryFiles, ftsIdxName )
        && !FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName, this ) )
      FTS_index_completed.ref();

    texCgiPath = Config::getProgramDataDir() + "/mimetex.cgi";
    if( QFileInfo( texCgiPath ).exists() )
    {
      QString dirName = QString::fromStdString( getId() );
      QDir( QDir::tempPath() ).mkdir( dirName );
      texCachePath = QDir::tempPath() + "/" + dirName;
    }
    else
      texCgiPath.clear();
}

SlobDictionary::~SlobDictionary()
{
  if( !texCachePath.isEmpty() )
    removeDirectory( texCachePath );
}

void SlobDictionary::removeDirectory( QString const & directory )
{
  QDir dir( directory );
  Q_FOREACH( QFileInfo info, dir.entryInfoList( QDir::NoDotAndDotDot
                                                | QDir::AllDirs
                                                | QDir::Files,
                                                QDir::DirsFirst))
  {
    if( info.isDir() )
      removeDirectory( info.absoluteFilePath() );
    else
      QFile::remove( info.absoluteFilePath() );
  }

  dir.rmdir( directory );
}

void SlobDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension
  fileName.chop( 4 );

  if( !loadIconFromFile( fileName ) )
  {
    // Load failed -- use default icons
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/icon32_slob.png");
  }

  dictionaryIconLoaded = true;
}

QString const& SlobDictionary::getDescription()
{
  if( !dictionaryDescription.isEmpty() )
    return dictionaryDescription;

  QMap< QString, QString > const & tags = sf.getTags();

  QMap< QString, QString >::const_iterator it;
  for( it = tags.begin(); it != tags.end(); ++it )
  {
    if( it != tags.begin() )
      dictionaryDescription += "\n\n";

    dictionaryDescription += it.key() + ": " +it.value();
  }

  return dictionaryDescription;
}

void SlobDictionary::loadArticle( quint32 address,
                                  string & articleText )
{
  articleText.clear();

  RefEntry entry;

  readArticle( address, articleText, entry );

  if( !articleText.empty() )
  {
    articleText = convert( articleText, entry );
  }
  else
    articleText = string( QObject::tr( "Article decoding error" ).toUtf8().constData() );

  // See Issue #271: A mechanism to clean-up invalid HTML cards.
  string cleaner = "</font>""</font>""</font>""</font>""</font>""</font>"
                   "</font>""</font>""</font>""</font>""</font>""</font>"
                   "</b></b></b></b></b></b></b></b>"
                   "</i></i></i></i></i></i></i></i>"
                   "</a></a></a></a></a></a></a></a>";

  string prefix( "<div class=\"slobdict\"" );
  if( isToLanguageRTL() )
    prefix += " dir=\"rtl\"";
  prefix += ">";

  articleText = prefix + articleText + cleaner + "</div>";
}

string SlobDictionary::convert( const string & in, RefEntry const & entry )
{
  QString text = QString::fromUtf8( in.c_str() );

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  // pattern of img and script
  text.replace( QRegularExpression( "<\\s*(img|script)\\s+([^>]*)src=\"(?!(?:data|https?|ftp):)(|/)([^\"]*)\"" ),
                QString( "<\\1 \\2src=\"bres://%1/\\4\"").arg( getId().c_str() ) );

  // pattern <link... href="..." ...>
  text.replace( QRegularExpression( "<\\s*link\\s+([^>]*)href=\"(?!(?:data|https?|ftp):)" ),
                QString( "<link \\1href=\"bres://%1/").arg( getId().c_str() ) );
#else
  // pattern of img and script
  text.replace( QRegExp( "<\\s*(img|script)\\s+([^>]*)src=\"(?!(?:data|https?|ftp):)(|/)([^\"]*)\"" ),
                QString( "<\\1 \\2src=\"bres://%1/\\4\"").arg( getId().c_str() ) );

  // pattern <link... href="..." ...>
  text.replace( QRegExp( "<\\s*link\\s+([^>]*)href=\"(?!(?:data|https?|ftp):)" ),
                QString( "<link \\1href=\"bres://%1/").arg( getId().c_str() ) );
#endif

  // pattern <a href="..." ...>, excluding any known protocols such as http://, mailto:, #(comment)
  // these links will be translated into local definitions
  QString anchor;
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  QRegularExpression rxLink( "<\\s*a\\s+([^>]*)href=\"(?!(?:\\w+://|#|mailto:|tel:))(/|)([^\"]*)\"\\s*(title=\"[^\"]*\")?[^>]*>" );
  QRegularExpressionMatchIterator it = rxLink.globalMatch( text );
  int pos = 0;
  QString newText;
  while( it.hasNext() )
  {
    QRegularExpressionMatch match = it.next();

    newText += text.midRef( pos, match.capturedStart() - pos );
    pos = match.capturedEnd();

    QStringList list = match.capturedTexts();
    // Add empty strings for compatibility with QRegExp behaviour
    for( int i = match.lastCapturedIndex() + 1; i < 5; i++ )
      list.append( QString() );
#else
  QRegExp rxLink( "<\\s*a\\s+([^>]*)href=\"(?!(\\w+://|#|mailto:|tel:))(/|)([^\"]*)\"\\s*(title=\"[^\"]*\")?[^>]*>",
                       Qt::CaseSensitive,
                       QRegExp::RegExp2 );

  int pos = 0;
  while( (pos = rxLink.indexIn( text, pos )) >= 0 )
  {
    QStringList list = rxLink.capturedTexts();
#endif
    QString tag = list[3];
    if ( !list[4].isEmpty() )
      tag = list[4].split("\"")[1];

    // Find anchor
    int n = list[ 3 ].indexOf( '#' );
    if( n > 0 )
      anchor = QString( "?gdanchor=" ) + list[ 3 ].mid( n + 1 );
    else
      anchor.clear();

    tag.remove( QRegExp(".*/") ).
        remove( QRegExp( "\\.(s|)htm(l|)$", Qt::CaseInsensitive ) ).
        replace( "_", "%20" ).
        prepend( "<a href=\"gdlookup://localhost/" ).
        append( anchor + "\" " + list[4] + ">" );

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    newText += tag;
  }
  if( pos )
  {
    newText += text.midRef( pos );
    text = newText;
  }
  newText.clear();
#else
    text.replace( pos, list[0].length(), tag );
    pos += tag.length() + 1;
  }
#endif

  // Handle TeX formulas via mimetex.cgi

  if( !texCgiPath.isEmpty() )
  {
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
      QRegularExpression texImage( "<\\s*img\\s+class=\"([^\"]+)\"\\s*([^>]*)alt=\"([^\"]+)\"[^>]*>" );
      QRegularExpression regFrac( "\\\\[dt]frac" );
      QRegularExpression regSpaces( "\\s+([\\{\\(\\[\\}\\)\\]])" );
#else
    QRegExp texImage( "<\\s*img\\s+class=\"([^\"]+)\"\\s*([^>]*)alt=\"([^\"]+)\"[^>]*>",
                      Qt::CaseSensitive,
                      QRegExp::RegExp2 );
    QRegExp regFrac = QRegExp( "\\\\[dt]frac" );
    QRegExp regSpaces = QRegExp( "\\s+([\\{\\(\\[\\}\\)\\]])", Qt::CaseSensitive, QRegExp::RegExp2 );
#endif
    QRegExp multReg = QRegExp( "\\*\\{(\\d+)\\}([^\\{]|\\{([^\\}]+)\\})", Qt::CaseSensitive, QRegExp::RegExp2 );

    QString arrayDesc( "\\begin{array}{" );
    pos = 0;
    unsigned texCount = 0;
    QString imgName;

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    QRegularExpressionMatchIterator it = texImage.globalMatch( text );
    QString newText;
    while( it.hasNext() )
    {
      QRegularExpressionMatch match = it.next();

      newText += text.midRef( pos, match.capturedStart() - pos );
      pos = match.capturedEnd();

      QStringList list = match.capturedTexts();
#else
    while( (pos = texImage.indexIn( text, pos )) >= 0 )
    {
      QStringList list = texImage.capturedTexts();
#endif

      if( list[ 1 ].compare( "tex" ) == 0
          || list[ 1 ].compare( "mwe-math-fallback-image-inline" ) == 0
          || list[ 1 ].endsWith( " tex" ) )
      {
        QString name;
        name.sprintf( "%04X%04X%04X.gif", entry.itemIndex, entry.binIndex, texCount );
        imgName = texCachePath + "/" + name;

        if( !QFileInfo( imgName ).exists() )
        {

          // Replace some TeX commands which don't support by mimetex.cgi

          QString tex = list[ 3 ];
          tex.replace( regSpaces, "\\1" );
          tex.replace( regFrac, "\\frac" );
          tex.replace( "\\leqslant", "\\leq" );
          tex.replace( "\\geqslant", "\\geq" );
          tex.replace( "\\infin", "\\infty" );
          tex.replace( "\\iff", "\\Longleftrightarrow" );
          tex.replace( "\\tbinom", "\\binom" );
          tex.replace( "\\implies", "\\Longrightarrow" );
          tex.replace( "{aligned}", "{align*}" );
          tex.replace( "\\Subset", "\\subset" );
          tex.replace( "\\xrightarrow", "\\longrightarrow^" );
          tex.remove( "\\scriptstyle" );
          tex.remove( "\\mathop" );
          tex.replace( "\\bigg|", "|" );

          // Format array descriptions (mimetex now don't support *{N}x constructions in it)

          int pos1 = 0;
          while( pos1 >= 0 )
          {
            pos1 = tex.indexOf( arrayDesc, pos1, Qt::CaseInsensitive );
            if( pos1 >= 0 )
            {
              // Retrieve array description
              QString desc, newDesc;
              int n = 0;
              int nstart = pos1 + arrayDesc.size();
              int i;
              for( i = 0; i + nstart < tex.size(); i++ )
              {
                if( tex[ i + nstart ] == '{' )
                  n += 1;
                if( tex[ i + nstart ] == '}' )
                  n -= 1;
                if( n < 0 )
                  break;
              }
              if( i > 0 && i + nstart + 1 < tex.size() )
                desc = tex.mid( nstart, i );

              if( !desc.isEmpty() )
              {
                // Expand multipliers: "*{5}x" -> "xxxxx"

                newDesc = desc;
                QString newStr;
                int pos2 = 0;
                while( pos2 >= 0 )
                {
                  pos2 = multReg.indexIn( newDesc, pos2 );
                  if( pos2 >= 0 )
                  {
                    QStringList list = multReg.capturedTexts();
                    int n = list[ 1 ].toInt();
                    for( int i = 0; i < n; i++ )
                      newStr += list[ 3 ].isEmpty() ? list[ 2 ] : list[ 3 ];
                    newDesc.replace( pos2, list[ 0 ].size(), newStr );
                    pos2 += newStr.size();
                  }
                  else
                    break;
                }
                tex.replace( pos1 + arrayDesc.size(), desc.size(), newDesc );
                pos1 += arrayDesc.size() + newDesc.size();
              }
              else
                pos1 += arrayDesc.size();
            }
            else
              break;
          }

          QString command = texCgiPath + " -e " +  imgName
                            + " \"" + tex + "\"";
          QProcess::execute( command );
        }

        QString tag = QString( "<img class=\"imgtex\" src=\"file://" )
#ifdef Q_OS_WIN32
                      + "/"
#endif
                      + imgName + "\" alt=\"" + list[ 3 ] + "\">";

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
        newText += tag;
#else
        text.replace( pos, list[0].length(), tag );
        pos += tag.length() + 1;
#endif

        texCount += 1;
      }
      else
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
        newText += list[ 0 ];
#else
        pos += list[ 0 ].length();
#endif
    }
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    if( pos )
    {
      newText += text.midRef( pos );
      text = newText;
    }
    newText.clear();
#endif
  }
#ifdef Q_OS_WIN32
  else
  {
    // Increase equations scale
    text = QString::fromLatin1( "<script type=\"text/x-mathjax-config\">MathJax.Hub.Config({" )
           + " SVG: { scale: 170, linebreaks: { automatic:true } }"
           + ", \"HTML-CSS\": { scale: 210, linebreaks: { automatic:true } }"
           + ", CommonHTML: { scale: 210, linebreaks: { automatic:true } }"
           + " });</script>"
           + text;
  }
#endif

  // Fix outstanding elements
  text += "<br style=\"clear:both;\" />";

  return text.toUtf8().data();
}

void SlobDictionary::loadResource( std::string & resourceName, string & data )
{
  vector< WordArticleLink > link;
  RefEntry entry;

  link = resourceIndex.findArticles( Utf8::decode( resourceName ) );

  if( link.empty() )
    return;

  readArticle( link[ 0 ].articleOffset, data, entry );
}

quint32 SlobDictionary::readArticle( quint32 articleNumber, std::string & result,
                                     RefEntry & entry )
{
  string data;
  quint8 contentId;

  {
    Mutex::Lock _( slobMutex );
    if( entry.key.isEmpty() )
      sf.getRefEntry( articleNumber, entry );
    contentId = sf.getItem( entry, &data );
  }

  if( contentId == 0xFF )
    return 0xFFFFFFFF;

  QString contentType = sf.getContentType( contentId );

  if( contentType.contains( "text/html", Qt::CaseInsensitive )
      || contentType.contains( "text/plain", Qt::CaseInsensitive )
      || contentType.contains( "/css", Qt::CaseInsensitive )
      || contentType.contains( "/javascript", Qt::CaseInsensitive )
      || contentType.contains( "/json", Qt::CaseInsensitive ))
  {
    QTextCodec *codec = sf.getCodec();
    QString content = codec->toUnicode( data.c_str(), data.size() );
    result = string( content.toUtf8().data() );
  }
  else
    result = data;

  return contentId;
}

void SlobDictionary::makeFTSIndex( QAtomicInt & isCancelled, bool firstIteration )
{
  if( !( Dictionary::needToRebuildIndex( getDictionaryFilenames(), ftsIdxName )
         || FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName, this ) ) )
    FTS_index_completed.ref();

  if( haveFTSIndex() )
    return;

  if( ensureInitDone().size() )
    return;

  if( firstIteration && getArticleCount() > FTS::MaxDictionarySizeForFastSearch )
    return;

  gdDebug( "Slob: Building the full-text index for dictionary: %s\n",
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

    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
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

    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    qSort( offsets );

    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    QMap< QString, QVector< uint32_t > > ftsWords;

    set< quint64 > indexedArticles;
    RefEntry entry;
    string articleText;
    quint32 htmlType = 0xFFFFFFFF;

    for( unsigned i = 0; i < sf.getContentTypesCount(); i++ )
    {
      if( sf.getContentType( i ).startsWith( "text/html", Qt::CaseInsensitive ) )
      {
        htmlType = i;
        break;
      }
    }

    // index articles for full-text search
    for( int i = 0; i < offsets.size(); i++ )
    {
      if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
        throw exUserAbort();

      QString articleStr;
      quint32 articleNom = offsets.at( i );

      {
        Mutex::Lock _( slobMutex );
        sf.getRefEntry( articleNom, entry );
      }

      quint64 articleID = ( ( (quint64)entry.itemIndex ) << 32 ) | entry.binIndex;

      set< quint64 >::iterator it = indexedArticles.find( articleID );
      if( it != indexedArticles.end() )
        continue;

      indexedArticles.insert( articleID );

      quint32 type = readArticle( 0, articleText, entry );

      articleStr = QString::fromUtf8( articleText.c_str(), articleText.length() );

      if( type == htmlType )
        articleStr = Html::unescape( articleStr );

      FtsHelpers::parseArticleForFts( articleNom, articleStr, ftsWords );
    }

    // Free memory
    offsets.clear();

    QMap< QString, QVector< uint32_t > >::iterator it = ftsWords.begin();
    while( it != ftsWords.end() )
    {
      if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
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

    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    ftsIdxHeader.chunksOffset = chunks.finish();
    ftsIdxHeader.wordCount = indexedWords.size();

    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
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
    gdWarning( "Slob: Failed building full-text search index for \"%s\", reason: %s\n", getName().c_str(), ex.what() );
    QFile::remove( FsEncoding::decode( ftsIdxName.c_str() ) );
  }
}

void SlobDictionary::getArticleText( uint32_t articleAddress, QString & headword, QString & text )
{
  try
  {
    RefEntry entry;
    string articleText;

    quint32 htmlType = 0xFFFFFFFF;

    for( unsigned i = 0; i < sf.getContentTypesCount(); i++ )
    {
      if( sf.getContentType( i ).startsWith( "text/html", Qt::CaseInsensitive ) )
      {
        htmlType = i;
        break;
      }
    }

    quint32 type = readArticle( articleAddress, articleText, entry );
    headword = entry.key;

    text = QString::fromUtf8( articleText.data(), articleText.size() );

    if( type == htmlType )
      text = Html::unescape( text );
  }
  catch( std::exception &ex )
  {
    gdWarning( "Slob: Failed retrieving article from \"%s\", reason: %s\n", getName().c_str(), ex.what() );
  }
}


sptr< Dictionary::DataRequest > SlobDictionary::getSearchResults( QString const & searchString,
                                                                  int searchMode, bool matchCase,
                                                                  int distanceBetweenWords,
                                                                  int maxResults,
                                                                  bool ignoreWordsOrder,
                                                                  bool ignoreDiacritics )
{
  return new FtsHelpers::FTSResultsRequest( *this, searchString, searchMode, matchCase, distanceBetweenWords, maxResults, ignoreWordsOrder, ignoreDiacritics );
}


/// SlobDictionary::getArticle()

class SlobArticleRequest;

class SlobArticleRequestRunnable: public QRunnable
{
  SlobArticleRequest & r;
  QSemaphore & hasExited;

public:

  SlobArticleRequestRunnable( SlobArticleRequest & r_,
                              QSemaphore & hasExited_ ): r( r_ ),
                                                         hasExited( hasExited_ )
  {}

  ~SlobArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class SlobArticleRequest: public Dictionary::DataRequest
{
  friend class SlobArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  SlobDictionary & dict;
  bool ignoreDiacritics;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  SlobArticleRequest( wstring const & word_,
                      vector< wstring > const & alts_,
                      SlobDictionary & dict_, bool ignoreDiacritics_ ):
    word( word_ ), alts( alts_ ), dict( dict_ ), ignoreDiacritics( ignoreDiacritics_ )
  {
    QThreadPool::globalInstance()->start(
      new SlobArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by DslArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~SlobArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void SlobArticleRequestRunnable::run()
{
  r.run();
}

void SlobArticleRequest::run()
{
  if ( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
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
    if ( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
    {
      finish();
      return;
    }

    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    // Now grab that article

    string headword, articleText;

    headword = chain[ x ].word;
    try
    {
      dict.loadArticle( chain[ x ].articleOffset, articleText );
    }
    catch(...)
    {
    }

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

sptr< Dictionary::DataRequest > SlobDictionary::getArticle( wstring const & word,
                                                            vector< wstring > const & alts,
                                                            wstring const &,
                                                            bool ignoreDiacritics )
  THROW_SPEC( std::exception )
{
  return new SlobArticleRequest( word, alts, *this, ignoreDiacritics );
}

//// SlobDictionary::getResource()

class SlobResourceRequest;

class SlobResourceRequestRunnable: public QRunnable
{
  SlobResourceRequest & r;
  QSemaphore & hasExited;

public:

  SlobResourceRequestRunnable( SlobResourceRequest & r_,
                               QSemaphore & hasExited_ ): r( r_ ),
                                                          hasExited( hasExited_ )
  {}

  ~SlobResourceRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class SlobResourceRequest: public Dictionary::DataRequest
{
  friend class SlobResourceRequestRunnable;

  SlobDictionary & dict;

  string resourceName;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  SlobResourceRequest( SlobDictionary & dict_,
                       string const & resourceName_ ):
    dict( dict_ ),
    resourceName( resourceName_ )
  {
    QThreadPool::globalInstance()->start(
      new SlobResourceRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by ZimResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~SlobResourceRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void SlobResourceRequestRunnable::run()
{
  r.run();
}

void SlobResourceRequest::run()
{
  // Some runnables linger enough that they are cancelled before they start
  if ( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
  {
    finish();
    return;
  }

  try
  {
    string resource;
    dict.loadResource( resourceName, resource );
    if( resource.empty() )
      throw exNoResource();

    if( Filetype::isNameOfCSS( resourceName ) )
    {
      QString css = QString::fromUtf8( resource.data(), resource.size() );
      dict.isolateCSS( css, ".slobdict" );
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
    gdWarning( "SLOB: Failed loading resource \"%s\" from \"%s\", reason: %s\n",
               resourceName.c_str(), dict.getName().c_str(), ex.what() );
    // Resource not loaded -- we don't set the hasAnyData flag then
  }

  finish();
}

sptr< Dictionary::DataRequest > SlobDictionary::getResource( string const & name )
  THROW_SPEC( std::exception )
{
  return new SlobResourceRequest( *this, name );
}


vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & initializing,
                                      unsigned maxHeadwordsToExpand )
  THROW_SPEC( std::exception )
{
  vector< sptr< Dictionary::Class > > dictionaries;

  for( vector< string >::const_iterator i = fileNames.begin(); i != fileNames.end();
       ++i )
  {
      // Skip files with the extensions different to .slob to speed up the
      // scanning

      QString firstName = QDir::fromNativeSeparators( FsEncoding::decode( i->c_str() ) );
      if( !firstName.endsWith( ".slob") )
        continue;

      // Got the file -- check if we need to rebuid the index

      vector< string > dictFiles( 1, *i );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      try
      {
        if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
             indexIsOldOrBad( indexFile ) )
        {
          SlobFile sf;

          gdDebug( "Slob: Building the index for dictionary: %s\n", i->c_str() );

          sf.open( firstName );

          initializing.indexingDictionary( sf.getDictionaryName().toUtf8().constData() );

          File::Class idx( indexFile, "wb" );
          IdxHeader idxHeader;
          memset( &idxHeader, 0, sizeof( idxHeader ) );

          // We write a dummy header first. At the end of the process the header
          // will be rewritten with the right values.

          idx.write( idxHeader );

          RefEntry refEntry;
          quint32 entries = sf.getRefsCount();

          IndexedWords indexedWords, indexedResources;

          set< quint64 > articlesPos;
          quint32 articleCount = 0, wordCount = 0;

          for( quint32 i = 0; i < entries; i++ )
          {
            sf.getRefEntry( i, refEntry );

            quint8 type = sf.getItem( refEntry, 0 );

            QString contentType = sf.getContentType( type );

            if( contentType.startsWith( "text/html", Qt::CaseInsensitive )
                || contentType.startsWith( "text/plain", Qt::CaseInsensitive ) )
            {
              //Article
              if( maxHeadwordsToExpand && entries > maxHeadwordsToExpand )
                indexedWords.addSingleWord( gd::toWString( refEntry.key ), i );
              else
                indexedWords.addWord( gd::toWString( refEntry.key ), i );

              wordCount += 1;

              quint64 pos = ( ( (quint64)refEntry.itemIndex ) << 32 ) + refEntry.binIndex;
              if( articlesPos.find( pos ) == articlesPos.end() )
              {
                articleCount += 1;
                articlesPos.insert( pos );
              }
            }
            else
            {
              indexedResources.addSingleWord( gd::toWString( refEntry.key ), i );
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

          QPair<quint32,quint32> langs =
              LangCoder::findIdsForFilename( QString::fromStdString( dictFiles[ 0 ] ) );

          idxHeader.langFrom = langs.first;
          idxHeader.langTo = langs.second;

          idx.rewind();

          idx.write( &idxHeader, sizeof( idxHeader ) );

        }
        dictionaries.push_back( new SlobDictionary( dictId,
                                                    indexFile,
                                                    dictFiles ) );
      }
      catch( std::exception & e )
      {
        gdWarning( "Slob dictionary initializing failed: %s, error: %s\n",
                   i->c_str(), e.what() );
        continue;
      }
      catch( ... )
      {
        qWarning( "Slob dictionary initializing failed\n" );
        continue;
      }
  }
  return dictionaries;
}

} // namespace Slob

#endif
