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
#include "gddebug.hh"
#include "langcoder.hh"
#include "fsencoding.hh"
#include "audiolink.hh"
#include "ex.hh"
#include "mdictparser.hh"
#include "filetype.hh"
#include "ftshelpers.hh"
#include "htmlescape.hh"

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

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
#include <QRegularExpression>
#endif

#include "qt4x5.hh"

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
  kCurrentFormatVersion = 11 + BtreeIndexing::FormatVersion + Folding::Version
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
  Mutex fileMutex;
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
#ifdef USE_QTWEBKIT
  QString cacheDirName;
#endif

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
                                                      wstring const &,
                                                      bool ignoreDiacritics ) THROW_SPEC( std::exception );
  virtual sptr< Dictionary::DataRequest > getResource( string const & name ) THROW_SPEC( std::exception );
  virtual QString const & getDescription();

  virtual sptr< Dictionary::DataRequest > getSearchResults( QString const & searchString,
                                                            int searchMode, bool matchCase,
                                                            int distanceBetweenWords,
                                                            int maxResults,
                                                            bool ignoreWordsOrder,
                                                            bool ignoreDiacritics,
                                                            QThreadPool * ftsThreadPoolPtr );
  virtual void getArticleText( uint32_t articleAddress, QString & headword, QString & text );

  virtual void makeFTSIndex(QAtomicInt & isCancelled, bool firstIteration );

  virtual void setFTSParameters( Config::FullTextSearch const & fts )
  {
    if( ensureInitDone().size() )
      return;

    can_FTS = fts.enabled
              && !fts.disabledTypes.contains( "MDICT", Qt::CaseInsensitive )
              && ( fts.maxDictionarySize == 0 || getArticleCount() <= fts.maxDictionarySize );
  }

#ifdef USE_QTWEBKIT
  QString getCachedFileName( QString name );
#endif

protected:

  virtual void loadIcon() throw();

private:

  virtual string const & ensureInitDone();
  void doDeferredInit();

  /// Loads an article with the given offset, filling the given strings.
  void loadArticle( uint32_t offset, string & articleText, bool noFilter = false );

  /// Process resource links (images, audios, etc)
  QString & filterResource( QString const & articleId, QString & article );

#ifdef USE_QTWEBKIT
  void removeDirectory( QString const & directory );
#endif

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
  if( len > 0 )
  {
    idx.read( &buf.front(), len );
    dictionaryName = string( &buf.front(), len );
  }

  // then read the dictionary's encoding
  len = idx.read< uint32_t >();
  if( len > 0 )
  {
    buf.resize( len );
    idx.read( &buf.front(), len );
    encoding = string( &buf.front(), len );
  }

  dictFile.setFileName( QString::fromUtf8( dictionaryFiles[ 0 ].c_str() ) );
  dictFile.open( QIODevice::ReadOnly );

  // Full-text search parameters

  can_FTS = true;

  ftsIdxName = indexFile + "_FTS";

  if( !Dictionary::needToRebuildIndex( dictionaryFiles, ftsIdxName )
      && !FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName, this ) )
    FTS_index_completed.ref();

#ifdef USE_QTWEBKIT
  cacheDirName = QDir::tempPath() + QDir::separator()
                 + QString::fromUtf8( getId().c_str() )
                 + ".cache";
#endif
}

MdxDictionary::~MdxDictionary()
{
  Mutex::Lock _( deferredInitMutex );

  // Wait for init runnable to complete if it was ever started
  if ( deferredInitRunnableStarted )
    deferredInitRunnableExited.acquire();

  dictFile.close();

#ifdef USE_QTWEBKIT
  removeDirectory( cacheDirName );
#endif
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
  if ( !Qt4x5::AtomicInt::loadAcquire( deferredInitDone ) )
  {
    Mutex::Lock _( deferredInitMutex );

    if ( Qt4x5::AtomicInt::loadAcquire( deferredInitDone ) )
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
  if ( !Qt4x5::AtomicInt::loadAcquire( deferredInitDone ) )
  {
    Mutex::Lock _( deferredInitMutex );

    if ( Qt4x5::AtomicInt::loadAcquire( deferredInitDone ) )
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

void MdxDictionary::makeFTSIndex( QAtomicInt & isCancelled, bool firstIteration )
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

  gdDebug( "MDict: Building the full-text index for dictionary: %s\n",
           getName().c_str() );

  try
  {
    FtsHelpers::makeFTSIndex( this, isCancelled );
    FTS_index_completed.ref();
  }
  catch( std::exception &ex )
  {
    gdWarning( "MDict: Failed building full-text search index for \"%s\", reason: %s\n", getName().c_str(), ex.what() );
    QFile::remove( FsEncoding::decode( ftsIdxName.c_str() ) );
  }
}

void MdxDictionary::getArticleText( uint32_t articleAddress, QString & headword, QString & text )
{
  try
  {
    headword.clear();
    string articleText;

    loadArticle( articleAddress, articleText, true );
    text = Html::unescape( QString::fromUtf8( articleText.data(), articleText.size() ) );
  }
  catch( std::exception &ex )
  {
    gdWarning( "MDict: Failed retrieving article from \"%s\", reason: %s\n", getName().c_str(), ex.what() );
  }
}

sptr< Dictionary::DataRequest > MdxDictionary::getSearchResults( QString const & searchString,
                                                                 int searchMode, bool matchCase,
                                                                 int distanceBetweenWords,
                                                                 int maxResults,
                                                                 bool ignoreWordsOrder,
                                                                 bool ignoreDiacritics,
                                                                 QThreadPool * ftsThreadPoolPtr )
{
  return new FtsHelpers::FTSResultsRequest( *this, searchString,searchMode, matchCase, distanceBetweenWords, maxResults, ignoreWordsOrder, ignoreDiacritics, ftsThreadPoolPtr );
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
  bool ignoreDiacritics;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  MdxArticleRequest( wstring const & word_,
                     vector< wstring > const & alts_,
                     MdxDictionary & dict_,
                     bool ignoreDiacritics_ ):
    word( word_ ),
    alts( alts_ ),
    dict( dict_ ),
    ignoreDiacritics( ignoreDiacritics_ )
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
  if ( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
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

  vector< WordArticleLink > chain = dict.findArticles( word, ignoreDiacritics );

  for ( unsigned x = 0; x < alts.size(); ++x )
  {
    /// Make an additional query for each alt
    vector< WordArticleLink > altChain = dict.findArticles( alts[ x ], ignoreDiacritics );
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
    if ( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
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
  }

  if ( !articleText.empty() )
  {
    Mutex::Lock _( dataMutex );
    data.insert( data.end(), articleText.begin(), articleText.end() );
    hasAnyData = true;
  }

  finish();
}

sptr<Dictionary::DataRequest> MdxDictionary::getArticle( const wstring & word, const vector<wstring> & alts,
                                                         const wstring &, bool ignoreDiacritics ) THROW_SPEC( std::exception )
{
  return new MdxArticleRequest( word, alts, *this, ignoreDiacritics );
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
  if ( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
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
    if ( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
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
            i != dict.mddResources.end(); ++i  )
      {
        if ( ( *i )->loadFile( resourceName, data ) )
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

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
        QRegularExpression links( "url\\(\\s*(['\"]?)([^'\"]*)(['\"]?)\\s*\\)",
                                  QRegularExpression::CaseInsensitiveOption );
#else
        QRegExp links( "url\\(\\s*(['\"]?)([^'\"]*)(['\"]?)\\s*\\)", Qt::CaseInsensitive, QRegExp::RegExp );
#endif
        QString id = QString::fromUtf8( dict.getId().c_str() );
        int pos = 0;

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
        QString newCSS;
        QRegularExpressionMatchIterator it = links.globalMatch( css );
        while ( it.hasNext() )
        {
          QRegularExpressionMatch match = it.next();
          newCSS += css.midRef( pos, match.capturedStart() - pos );
          pos = match.capturedEnd();
          QString url = match.captured( 2 );
#else
        for( ; ; )
        {
          pos = links.indexIn( css, pos );
          if( pos < 0 )
            break;
          QString url = links.cap( 2 );
#endif

          if( url.indexOf( ":/" ) >= 0 || url.indexOf( "data:" ) >= 0)
          {
            // External link or base64-encoded data
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
            newCSS += match.captured();
#else
            pos += links.cap().size();
#endif
            continue;
          }

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
          QString newUrl = QString( "url(" ) + match.captured( 1 ) + "bres://"
                                             + id + "/" + url + match.captured( 3 ) + ")";
          newCSS += newUrl;
#else
          QString newUrl = QString( "url(" ) + links.cap( 1 ) + "bres://"
                                             + id + "/" + url + links.cap( 3 ) + ")";
          css.replace( pos, links.cap().size(), newUrl );
          pos += newUrl.size();
#endif
        }
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
        if( pos )
        {
          newCSS += css.midRef( pos );
          css = newCSS;
          newCSS.clear();
        }
#endif
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

sptr<Dictionary::DataRequest> MdxDictionary::getResource( const string & name ) THROW_SPEC( std::exception )
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
    Mutex::Lock _( idxMutex );
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

void MdxDictionary::loadArticle( uint32_t offset, string & articleText, bool noFilter )
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

  if( !noFilter )
    article = filterResource( articleId, article );

  // Check for unclosed <span> and <div>

  int openTags = article.count( QRegExp( "<\\s*span\\b", Qt::CaseInsensitive ) );
  int closedTags = article.count( QRegExp( "<\\s*/span\\s*>", Qt::CaseInsensitive ) );
  while( openTags > closedTags )
  {
    article += "</span>";
    closedTags += 1;
  }

  openTags = article.count( QRegExp( "<\\s*div\\b", Qt::CaseInsensitive ) );
  closedTags = article.count( QRegExp( "<\\s*/div\\s*>", Qt::CaseInsensitive ) );
  while( openTags > closedTags )
  {
    article += "</div>";
    closedTags += 1;
  }

  articleText = string( article.toUtf8().constData() );
}

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
QString & MdxDictionary::filterResource( QString const & articleId, QString & article )
{
  QString id = QString::fromStdString( getId() );
  QString uniquePrefix = QString::fromLatin1( "g" ) + id + "_" + articleId + "_";

#ifdef USE_QTWEBKIT
  QRegularExpression allLinksRe( "(?:<\\s*(a(?:rea)?|img|link|script|source)(?:\\s+[^>]+|\\s*)>)",
#else
  QRegularExpression allLinksRe( "(?:<\\s*(a(?:rea)?|img|link|script|source|audio|video)(?:\\s+[^>]+|\\s*)>)",
#endif
                                 QRegularExpression::CaseInsensitiveOption );
  QRegularExpression wordCrossLink( "([\\s\"']href\\s*=)\\s*([\"'])entry://([^>#]*?)((?:#[^>]*?)?)\\2",
                                    QRegularExpression::CaseInsensitiveOption );
  QRegularExpression anchorIdRe( "([\\s\"'](?:name|id)\\s*=)\\s*([\"'])\\s*(?=\\S)",
                                 QRegularExpression::CaseInsensitiveOption );
  QRegularExpression anchorIdRe2( "([\\s\"'](?:name|id)\\s*=)\\s*(?=[^\"'])([^\\s\">]+)",
                                  QRegularExpression::CaseInsensitiveOption );
  QRegularExpression anchorLinkRe( "([\\s\"']href\\s*=\\s*[\"'])entry://#",
                                   QRegularExpression::CaseInsensitiveOption );
  QRegularExpression audioRe( "([\\s\"']href\\s*=)\\s*([\"'])sound://([^\">]+)\\2",
                              QRegularExpression::CaseInsensitiveOption
                              | QRegularExpression::InvertedGreedinessOption );
  QRegularExpression stylesRe( "([\\s\"']href\\s*=)\\s*([\"'])(?!\\s*\\b(?:(?:bres|https?|ftp)://|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\">]+)\\2",
                               QRegularExpression::CaseInsensitiveOption );
  QRegularExpression stylesRe2( "([\\s\"']href\\s*=)\\s*(?![\\s\"']|\\b(?:(?:bres|https?|ftp)://|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\\s\">]+)",
                                QRegularExpression::CaseInsensitiveOption );
  QRegularExpression inlineScriptRe( "<\\s*script(?:(?=\\s)(?:(?![\\s\"']src\\s*=)[^>])+|\\s*)>",
                                     QRegularExpression::CaseInsensitiveOption );
  QRegularExpression closeScriptTagRe( "<\\s*/script\\s*>",
                                       QRegularExpression::CaseInsensitiveOption );
  QRegularExpression srcRe( "([\\s\"']src\\s*=)\\s*([\"'])(?!\\s*\\b(?:(?:bres|https?|ftp)://|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\">]+)\\2",
                            QRegularExpression::CaseInsensitiveOption );
  QRegularExpression srcRe2( "([\\s\"']src\\s*=)\\s*(?![\\s\"']|\\b(?:(?:bres|https?|ftp)://|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\\s\">]+)",
                             QRegularExpression::CaseInsensitiveOption );

  QString articleNewText;
  int linkPos = 0;
  QRegularExpressionMatchIterator it = allLinksRe.globalMatch( article );
  while( it.hasNext() )
  {
    QRegularExpressionMatch allLinksMatch = it.next();

    if( allLinksMatch.capturedEnd() < linkPos )
      continue;

    articleNewText += article.midRef( linkPos, allLinksMatch.capturedStart() - linkPos );
    linkPos = allLinksMatch.capturedEnd();

    QString linkTxt = allLinksMatch.captured();
    QString linkType = allLinksMatch.captured( 1 ).toLower();
    QString newLink;

    if( !linkType.isEmpty() && linkType.at( 0 ) == 'a' )
    {
      QRegularExpressionMatch match = anchorIdRe.match( linkTxt );
      if( match.hasMatch() )
      {
        QString newText = match.captured( 1 ) + match.captured( 2 ) + uniquePrefix;
        newLink = linkTxt.replace( match.capturedStart(), match.capturedLength(), newText );
      }
      else
        newLink = linkTxt.replace( anchorIdRe2, "\\1\"" + uniquePrefix + "\\2\"" );

      newLink = newLink.replace( anchorLinkRe, "\\1#" + uniquePrefix );

      match = audioRe.match( newLink );
      if( match.hasMatch() )
      {
        // sounds and audio link script
        QString newTxt = match.captured( 1 ) + match.captured( 2 )
                         + "gdau://" + id + "/"
                         + match.captured( 3 ) + match.captured( 2 );
        newLink = QString::fromUtf8( addAudioLink( "\"gdau://" + getId() + "/" + match.captured( 3 ).toUtf8().data() + "\"" ).c_str() )
                  + newLink.replace( match.capturedStart(), match.capturedLength(), newTxt );
      }

      match = wordCrossLink.match( newLink );
      if( match.hasMatch() )
      {
        QString newTxt = match.captured( 1 ) + match.captured( 2 )
                         + "gdlookup://localhost/"
                         + match.captured( 3 );

        if( match.lastCapturedIndex() >= 4 && !match.captured( 4 ).isEmpty() )
          newTxt += QString( "?gdanchor=" ) + uniquePrefix + match.captured( 4 ).mid( 1 );

        newTxt += match.captured( 2 );
        newLink.replace( match.capturedStart(), match.capturedLength(), newTxt );
      }
    }
    else
    if( linkType.compare( "link" ) == 0 )
    {
      // stylesheets
      QRegularExpressionMatch match = stylesRe.match( linkTxt );
      if( match.hasMatch() )
      {
        QString newText = match.captured( 1 ) + match.captured( 2 )
                          + "bres://" + id + "/"
                          + match.captured( 3 ) + match.captured( 2 );
        newLink = linkTxt.replace( match.capturedStart(), match.capturedLength(), newText );
      }
      else
        newLink = linkTxt.replace( stylesRe2,
                                   "\\1\"bres://" + id + "/\\2\"" );
    }
    else
    if( linkType.compare( "script" ) == 0 || linkType.compare( "img" ) == 0
        || linkType.compare( "source" ) == 0 )
    {
      // javascripts and images
      QRegularExpressionMatch match = inlineScriptRe.match( linkTxt );
      if( linkType.at( 1 ) == 'c' // "script" tag
          && match.hasMatch() && match.capturedLength() == linkTxt.length() )
      {
        // skip inline scripts
        articleNewText += linkTxt;
        match = closeScriptTagRe.match( article, linkPos );
        if( match.hasMatch() )
        {
          articleNewText += article.midRef( linkPos, match.capturedEnd() - linkPos );
          linkPos = match.capturedEnd();
        }
        continue;
      }
      else
      {
        match = srcRe.match( linkTxt );
        if( match.hasMatch() )
        {
          QString newText;
          if( linkType.at( 1 ) == 'o' ) // "source" tag
          {
            // Quote from https://developer.mozilla.org/en-US/docs/Web/HTML/Element/source:
            //   The <source> HTML element specifies multiple media resources for
            //   the <picture>, the <audio> element, or the <video> element.

            QString filename = match.captured( 3 );
#ifdef USE_QTWEBKIT
            // Store the file in a temporary cache directory and reference it in the link,
            // because gdvideo:// links don't work in the Qt WebKit version for some reason.
            // TODO (Qt WebKit): this workaround is applied to all <source> elements,
            // including those nested in <audio> and <picture> elements. Are such elements
            // present in any MDict dictionaries? If yes, is the workaround needed for them?
            QString newName = getCachedFileName( filename );
            newName.replace( '\\', '/' );
            newText = match.captured( 1 ) + match.captured( 2 )
                      + "file:///" + newName + match.captured( 2 );
#else
            QLatin1String scheme;
            auto const filenameStd = filename.toStdString();
            if( Filetype::isNameOfSound( filenameStd ) )
              scheme = QLatin1String{ "gdau" };
            else
            if( Filetype::isNameOfVideo( filenameStd ) )
              scheme = QLatin1String{ "gdvideo" };
            else
              scheme = QLatin1String{ "bres" };

            auto const quote = match.capturedView( 2 );
            // Use % in place of +, because operator+ is missing for QStringView.
            // TODO: consider whether defining QT_USE_QSTRINGBUILDER in goldendict.pro
            // to improve performance everywhere is safe enough (see QString documentation).
            newText = match.capturedView( 1 ) % quote % scheme % QLatin1String{ "://" }
                      % id % QLatin1Char{ '/' } % filename % quote;
#endif
          }
          else
          {
            newText = match.captured( 1 ) + match.captured( 2 )
                      + "bres://" + id + "/"
                      + match.captured( 3 ) + match.captured( 2 );
          }
          newLink = linkTxt.replace( match.capturedStart(), match.capturedLength(), newText );
        }
        else
          newLink = linkTxt.replace( srcRe2,
                                     "\\1\"bres://" + id + "/\\2\"" );
      }
    }
// The getCachedFileName() workaround in the Qt WebKit version eliminates the preload attribute value requirement.
#ifndef USE_QTWEBKIT
    else
    {
      Q_ASSERT( linkType == QLatin1String{ "audio" } || linkType == QLatin1String{ "video" } );
      // If the preload attribute value is not "auto", the video is unplayable
      // and the following errors appear in GoldenDict's output:
      // QIODevice::seek (ArticleResourceReply): Cannot call seek on a sequential device
      // [...:ERROR:batching_media_log.cc(38)] MediaEvent: {"error":"FFmpegDemuxer: open context failed"}
      // [...:ERROR:batching_media_log.cc(35)] MediaEvent: {"pipeline_error":12}
      // TODO (Qt WebEngine): is forcing the preload attribute value needed for <audio> elements?

      // Remove all existing preload attributes.
      static QRegularExpression const preloadRe( QStringLiteral( R"(\spreload\s*=\s*(["'])\w*\1)" ),
                                                 QRegularExpression::CaseInsensitiveOption );
      linkTxt.remove( preloadRe );

      // Insert the necessary preload attribute value at the end of the tag.
      Q_ASSERT( linkTxt.at( linkTxt.size() - 1 ) == QLatin1Char{ '>' } );
      linkTxt.insert( linkTxt.size() - 1, QLatin1String{ " preload=\"auto\"" } );

      newLink = linkTxt;
    }
#endif

    if( !newLink.isEmpty() )
    {
      articleNewText += newLink;
    }
    else
      articleNewText += allLinksMatch.captured();
  }
  if( linkPos )
  {
    articleNewText += article.midRef( linkPos );
    article = articleNewText;
  }

  return article;
}
#else
QString & MdxDictionary::filterResource( QString const & articleId, QString & article )
{
  QString id = QString::fromStdString( getId() );
  QString uniquePrefix = QString::fromLatin1( "g" ) + id + "_" + articleId + "_";

  QRegExp allLinksRe( "(?:<\\s*(a(?:rea)?|img|link|script|source)(?:\\s+[^>]+|\\s*)>)", Qt::CaseInsensitive );
  QRegExp wordCrossLink( "([\\s\"']href\\s*=)\\s*([\"'])entry://([^>#]*)((?:#[^>]*)?)\\2", Qt::CaseInsensitive );
  wordCrossLink.setMinimal( true );

  QRegExp anchorIdRe( "([\\s\"'](?:name|id)\\s*=)\\s*([\"'])\\s*(?=\\S)", Qt::CaseInsensitive );
  QRegExp anchorIdRe2( "([\\s\"'](?:name|id)\\s*=)\\s*(?=[^\"'])([^\\s\">]+)", Qt::CaseInsensitive );
  QRegExp anchorLinkRe( "([\\s\"']href\\s*=\\s*[\"'])entry://#", Qt::CaseInsensitive );
  QRegExp audioRe( "([\\s\"']href\\s*=)\\s*([\"'])sound://([^\">]+)\\2", Qt::CaseInsensitive );
  audioRe.setMinimal( true );

  QRegExp stylesRe( "([\\s\"']href\\s*=)\\s*([\"'])(?!\\s*\\b(?:(?:bres|https?|ftp)://|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\">]+)\\2",
                    Qt::CaseInsensitive, QRegExp::RegExp2 );
  stylesRe.setMinimal( true );
  QRegExp stylesRe2( "([\\s\"']href\\s*=)\\s*(?![\\s\"']|\\b(?:(?:bres|https?|ftp)://|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\\s\">]+)",
                     Qt::CaseInsensitive, QRegExp::RegExp2 );
  QRegExp inlineScriptRe( "<\\s*script(?:(?=\\s)(?:(?![\\s\"']src\\s*=)[^>])+|\\s*)>", Qt::CaseInsensitive, QRegExp::RegExp2 );
  QRegExp closeScriptTagRe( "<\\s*/script\\s*>", Qt::CaseInsensitive, QRegExp::RegExp2 );
  QRegExp srcRe( "([\\s\"']src\\s*=)\\s*([\"'])(?!\\s*\\b(?:(?:bres|https?|ftp)://|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\">]+)\\2",
                     Qt::CaseInsensitive, QRegExp::RegExp2 );
  srcRe.setMinimal( true );
  QRegExp srcRe2( "([\\s\"']src\\s*=)\\s*(?![\\s\"']|\\b(?:(?:bres|https?|ftp)://|(?:data|javascript):))(?:file://)?[\\x00-\\x1f\\x7f]*\\.*/?([^\\s\">]+)",
                    Qt::CaseInsensitive, QRegExp::RegExp2 );

  int linkPos = 0;
  while( linkPos >= 0 )
  {
    linkPos = allLinksRe.indexIn( article, linkPos );
    if( linkPos < 0 )
      break;

    QString linkTxt = allLinksRe.cap( 0 );
    QString linkType = allLinksRe.cap( 1 ).toLower();
    QString newLink;

    if( !linkType.isEmpty() && linkType.at( 0 ) == 'a' )
    {
      int pos = anchorIdRe.indexIn( linkTxt );
      if( pos >= 0 )
      {
        QString newText = anchorIdRe.cap( 1 ) + anchorIdRe.cap( 2 ) + uniquePrefix;
        newLink = linkTxt.replace( pos, anchorIdRe.cap().length(), newText );
      }
      else
        newLink = linkTxt.replace( anchorIdRe2, "\\1\"" + uniquePrefix + "\\2\"" );

      newLink = newLink.replace( anchorLinkRe, "\\1#" + uniquePrefix );

      pos = audioRe.indexIn( newLink );
      if( pos >= 0 )
      {
        // sounds and audio link script
        QString newTxt = audioRe.cap( 1 ) + audioRe.cap( 2 )
                         + "gdau://" + id + "/"
                         + audioRe.cap( 3 ) + audioRe.cap( 2 );
        newLink = QString::fromUtf8( addAudioLink( "\"gdau://" + getId() + "/" + audioRe.cap( 3 ).toUtf8().data() + "\"" ).c_str() )
                  + newLink.replace( pos, audioRe.cap().length(), newTxt );
      }

      pos = wordCrossLink.indexIn( newLink );
      if( pos >= 0 )
      {
        QString newTxt = wordCrossLink.cap( 1 ) + wordCrossLink.cap( 2 )
                         + "gdlookup://localhost/"
                         + wordCrossLink.cap( 3 );

        if( !wordCrossLink.cap( 4 ).isEmpty() )
          newTxt += QString( "?gdanchor=" ) + uniquePrefix + wordCrossLink.cap( 4 ).mid( 1 );

        newTxt += wordCrossLink.cap( 2 );
        newLink.replace( pos, wordCrossLink.cap( 0 ).length(), newTxt );
      }
    }
    else
    if( linkType.compare( "link" ) == 0 )
    {
      // stylesheets
      int pos = stylesRe.indexIn( linkTxt );
      if( pos >= 0 )
      {
        QString newText = stylesRe.cap( 1 ) + stylesRe.cap( 2 )
                          + "bres://" + id + "/"
                          + stylesRe.cap( 3 ) + stylesRe.cap( 2 );
        newLink = linkTxt.replace( pos, stylesRe.cap().length(), newText );
      }
      else
        newLink = linkTxt.replace( stylesRe2,
                                   "\\1\"bres://" + id + "/\\2\"" );
    }
    else
    if( linkType.compare( "script" ) == 0 || linkType.compare( "img" ) == 0
        || linkType.compare( "source" ) == 0 )
    {
      // javascripts and images
      if( linkType.at( 0 ) == 's' && inlineScriptRe.exactMatch( linkTxt ) )
      {
        // skip inline scripts
        linkPos += linkTxt.length();
        int pos = closeScriptTagRe.indexIn( article, linkPos );
        if( pos > 0 )
          linkPos = pos + closeScriptTagRe.cap().length();
        continue;
      }
      else
      {
        int pos = srcRe.indexIn( linkTxt );
        if( pos >= 0 )
        {
          QString newText;
          if( linkType.at( 1 ) == 'o' ) // "source" tag
          {
            QString filename = srcRe.cap( 3 );
            QString newName = getCachedFileName( filename );
            newName.replace( '\\', '/' );
            newText = srcRe.cap( 1 ) + srcRe.cap( 2 )
                      + "file:///" + newName + srcRe.cap( 2 );
          }
          else
          {
            newText = srcRe.cap( 1 ) + srcRe.cap( 2 )
                      + "bres://" + id + "/"
                      + srcRe.cap( 3 ) + srcRe.cap( 2 );
          }
          newLink = linkTxt.replace( pos, srcRe.cap().length(), newText );
        }
        else
          newLink = linkTxt.replace( srcRe2,
                                     "\\1\"bres://" + id + "/\\2\"" );
      }
    }
    if( !newLink.isEmpty() )
    {
      article.replace( linkPos, allLinksRe.cap().length(), newLink );
      linkPos += newLink.length();
    }
    else
      linkPos += allLinksRe.cap().length();
  }

  return article;
}
#endif

#ifdef USE_QTWEBKIT
QString MdxDictionary::getCachedFileName( QString filename )
{
  QDir dir;
  QFileInfo info( cacheDirName );
  if( !info.exists() || !info.isDir() )
  {
    if( !dir.mkdir( cacheDirName ) )
    {
      gdWarning( "Mdx: can't create cache directory \"%s\"", cacheDirName.toUtf8().data() );
      return QString();
    }
  }

  // Create subfolders if needed

  QString name = filename;
  name.replace( '/', '\\' );
  QStringList list = name.split( '\\' );
  int subFolders = list.size() - 1;
  if( subFolders > 0 )
  {
    QString dirName = cacheDirName;
    for( int i = 0; i < subFolders; i++ )
    {
      dirName += QDir::separator() + list.at( i );
      QFileInfo dirInfo( dirName );
      if( !dirInfo.exists() )
      {
        if( !dir.mkdir( dirName ) )
        {
          gdWarning( "Mdx: can't create cache directory \"%s\"", dirName.toUtf8().data() );
          return QString();
        }
      }
    }
  }

  QString fullName = cacheDirName + QDir::separator() + filename;

  info.setFile( fullName );
  if( !info.exists() )
  {
    QFile f( fullName );
    if( f.open( QFile::WriteOnly ) )
    {
      gd::wstring resourceName = FsEncoding::decode( filename.toStdString() );
      vector< char > data;

      // In order to prevent recursive internal redirection...
      set< QByteArray > resourceIncluded;

      for ( ;; )
      {

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

        try
        {
          // local file takes precedence
          string fn = FsEncoding::dirname( getDictionaryFilenames()[ 0 ] ) +
                      FsEncoding::separator() + u8ResourceName;
          File::loadFromFile( fn, data );
        }
        catch ( File::exCantOpen & )
        {
          for ( vector< sptr< IndexedMdd > >::const_iterator i = mddResources.begin();
                i != mddResources.end(); ++i )
          {
            if ( ( *i )->loadFile( resourceName, data ) )
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
        break;
      }

      qint64 n = 0;
      if( !data.empty() )
        n = f.write( data.data(), data.size() );

      f.close();

      if( n < (qint64)data.size() )
      {
        gdWarning( "Mdx: file \"%s\" writing error: \"%s\"", fullName.toUtf8().data(),
                                                             f.errorString().toUtf8().data() );
        return QString();
      }
    }
    else
    {
      gdWarning( "Mdx: file \"%s\" creating error: \"%s\"", fullName.toUtf8().data(),
                                                            f.errorString().toUtf8().data() );
      return QString();
    }
  }
  return fullName;
}

void MdxDictionary::removeDirectory( QString const & directory )
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
#endif // USE_QTWEBKIT

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
                                                      Dictionary::Initializing & initializing ) THROW_SPEC( std::exception )
{
  vector< sptr< Dictionary::Class > > dictionaries;

  for ( vector< string >::const_iterator i = fileNames.begin(); i != fileNames.end(); ++i )
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

      gdDebug( "MDict: Building the index for dictionary: %s\n", i->c_str() );

      MdictParser parser;
      list< sptr< MdictParser > > mddParsers;

      if ( !parser.open( i->c_str() ) )
        continue;

      string title = string( parser.title().toUtf8().constData() );
      initializing.indexingDictionary( title );

      for ( vector< string >::const_iterator mddIter = dictFiles.begin() + 1;
            mddIter != dictFiles.end(); ++mddIter )
      {
        if ( File::exists( *mddIter ) )
        {
          sptr< MdictParser > mddParser = new MdictParser();
          if ( !mddParser->open( mddIter->c_str() ) )
          {
            gdWarning( "Broken mdd (resource) file: %s\n", mddIter->c_str() );
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
        MdictParser::HeadWordIndex resourcesIndex;
        ResourceHandler resourceHandler( chunks, *mddIndexedWords );

        while ( mddParser->readNextHeadWordIndex( headWordIndex ) )
        {
          resourcesIndex.insert( resourcesIndex.end(), headWordIndex.begin(), headWordIndex.end() );
        }
        mddParser->readRecordBlock( resourcesIndex, resourceHandler );

        mddIndices.push_back( mddIndexedWords );
        // Save filename for .mdd files only
        QFileInfo fi( mddParser->filename() );
        mddFileNames.push_back( string( fi.fileName().toUtf8().constData() ) );
        mddParsers.pop_front();
      }

      // Finish with the chunks
      idxHeader.chunksOffset = chunks.finish();

      GD_DPRINTF( "Writing index...\n" );

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
              iter != styleSheets.end(); ++iter )
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
            mddIndexIter != mddIndices.end(); ++mddIndexIter )
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
