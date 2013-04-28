/* This file is (c) 2013 Timon Wong <timon86.wang.gmail.com>
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


/// Checks if the given string ends with the given substring
static bool endsWith( string const & str, string const & tail )
{
  return str.size() >= tail.size() &&
         str.compare( str.size() - tail.size(), tail.size(), tail ) == 0;
}

enum
{
  kSignature = 0x4349444d,  // MDIC
  kCurrentFormatVersion = 7 + BtreeIndexing::FormatVersion
};

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

  uint32_t hasMddFile;
  uint32_t mddIndexBtreeMaxElements;
  uint32_t mddIndexRootOffset;
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
  IndexedMdd mddResource;
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
  string filterResource( const char * articleId, const char * article );

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
  mddResource( idxMutex, chunks ),
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
        int key = idx.read<int>();
        vector< char > buf;
        string::size_type sz;

        sz = idx.read< string::size_type >();
        buf.resize( sz );
        idx.read( &buf.front(), sz );
        QString styleBegin = QString::fromUtf8( buf.data() );

        sz = idx.read< string::size_type >();
        buf.resize( sz );
        idx.read( &buf.front(), sz );
        QString styleEnd = QString::fromUtf8( buf.data() );

        styleSheets[ key ] = pair<QString, QString>( styleBegin, styleEnd );
      }

      // Initialize the index
      openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                            idxHeader.indexRootOffset ), idx, idxMutex );

      for ( vector<string>::const_iterator i = getDictionaryFilenames().begin();
            i != getDictionaryFilenames().end(); i++ )
      {
        if ( endsWith( *i, ".mdd" ) && File::exists( *i ) )
        {
          if ( idxHeader.hasMddFile && ( idxHeader.mddIndexBtreeMaxElements ||
                                         idxHeader.mddIndexRootOffset ) )
          {
            mddResource.openIndex( IndexInfo( idxHeader.mddIndexBtreeMaxElements,
                                              idxHeader.mddIndexRootOffset ),
                                   idx, idxMutex );
            mddResource.open( i->c_str() );
          }
        }
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

    dict.loadArticle( chain[ x ].articleOffset, articleBody );

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

    // Convert to the Windows separator
    std::replace( resourceName.begin(), resourceName.end(), '/', '\\' );
    if ( resourceName[ 0 ] != '\\' )
    {
      resourceName.insert( 0, 1, '\\' );
    }

    string u8ResourceName = Utf8::encode( resourceName );
    QCryptographicHash hash( QCryptographicHash::Md5 );
    hash.addData( u8ResourceName.data(), u8ResourceName.size() );
    if ( !resourceIncluded.insert( hash.result() ).second )
      continue;

    // Get actual resource
    Mutex::Lock _( dataMutex );
    data.clear();
    if ( dict.mddResource.loadFile( resourceName, data ) )
    {
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

      hasAnyData = true;
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

DEF_EX( exCorruptDictionary, "dictionary file tampered or corrupted", std::exception )

void MdxDictionary::loadArticle( uint32_t offset, string & articleText )
{
  vector< char > chunk;
  Mutex::Lock _( idxMutex );

  // Load record info from index
  MdictParser::RecordInfo recordInfo;
  char * pRecordInfo = chunks.getBlock( offset, chunk );
  memcpy( &recordInfo, pRecordInfo, sizeof( recordInfo ) );

  // Make an sub unique id for this article
  QString articleId;
  articleId.setNum( ( quint64 )pRecordInfo, 16 );

  articleText = string( QObject::tr( "Article loading error" ).toUtf8().constData() );

  try
  {
    ScopedMemMap compressed( dictFile, recordInfo.compressedBlockPos, recordInfo.compressedBlockSize );
    if ( !compressed.startAddress() )
      throw exCorruptDictionary();

    QByteArray decompressed;
    if ( !MdictParser::parseCompressedBlock( recordInfo.compressedBlockSize, ( char * )compressed.startAddress(),
                                             recordInfo.decompressedBlockSize, decompressed ) )
      return;

    QString article = MdictParser::toUtf16( encoding.c_str(),
                                            decompressed.constData() + recordInfo.recordOffset,
                                            recordInfo.recordSize );

    article = MdictParser::substituteStylesheet( article, styleSheets );
    articleText = filterResource( articleId.toLatin1().constData(), article.toUtf8().constData() );
  }
  catch ( std::exception & e )
  {
    FDPRINTF( stderr, "MDict: load article from %s failed, error: %s\n",
              getDictionaryFilenames()[ 0 ].c_str(), e.what() );
  }
  catch ( ... )
  {
    FDPRINTF( stderr, "MDict: load article from %s failed, error: %s\n",
              getDictionaryFilenames()[ 0 ].c_str(), "unknown error" );
  }
}

string MdxDictionary::filterResource( const char * articleId, const char * article )
{
  QString id = QString::fromStdString( getId() );
  QString uniquePrefix = QString::fromStdString( getId() + "_" + articleId + "_" );

  return string( QString::fromUtf8( article )
                 // word cross links
                 .replace( QRegExp( "(href\\s*=\\s*[\"'])entry://([^#\"']+)#?[^\"']*", Qt::CaseInsensitive ),
                           "\\1gdlookup://localhost/\\2" )
                 // anchors
                 .replace( QRegExp( "(href\\s*=\\s*[\"'])entry://#", Qt::CaseInsensitive ),
                           "\\1#" + uniquePrefix )
                 .replace( QRegExp( "(<\\s*a\\s+[^>]*(name|id)\\s*=\\s*\")", Qt::CaseInsensitive ),
                           "\\1" + uniquePrefix )
                 // sounds, and audio link script
                 .replace( QRegExp( "(<\\s*a\\s+[^>]*href\\s*=\\s*\")sound://([^\"']*)", Qt::CaseInsensitive ),
                           QString::fromStdString( addAudioLink( "\"gdau://" + getId() + "/\\2\"", getId() ) ) +
                           "\\1gdau://" + id + "/\\2" )
                 // stylesheets
                 .replace( QRegExp( "(<\\s*link\\s+[^>]*href\\s*=\\s*[\"']+)(?:file://)?[\\x00-\\x30\\x7f]*([^\"']*)",
                                    Qt::CaseInsensitive, QRegExp::RegExp2 ),
                           "\\1bres://" + id + "/\\2" )
                 .replace( QRegExp( "(<\\s*link\\s+[^>]*href\\s*=\\s*)(?!['\"]+)(?!bres:|data:)(?:file://)?([^\\s>]+)",
                                    Qt::CaseInsensitive, QRegExp::RegExp2 ),
                           "\\1\"bres://" + id + "/\\\"" )
                 // images
                 .replace( QRegExp( "(<\\s*img\\s+[^>]*src\\s*=\\s*[\"']+)(?:file://)?[\\x00-\\x30\\x7f]*([^\"']*)",
                                    Qt::CaseInsensitive, QRegExp::RegExp2 ),
                           "\\1bres://" + id + "/\\2" )
                 .replace( QRegExp( "(<\\s*img\\s+[^>]*src\\s*=\\s*)(?!['\"]+)(?!bres:|data:)(?:file://)?([^\\s>]+)",
                                    Qt::CaseInsensitive, QRegExp::RegExp2 ),
                           "\\1\"bres://" + id + "/\\2\"" )
                 .toUtf8().constData() );
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


static bool indexIsOldOrBad( string const & indexFile, bool hasMddFile )
{
  File::Class idx( indexFile, "rb" );
  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != kSignature ||
         header.formatVersion != kCurrentFormatVersion ||
         header.parserVersion != MdictParser::kParserVersion ||
         header.foldingVersion != Folding::Version ||
         header.hasMddFile != hasMddFile;
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

    string baseName = ( ( *i )[ i->size() - 4 ] == '.' ) ?
                      string( *i, 0, i->size() - 4 ) : string( *i, 0, i->size() - 7 );

    // Check if there' is any file end with .mdd, which is the resource file for the dictionary
    string mddFileName;
    if ( File::tryPossibleName( baseName + ".mdd", mddFileName ) )
      dictFiles.push_back( mddFileName );

    string dictId = Dictionary::makeDictionaryId( dictFiles );

    string indexFile = indicesDir + dictId;

    if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
         indexIsOldOrBad( indexFile, !mddFileName.empty() ) )
    {
      // Building the index
      MdictParser parser( i->c_str() );
      sptr<MdictParser> mddParser = NULL;

      if ( !parser.open() )
        continue;

      if ( File::exists( mddFileName ) )
      {
        mddParser = new MdictParser( mddFileName.c_str() );
        if ( !mddParser->open() )
        {
          FDPRINTF( stderr, "Warning: Invalid mdd (resource) file: %s\n", mddFileName.c_str() );
          continue;
        }
      }

      string title = string( parser.title().toUtf8().constData() );
      initializing.indexingDictionary( title );

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
      sptr<IndexedWords> mddIndexedWords;
      if ( mddParser )
      {
        mddIndexedWords = new IndexedWords();
        ResourceHandler resourceHandler( chunks, *mddIndexedWords );

        while ( mddParser->readNextHeadWordIndex( headWordIndex ) )
        {
          mddParser->readRecordBlock( headWordIndex, resourceHandler );
        }
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
          idx.write<int>( iter->first );
          // styleBegin
          idx.write<string::size_type>( styleBegin.size() + 1 );
          idx.write( styleBegin.c_str(), styleBegin.size() + 1 );
          // styleEnd
          idx.write<string::size_type>( styleEnd.size() + 1 );
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

      if ( mddParser )
      {
        IndexInfo resourceIdxInfo = BtreeIndexing::buildIndex( *mddIndexedWords, idx );
        idxHeader.hasMddFile = true;
        idxHeader.mddIndexBtreeMaxElements = resourceIdxInfo.btreeMaxElements;
        idxHeader.mddIndexRootOffset = resourceIdxInfo.rootOffset;
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
