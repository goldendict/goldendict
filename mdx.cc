/* This file is (c) 2013 Timon Wong <timon86.wang AT gmail DOT com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mdx.hh"
#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include "file.hh"
#include "wstring.hh"
#include "wstring_qt.hh"
#include "chunkedstorage.hh"
#include "dprintf.hh"
#include "langcoder.hh"
#include "fsencoding.hh"
#include "audiolink.hh"
#include "ex.hh"
#include "mdictparser.hh"
#include "filetype.hh"

#include <algorithm>
#include <map>
#include <set>
#include <list>
#include <ctype.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QDir>
#include <QString>
#include <QSemaphore>
#include <QThreadPool>
#include <QAtomicInt>
#include <QTextDocument>
#include <QCryptographicHash>

namespace Mdx
{

using std::map;
using std::multimap;
using std::set;
using gd::wstring;
using gd::wchar;
using std::list;
using std::pair;
using std::string;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

using namespace Mdict;

enum
{
  kSignature = 0x4349444d,  // MDIC
  kCurrentFormatVersion = 10 + BtreeIndexing::FormatVersion + Folding::Version
};

DEF_EX( exCorruptDictionary, "dictionary file was tampered or corrupted", std::exception )

struct IdxHeader
{
  uint32_t signature; // First comes the signature, MDIC
  uint32_t formatVersion; // File format version, currently 1.
  uint32_t parserVersion; // Version of the parser used to parse the MDIC file.
  // Version of the folding algorithm used when building
  // index. If it's different from the current one,
  // the file is to be rebuilt.
  uint32_t foldingVersion;

  uint32_t articleCount; // Total number of articles, for informative purposes only
  uint32_t wordCount; // Total number of words, for informative purposes only

  uint32_t isRightToLeft; // Right to left
  uint32_t chunksOffset; // The offset to chunks' storage

  uint32_t descriptionAddress; // Address of the dictionary description in the chunks' storage
  uint32_t descriptionSize; // Size of the description in the chunks' storage, 0 = no description

  uint32_t styleSheetAddress;
  uint32_t styleSheetCount;

  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;

  uint32_t langFrom; // Source language
  uint32_t langTo; // Target language

  uint32_t mddIndexInfosOffset; // address of IndexInfos for resource files (.mdd)
  uint32_t mddIndexInfosCount; // count of IndexInfos for resource files
}
#ifndef _MSC_VER
__attribute__( ( packed ) )
#endif
;

// A helper method to read resources from .mdd file
class IndexedMdd: public BtreeIndexing::BtreeIndex
{
  Mutex & idxMutex;
  ChunkedStorage::Reader & chunks;
  QFile mddFile;
  bool isFileOpen;

public:

  IndexedMdd( Mutex & idxMutex, ChunkedStorage::Reader & chunks ):
    idxMutex( idxMutex ),
    chunks( chunks ),
    isFileOpen( false )
  {}

  /// Opens the index. The values are those previously returned by buildIndex().
  using BtreeIndexing::BtreeIndex::openIndex;

  /// Opens the mdd file itself. Returns true if succeeded, false otherwise.
  bool open( const char * fileName )
  {
    mddFile.setFileName( QString::fromUtf8( fileName ) );
    isFileOpen = mddFile.open( QFile::ReadOnly );
    return isFileOpen;
  }

  /// Returns true if the mdd is open, false otherwise.
  inline bool isOpen() const
  {
    return isFileOpen;
  }

  /// Checks whether the given file exists in the mdd file or not.
  /// Note that this function is thread-safe, since it does not access mdd file.
  bool hasFile( gd::wstring const & name )
  {
    if ( !isFileOpen )
      return false;
    vector< WordArticleLink > links = findArticles( name );
    return !links.empty();
  }

  /// Attempts loading the given file into the given vector. Returns true on
  /// success, false otherwise.
  bool loadFile( gd::wstring const & name, std::vector< char > & result )
  {
    if ( !isFileOpen )
      return false;

    vector< WordArticleLink > links = findArticles( name );
    if ( links.empty() )
      return false;

    MdictParser::RecordInfo indexEntry;
    vector< char > chunk;
    Mutex::Lock _( idxMutex );
    const char * indexEntryPtr = chunks.getBlock( links[ 0 ].articleOffset, chunk );
    memcpy( &indexEntry, indexEntryPtr, sizeof( indexEntry ) );

    ScopedMemMap compressed( mddFile, indexEntry.compressedBlockPos, indexEntry.compressedBlockSize );
    if ( !compressed.startAddress() )
    {
      return false;
    }

    QByteArray decompressed;
    if ( !MdictParser::parseCompressedBlock( indexEntry.compressedBlockSize, ( char * )compressed.startAddress(),
                                             indexEntry.decompressedBlockSize, decompressed ) )
    {
      return false;
    }

    result.resize( indexEntry.recordSize );
    memcpy( &result.front(), decompressed.constData() + indexEntry.recordOffset, indexEntry.recordSize );
    return true;
  }

};

class MdxDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;
  string dictionaryName;
  string encoding;
  ChunkedStorage::Reader chunks;
  QFile dictFile;
  vector< sptr< IndexedMdd > > mddResources;
  MdictParser::StyleSheets styleSheets;

  QAtomicInt deferredInitDone;
  Mutex deferredInitMutex;
  bool deferredInitRunnableStarted;
  QSemaphore deferredInitRunnableExited;

  string initError;

public:

  MdxDictionary( string const & id, string const & indexFile, vector<string> const & dictionaryFiles );

  ~MdxDictionary();

  virtual void deferredInit();

  virtual string getName() throw()
  {
    return dictionaryName;
  }

  virtual map< Dictionary::Property, string > getProperties() throw()
  {
    return map< Dictionary::Property, string >();
  }

  virtual unsigned long getArticleCount() throw()
  {
    return idxHeader.articleCount;
  }

  virtual unsigned long getWordCount() throw()
  {
    return idxHeader.wordCount;
  }

  inline virtual quint32 getLangFrom() const
  {
    return idxHeader.langFrom;
  }

  inline virtual quint32 getLangTo() const
  {
    return idxHeader.langTo;
  }

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const & word,
                                                      vector< wstring > const & alts,
                                                      wstring const & ) throw( std::exception );
  virtual sptr< Dictionary::DataRequest > getResource( string const & name ) throw( std::exception );
  virtual QString const & getDescription();

protected:

  virtual void loadIcon() throw();

private:

  virtual string const & ensureInitDone();
  void doDeferredInit();

  /// Loads an article with the given offset, filling the given strings.
  void loadArticle( uint32_t offset, string & articleText );

  /// Process resource links (images, audios, etc)
  QString & filterResource( QString const & articleId, QString & article );

  friend class MdxHeadwordsRequest;
  friend class MdxArticleRequest;
  friend class MddResourceRequest;
  friend class MdxDeferredInitRunnable;
};

MdxDictionary::MdxDictionary( string const & id, string const & indexFile,
                              vector<string> const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() ),
  chunks( idx, idxHeader.chunksOffset ),
  deferredInitRunnableStarted( false )
{
  // Read the dictionary's name
  idx.seek( sizeof( idxHeader ) );
  size_t len = idx.read< uint32_t >();
  vector< char > buf( len );
  idx.read( &buf.front(), len );
  dictionaryName = string( &buf.front(), len );

  // then read the dictionary's encoding
  len = idx.read< uint32_t >();
  buf.resize( len );
  idx.read( &buf.front(), len );
  encoding = string( &buf.front(), len );

  dictFile.setFileName( QString::fromUtf8( dictionaryFiles[ 0 ].c_str() ) );
  dictFile.open( QIODevice::ReadOnly );
}

MdxDictionary::~MdxDictionary()
{
  Mutex::Lock _( deferredInitMutex );

  // Wait for init runnable to complete if it was ever started
  if ( deferredInitRunnableStarted )
    deferredInitRunnableExited.acquire();

  dictFile.close();
}

//////// MdxDictionary::deferredInit()

class MdxDeferredInitRunnable: public QRunnable
{
  MdxDictionary & dictionary;
  QSemaphore & hasExited;

public:

  MdxDeferredInitRunnable( MdxDictionary & dictionary_,
                           QSemaphore & hasExited_ ):
    dictionary( dictionary_ ), hasExited( hasExited_ )
  {}

  ~MdxDeferredInitRunnable()
  {
    hasExited.release();
  }

  virtual void run()
  {
    dictionary.doDeferredInit();
  }
};

void MdxDictionary::deferredInit()
{
  if ( !deferredInitDone )
  {
    Mutex::Lock _( deferredInitMutex );

    if ( deferredInitDone )
      return;

    if ( !deferredInitRunnableStarted )
    {
      QThreadPool::globalInstance()->start(
        new MdxDeferredInitRunnable( *this, deferredInitRunnableExited ),
        -1000 );
      deferredInitRunnableStarted = true;
    }
  }
}

string const & MdxDictionary::ensureInitDone()
{
  doDeferredInit();
  return initError;
}

void MdxDictionary::doDeferredInit()
{
  if ( !deferredInitDone )
  {
    Mutex::Lock _( deferredInitMutex );

    if ( deferredInitDone )
      return;

    // Do deferred init

    try
    {
      // Retrieve stylesheets
      idx.seek( idxHeader.styleSheetAddress );
      for ( uint32_t i = 0; i < idxHeader.styleSheetCount; i++ )
      {
        qint32 key = idx.read< qint32 >();
        vector< char > buf;
        quint32 sz;

        sz = idx.read< quint32 >();
        buf.resize( sz );
        idx.read( &buf.front(), sz );
        QString styleBegin = QString::fromUtf8( buf.data() );

        sz = idx.read< quint32 >();
        buf.resize( sz );
        idx.read( &buf.front(), sz );
        QString styleEnd = QString::fromUtf8( buf.data() );

        styleSheets[ key ] = pair<QString, QString>( styleBegin, styleEnd );
      }

      // Initialize the index
      openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                            idxHeader.indexRootOffset ), idx, idxMutex );

      vector< string > mddFileNames;
      vector< IndexInfo > mddIndexInfos;
      idx.seek( idxHeader.mddIndexInfosOffset );
      for ( uint32_t i = 0; i < idxHeader.mddIndexInfosCount; i++ )
      {
        quint32 sz = idx.read< quint32 >();
        vector< char > buf( sz );
        idx.read( &buf.front(), sz );
        uint32_t btreeMaxElements = idx.read<uint32_t>();
        uint32_t rootOffset = idx.read<uint32_t>();
        mddFileNames.push_back( string( &buf.front() ) );
        mddIndexInfos.push_back( IndexInfo( btreeMaxElements, rootOffset ) );
      }

      vector< string > const dictFiles = getDictionaryFilenames();
      for ( uint32_t i = 1; i < dictFiles.size() && i < mddFileNames.size() + 1; i++ )
      {
        QFileInfo fi( QString::fromUtf8( dictFiles[ i ].c_str() ) );
        QString mddFileName = QString::fromUtf8( mddFileNames[ i - 1 ].c_str() );

        if ( fi.fileName() != mddFileName || !fi.exists() )
          continue;

        sptr< IndexedMdd > mdd = new IndexedMdd( idxMutex, chunks );
        mdd->openIndex( mddIndexInfos[ i - 1 ], idx, idxMutex );
        mdd->open( dictFiles[ i ].c_str() );
        mddResources.push_back( mdd );
      }
    }
    catch ( std::exception & e )
    {
      initError = e.what();
    }
    catch ( ... )
    {
      initError = "Unknown error";
    }

    deferredInitDone.ref();
  }
}

/// MdxDictionary::getArticle

class MdxArticleRequest;

class MdxArticleRequestRunnable: public QRunnable
{
  MdxArticleRequest & r;
  QSemaphore & hasExited;

public:

  MdxArticleRequestRunnable( MdxArticleRequest & r_,
                             QSemaphore & hasExited_ ):
    r( r_ ),
    hasExited( hasExited_ )
  {}

  ~MdxArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class MdxArticleRequest: public Dictionary::DataRequest
{
  friend class MdxArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  MdxDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  MdxArticleRequest( wstring const & word_,
                     vector< wstring > const & alts_,
                     MdxDictionary & dict_ ):
    word( word_ ),
    alts( alts_ ),
    dict( dict_ )
  {
    QThreadPool::globalInstance()->start( new MdxArticleRequestRunnable( *this, hasExited ) );
  }

  void run();

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~MdxArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void MdxArticleRequestRunnable::run()
{
  r.run();
}

void MdxArticleRequest::run()
{
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

  vector< WordArticleLink > chain = dict.findArticles( word );

  for ( unsigned x = 0; x < alts.size(); ++x )
  {
    /// Make an additional query for each alt
    vector< WordArticleLink > altChain = dict.findArticles( alts[ x ] );
    chain.insert( chain.end(), altChain.begin(), altChain.end() );
  }

  // Some synonims make it that the articles appear several times. We combat this
  // by only allowing them to appear once.
  set< uint32_t > articlesIncluded;
  // Sometimes the articles are physically duplicated. We store hashes of
  // the bodies to account for this.
  set< QByteArray > articleBodiesIncluded;
  string articleText;

  for ( unsigned x = 0; x < chain.size(); ++x )
  {
    if ( isCancelled )
    {
      finish();
      return;
    }

    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    // Grab that article
    string articleBody;
    bool hasError = false;
    QString errorMessage;

    try
    {
      dict.loadArticle( chain[ x ].articleOffset, articleBody );
    }
    catch ( exCorruptDictionary & )
    {
      errorMessage = tr( "Dictionary file was tampered or corrupted" );
      hasError = true;
    }
    catch ( std::exception & e )
    {
      errorMessage = e.what();
      hasError = true;
    }

    if ( hasError )
    {
      setErrorString( tr( "Failed loading article from %1, reason: %2" )
                      .arg( QString::fromUtf8( dict.getDictionaryFilenames()[ 0 ].c_str() ) )
                      .arg( errorMessage ) );
      finish();
      return;
    }

    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    QCryptographicHash hash( QCryptographicHash::Md5 );
    hash.addData( articleBody.data(), articleBody.size() );
    if ( !articleBodiesIncluded.insert( hash.result() ).second )
      continue; // Already had this body

    // Handle internal redirects
    if ( strncmp( articleBody.c_str(), "@@@LINK=", 8 ) == 0 )
    {
      wstring target = Utf8::decode( articleBody.c_str() + 8 );
      target = Folding::trimWhitespace( target );
      // Make an additional query for this redirection
      vector< WordArticleLink > altChain = dict.findArticles( target );
      chain.insert( chain.end(), altChain.begin(), altChain.end() );
      continue;
    }

    // See Issue #271: A mechanism to clean-up invalid HTML cards.
    string cleaner = "</font>""</font>""</font>""</font>""</font>""</font>"
                     "</font>""</font>""</font>""</font>""</font>""</font>"
                     "</b></b></b></b></b></b></b></b>"
                     "</i></i></i></i></i></i></i></i>"
                     "</a></a></a></a></a></a></a></a>";
    articleText += "<div class=\"mdict\">" + articleBody + cleaner + "</div>\n";
    hasAnyData = true;
  }

  if ( hasAnyData )
  {
    Mutex::Lock _( dataMutex );
    data.insert( data.end(), articleText.begin(), articleText.end() );
  }

  finish();
}

sptr<Dictionary::DataRequest> MdxDictionary::getArticle( const wstring & word, const vector<wstring> & alts,
                                                         const wstring & ) throw( std::exception )
{
  return new MdxArticleRequest( word, alts, *this );
}

/// MdxDictionary::getResource

class MddResourceRequest;

class MddResourceRequestRunnable: public QRunnable
{
  MddResourceRequest & r;
  QSemaphore & hasExited;

public:

  MddResourceRequestRunnable( MddResourceRequest & r_,
                              QSemaphore & hasExited_ ): r( r_ ),
    hasExited( hasExited_ )
  {}

  ~MddResourceRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class MddResourceRequest: public Dictionary::DataRequest
{
  friend class MddResourceRequestRunnable;

  MdxDictionary & dict;
  wstring resourceName;
  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  MddResourceRequest( MdxDictionary & dict_,
                      string const & resourceName_ ):
    dict( dict_ ),
    resourceName( Utf8::decode( resourceName_ ) )
  {
    QThreadPool::globalInstance()->start( new MddResourceRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by MddResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~MddResourceRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void MddResourceRequestRunnable::run()
{
  r.run();
}

void MddResourceRequest::run()
{
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

  // In order to prevent recursive internal redirection...
  set< QByteArray > resourceIncluded;

  for ( ;; )
  {
    // Some runnables linger enough that they are cancelled before they start
    if ( isCancelled )
    {
      finish();
      return;
    }

    string u8ResourceName = Utf8::encode( resourceName );
    QCryptographicHash hash( QCryptographicHash::Md5 );
    hash.addData( u8ResourceName.data(), u8ResourceName.size() );
    if ( !resourceIncluded.insert( hash.result() ).second )
      continue;

    // Convert to the Windows separator
    std::replace( resourceName.begin(), resourceName.end(), '/', '\\' );
    if ( resourceName[ 0 ] != '\\' )
    {
      resourceName.insert( 0, 1, '\\' );
    }

    Mutex::Lock _( dataMutex );
    data.clear();

    try
    {
      // local file takes precedence
      string fn = FsEncoding::dirname( dict.getDictionaryFilenames()[ 0 ] ) +
                  FsEncoding::separator() + u8ResourceName;
      File::loadFromFile( fn, data );
    }
    catch ( File::exCantOpen & )
    {
      for ( vector< sptr< IndexedMdd > >::const_iterator i = dict.mddResources.begin();
            i != dict.mddResources.end(); i++  )
      {
        sptr< IndexedMdd > mddResource = *i;
        if ( mddResource->loadFile( resourceName, data ) )
          break;
      }
    }

    // Check if this file has a redirection
    // Always encoded in UTF16-LE
    // L"@@@LINK="
    static const char pattern[16] =
    {
      '@', '\0', '@', '\0', '@', '\0', 'L', '\0', 'I', '\0', 'N', '\0', 'K', '\0', '=', '\0'
    };

    if ( data.size() > sizeof( pattern ) )
    {
      if ( memcmp( &data.front(),  pattern, sizeof( pattern ) ) == 0 )
      {
        data.push_back( '\0' );
        data.push_back( '\0' );
        QString target = MdictParser::toUtf16( "UTF-16LE", &data.front() + sizeof( pattern ),
                                               data.size() - sizeof( pattern ) );
        resourceName = gd::toWString( target.trimmed() );
        continue;
      }
    }

    if ( data.size() > 0 )
    {
      hasAnyData = true;

      if ( Filetype::isNameOfCSS( u8ResourceName ) )
      {
        QString css = QString::fromUtf8( data.data(), data.size() );
        dict.isolateCSS( css, ".mdict" );
        QByteArray bytes = css.toUtf8();
        data.resize( bytes.size() );
        memcpy( &data.front(), bytes.constData(), bytes.size() );
      }
    }

    break;
  }

  finish();
}

sptr<Dictionary::DataRequest> MdxDictionary::getResource( const string & name ) throw( std::exception )
{
  return new MddResourceRequest( *this, name );
}

const QString & MdxDictionary::getDescription()
{
  if ( !dictionaryDescription.isEmpty() )
    return dictionaryDescription;

  if ( idxHeader.descriptionSize == 0 )
  {
    dictionaryDescription = "NONE";
  }
  else
  {
    vector< char > chunk;
    char * dictDescription = chunks.getBlock( idxHeader.descriptionAddress, chunk );
    string str( dictDescription );
    dictionaryDescription = QString::fromUtf8( str.c_str(), str.size() );
  }

  return dictionaryDescription;
}

void MdxDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension
  fileName.chop( 3 );

  if ( !loadIconFromFile( fileName ) )
  {
    // Use default icons
    dictionaryIcon = dictionaryNativeIcon = QIcon( ":/icons/mdict.png" );
  }

  dictionaryIconLoaded = true;
}

void MdxDictionary::loadArticle( uint32_t offset, string & articleText )
{
  vector< char > chunk;
  Mutex::Lock _( idxMutex );

  // Load record info from index
  MdictParser::RecordInfo recordInfo;
  char * pRecordInfo = chunks.getBlock( offset, chunk );
  memcpy( &recordInfo, pRecordInfo, sizeof( recordInfo ) );

  // Make a sub unique id for this article
  QString articleId;
  articleId.setNum( ( quint64 )pRecordInfo, 16 );

  ScopedMemMap compressed( dictFile, recordInfo.compressedBlockPos, recordInfo.compressedBlockSize );
  if ( !compressed.startAddress() )
    throw exCorruptDictionary();

  QByteArray decompressed;
  if ( !MdictParser::parseCompressedBlock( recordInfo.compressedBlockSize, ( char * )compressed.startAddress(),
                                           recordInfo.decompressedBlockSize, decompressed ) )
    throw exCorruptDictionary();

  QString article = MdictParser::toUtf16( encoding.c_str(),
                                          decompressed.constData() + recordInfo.recordOffset,
                                          recordInfo.recordSize );

  article = MdictParser::substituteStylesheet( article, styleSheets );
  article = filterResource( articleId, article );
  articleText = string( article.toUtf8().constData() );
}

QString & MdxDictionary::filterResource( QString const & articleId, QString & article )
{
  QString id = QString::fromStdString( getId() );
  QString uniquePrefix = id + "_" + articleId + "_";
  QRegExp anchorLinkRe( "(<\\s*a\\s+[^>]*\\b(?:name|id)\\b\\s*=\\s*[\"']*)(?=[^\"'])", Qt::CaseInsensitive );
  anchorLinkRe.setMinimal( true );

  return article
         // anchors
         .replace( anchorLinkRe,
                   "\\1" + uniquePrefix )
         .replace( QRegExp( "(href\\s*=\\s*[\"'])entry://#", Qt::CaseInsensitive ),
                   "\\1#" + uniquePrefix )
         // word cross links
         .replace( QRegExp( "(href\\s*=\\s*[\"'])entry://([^#\"'/]+)#?[^\"']*", Qt::CaseInsensitive ),
                   "\\1gdlookup://localhost/\\2" )
         // sounds, and audio link script
         .replace( QRegExp( "(<\\s*(?:a|area)\\s+[^>]*\\bhref\\b\\s*=\\s*\")sound://([^\"']*)", Qt::CaseInsensitive ),
                   QString::fromStdString( addAudioLink( "\"gdau://" + getId() + "/\\2\"", getId() ) ) +
                   "\\1gdau://" + id + "/\\2" )
         // stylesheets
         .replace( QRegExp( "(<\\s*link\\s+[^>]*\\bhref\\b\\s*=\\s*[\"']+)(?:file://)?[\\x00-\\x30\\x7f]*([^\"']*)",
                            Qt::CaseInsensitive, QRegExp::RegExp2 ),
                   "\\1bres://" + id + "/\\2" )
         .replace( QRegExp( "(<\\s*link\\s+[^>]*\\bhref\\b\\s*=\\s*)(?!['\"]+)(?!bres:|data:)(?:file://)?([^\\s>]+)",
                            Qt::CaseInsensitive, QRegExp::RegExp2 ),
                   "\\1\"bres://" + id + "/\\\"" )
         // images
         .replace( QRegExp( "(<\\s*img\\s+[^>]*\\bsrc\\b\\s*=\\s*[\"']+)(?:file://)?[\\x00-\\x1f\\x7f]*([^\"']*)",
                            Qt::CaseInsensitive, QRegExp::RegExp2 ),
                   "\\1bres://" + id + "/\\2" )
         .replace( QRegExp( "(<\\s*img\\s+[^>]*\\bsrc\\b\\s*=\\s*)(?!['\"]+)(?!bres:|data:)(?:file://)?([^\\s>]+)",
                            Qt::CaseInsensitive, QRegExp::RegExp2 ),
                   "\\1\"bres://" + id + "/\\2\"" );
}

static void addEntryToIndex( QString const & word, uint32_t offset, IndexedWords & indexedWords )
{
  // Strip any leading or trailing whitespaces
  QString wordTrimmed = word.trimmed();
  indexedWords.addWord( gd::toWString( wordTrimmed ), offset );
}

static void addEntryToIndexSingle( QString const & word, uint32_t offset, IndexedWords & indexedWords )
{
  // Strip any leading or trailing whitespaces
  QString wordTrimmed = word.trimmed();
  indexedWords.addSingleWord( gd::toWString( wordTrimmed ), offset );
}

class ArticleHandler: public MdictParser::RecordHandler
{
public:
  ArticleHandler( ChunkedStorage::Writer & chunks, IndexedWords & indexedWords ) :
    chunks( chunks ),
    indexedWords( indexedWords )
  {
  }

  virtual void handleRecord( QString const & headWord, MdictParser::RecordInfo const & recordInfo )
  {
    // Save the article's record info
    uint32_t articleAddress = chunks.startNewBlock();
    chunks.addToBlock( &recordInfo, sizeof( recordInfo ) );
    // Add entries to the index
    addEntryToIndex( headWord, articleAddress, indexedWords );
  }

private:
  ChunkedStorage::Writer & chunks;
  IndexedWords & indexedWords;
};

class ResourceHandler: public MdictParser::RecordHandler
{
public:
  ResourceHandler( ChunkedStorage::Writer & chunks, IndexedWords & indexedWords ):
    chunks( chunks ),
    indexedWords( indexedWords )
  {
  }

  virtual void handleRecord( QString const & fileName, MdictParser::RecordInfo const & recordInfo )
  {
    uint32_t resourceInfoAddress = chunks.startNewBlock();
    chunks.addToBlock( &recordInfo, sizeof( recordInfo ) );
    // Add entries to the index
    addEntryToIndexSingle( fileName, resourceInfoAddress, indexedWords );
  }

private:
  ChunkedStorage::Writer & chunks;
  IndexedWords & indexedWords;
};


static bool indexIsOldOrBad( vector< string > const & dictFiles, string const & indexFile )
{
  File::Class idx( indexFile, "rb" );
  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != kSignature ||
         header.formatVersion != kCurrentFormatVersion ||
         header.parserVersion != MdictParser::kParserVersion ||
         header.foldingVersion != Folding::Version ||
         header.mddIndexInfosCount != dictFiles.size() - 1;
}

static void findResourceFiles( string const & mdx, vector< string > & dictFiles )
{
  string base( mdx, 0, mdx.size() - 4 );
  // Check if there' is any file end with .mdd, which is the resource file for the dictionary
  string resFile;
  if ( File::tryPossibleName( base + ".mdd", resFile ) )
  {
    dictFiles.push_back( resFile );
    // Find complementary .mdd file (volumes), like follows:
    //   demo.mdx   <- main dictionary file
    //   demo.mdd   <- main resource file ( 1st volume )
    //   demo.1.mdd <- 2nd volume
    //   ...
    //   demo.n.mdd <- nth volume
    QString baseU8 = QString::fromUtf8( base.c_str() );
    int vol = 1;
    while ( File::tryPossibleName( string( QString( "%1.%2.mdd" ).arg( baseU8 ).arg( vol )
                                           .toUtf8().constBegin() ), resFile ) )
    {
      dictFiles.push_back( resFile );
      vol++;
    }
  }
}

vector< sptr< Dictionary::Class > > makeDictionaries( vector< string > const & fileNames,
                                                      string const & indicesDir,
                                                      Dictionary::Initializing & initializing ) throw( std::exception )
{
  vector< sptr< Dictionary::Class > > dictionaries;

  for ( vector< string >::const_iterator i = fileNames.begin(); i != fileNames.end(); i++ )
  {
    // Skip files with the extensions different to .mdx to speed up the
    // scanning
    if ( i->size() < 4 || strcasecmp( i->c_str() + ( i->size() - 4 ), ".mdx" ) != 0 )
      continue;

    vector< string > dictFiles( 1, *i );
    findResourceFiles( *i, dictFiles );

    string dictId = Dictionary::makeDictionaryId( dictFiles );
    string indexFile = indicesDir + dictId;

    if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
         indexIsOldOrBad( dictFiles, indexFile ) )
    {
      // Building the index

      qDebug( "MDict: Building the index for dictionary: %s\n", i->c_str() );

      MdictParser parser;
      list< sptr< MdictParser > > mddParsers;

      if ( !parser.open( i->c_str() ) )
        continue;

      string title = string( parser.title().toUtf8().constData() );
      initializing.indexingDictionary( title );

      for ( vector< string >::const_iterator mddIter = dictFiles.begin() + 1;
            mddIter != dictFiles.end(); mddIter++ )
      {
        if ( File::exists( *mddIter ) )
        {
          sptr< MdictParser > mddParser = new MdictParser();
          if ( !mddParser->open( mddIter->c_str() ) )
          {
            qWarning( "Warning: Broken mdd (resource) file: %s\n", mddIter->c_str() );
            continue;
          }
          mddParsers.push_back( mddParser );
        }
      }

      File::Class idx( indexFile, "wb" );
      IdxHeader idxHeader;
      memset( &idxHeader, 0, sizeof( idxHeader ) );
      // We write a dummy header first. At the end of the process the header
      // will be rewritten with the right values.
      idx.write( idxHeader );

      // Write the title first
      idx.write< uint32_t >( title.size() );
      idx.write( title.data(), title.size() );

      // then the encoding
      {
        string encoding = string( parser.encoding().toUtf8().constData() );
        idx.write< uint32_t >( encoding.size() );
        idx.write( encoding.data(), encoding.size() );
      }

      // This is our index data that we accumulate during the loading process.
      // For each new word encountered, we emit the article's body to the file
      // immediately, inserting the word itself and its offset in this map.
      // This map maps folded words to the original words and the corresponding
      // articles' offsets.
      IndexedWords indexedWords;
      ChunkedStorage::Writer chunks( idx );

      idxHeader.isRightToLeft = parser.isRightToLeft();

      // Save dictionary description if there's one
      {
        string description = string( parser.description().toUtf8().constData() );
        idxHeader.descriptionAddress = chunks.startNewBlock();
        chunks.addToBlock( description.c_str(), description.size() + 1 );
        idxHeader.descriptionSize = description.size() + 1;
      }

      ArticleHandler articleHandler( chunks, indexedWords );
      MdictParser::HeadWordIndex headWordIndex;

      // enumerating word and its definition
      while ( parser.readNextHeadWordIndex( headWordIndex ) )
      {
        parser.readRecordBlock( headWordIndex, articleHandler );
      }

      // enumerating resources if there's any
      vector< sptr< IndexedWords > > mddIndices;
      vector< string > mddFileNames;
      while ( !mddParsers.empty() )
      {
        sptr< MdictParser > mddParser = mddParsers.front();
        sptr< IndexedWords > mddIndexedWords = new IndexedWords();
        ResourceHandler resourceHandler( chunks, *mddIndexedWords );

        while ( mddParser->readNextHeadWordIndex( headWordIndex ) )
        {
          mddParser->readRecordBlock( headWordIndex, resourceHandler );
        }

        mddIndices.push_back( mddIndexedWords );
        // Save filename for .mdd files only
        QFileInfo fi( mddParser->filename() );
        mddFileNames.push_back( string( fi.fileName().toUtf8().constData() ) );
        mddParsers.pop_front();
      }

      // Finish with the chunks
      idxHeader.chunksOffset = chunks.finish();

      DPRINTF( "Writing index...\n" );

      // Good. Now build the index
      IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );
      idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
      idxHeader.indexRootOffset = idxInfo.rootOffset;

      // Save dictionary stylesheets
      {
        MdictParser::StyleSheets const & styleSheets = parser.styleSheets();
        idxHeader.styleSheetAddress = idx.tell();
        idxHeader.styleSheetCount = styleSheets.size();

        for ( MdictParser::StyleSheets::const_iterator iter = styleSheets.begin();
              iter != styleSheets.end(); iter++ )
        {
          string styleBegin( iter->second.first.toUtf8().constData() );
          string styleEnd( iter->second.second.toUtf8().constData() );

          // key
          idx.write<qint32>( iter->first );
          // styleBegin
          idx.write<quint32>( ( quint32 )styleBegin.size() + 1 );
          idx.write( styleBegin.c_str(), styleBegin.size() + 1 );
          // styleEnd
          idx.write<quint32>( ( quint32 )styleEnd.size() + 1 );
          idx.write( styleEnd.c_str(), styleEnd.size() + 1 );
        }
      }

      // read languages
      QPair<quint32, quint32> langs = LangCoder::findIdsForFilename( QString::fromStdString( *i ) );

      // if no languages found, try dictionary's name
      if ( langs.first == 0 || langs.second == 0 )
      {
        langs = LangCoder::findIdsForFilename( parser.title() );
      }

      idxHeader.langFrom = langs.first;
      idxHeader.langTo = langs.second;

      // Build index info for each mdd file
      vector< IndexInfo > mddIndexInfos;
      for ( vector< sptr< IndexedWords > >::const_iterator mddIndexIter = mddIndices.begin();
            mddIndexIter != mddIndices.end(); mddIndexIter++ )
      {
        IndexInfo resourceIdxInfo = BtreeIndexing::buildIndex( *( *mddIndexIter ), idx );
        mddIndexInfos.push_back( resourceIdxInfo );
      }

      // Save address of IndexInfos for resource files
      idxHeader.mddIndexInfosOffset = idx.tell();
      idxHeader.mddIndexInfosCount = mddIndexInfos.size();
      for ( uint32_t mi = 0; mi < mddIndexInfos.size(); mi++ )
      {
        const string & mddfile = mddFileNames[ mi ];

        idx.write<quint32>( ( quint32 )mddfile.size() + 1 );
        idx.write( mddfile.c_str(), mddfile.size() + 1 );
        idx.write<uint32_t>( mddIndexInfos[ mi ].btreeMaxElements );
        idx.write<uint32_t>( mddIndexInfos[ mi ].rootOffset );
      }

      // That concludes it. Update the header.
      idxHeader.signature = kSignature;
      idxHeader.formatVersion = kCurrentFormatVersion;
      idxHeader.parserVersion = MdictParser::kParserVersion;
      idxHeader.foldingVersion = Folding::Version;
      idxHeader.articleCount = parser.wordCount();
      idxHeader.wordCount = parser.wordCount();

      idx.rewind();
      idx.write( &idxHeader, sizeof( idxHeader ) );
    }

    dictionaries.push_back( new MdxDictionary( dictId, indexFile, dictFiles ) );
  }

  return dictionaries;
}

}
