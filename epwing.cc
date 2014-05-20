/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "epwing_book.hh"
#include "epwing.hh"

#include <QByteArray>
#include <QDir>

#include <map>
#include <set>
#include <string>

#include "btreeidx.hh"
#include "folding.hh"
#include "gddebug.hh"
#include "fsencoding.hh"
#include "chunkedstorage.hh"
#include "wstring.hh"
#include "wstring_qt.hh"
#include "utf8.hh"
#include "filetype.hh"
#include "ftshelpers.hh"

namespace Epwing {

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

using std::map;
using std::multimap;
using std::vector;
using std::set;
using std::pair;
using gd::wstring;

namespace {

#pragma pack( push, 1 )

enum
{
  Signature = 0x58575045, // EPWX on little-endian, XWPE on big-endian
  CurrentFormatVersion = 1 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  quint32 signature; // First comes the signature, EPWX
  quint32 formatVersion; // File format version (CurrentFormatVersion)
  quint32 chunksOffset; // The offset to chunks' storage
  quint32 indexBtreeMaxElements; // Two fields from IndexInfo
  quint32 indexRootOffset;
  quint32 wordCount;
  quint32 articleCount;
  quint32 nameSize;
  quint32 langFrom;  // Source language
  quint32 langTo;    // Target language
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

#pragma pack( pop )

bool indexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion;
}

class EpwingDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;
  string bookName;
  ChunkedStorage::Reader chunks;
  Epwing::Book::EpwingBook eBook;
  QString cacheDirectory;

public:

  EpwingDictionary( string const & id, string const & indexFile,
                    vector< string > const & dictionaryFiles,
                    int subBook );

  ~EpwingDictionary();

  virtual string getName() throw()
  { return bookName; }

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

  virtual QString const& getDescription();

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const & alts,
                                                      wstring const & )
    throw( std::exception );

  virtual sptr< Dictionary::DataRequest > getResource( string const & name )
    throw( std::exception );

  virtual sptr< Dictionary::DataRequest > getSearchResults( QString const & searchString,
                                                            int searchMode, bool matchCase,
                                                            int distanceBetweenWords,
                                                            int maxResults );
  virtual void getArticleText( uint32_t articleAddress, QString & headword, QString & text );

  virtual void makeFTSIndex(QAtomicInt & isCancelled, bool firstIteration );

  virtual void setFTSParameters( Config::FullTextSearch const & fts )
  {
    if( ensureInitDone().size() )
      return;

    can_FTS = fts.enabled
              && !fts.disabledTypes.contains( "EPWING", Qt::CaseInsensitive )
              && ( fts.maxDictionarySize == 0 || getArticleCount() <= fts.maxDictionarySize );
  }

protected:

  void loadIcon() throw();

private:

  /// Loads the article.
  void loadArticle( quint32 address, string & articleHeadword,
                    string & articleText );

  void createCacheDirectory();

  void removeDirectory( QString const & directory );

  QString const & getImagesCacheDir()
  { return eBook.getImagesCacheDir(); }

  QString const & getSoundsCacheDir()
  { return eBook.getSoundsCacheDir(); }

  QString const & getMoviesCacheDir()
  { return eBook.getMoviesCacheDir(); }

  friend class EpwingArticleRequest;
  friend class EpwingResourceRequest;
};


EpwingDictionary::EpwingDictionary( string const & id,
                                    string const & indexFile,
                                    vector< string > const & dictionaryFiles,
                                    int subBook ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() ),
  chunks( idx, idxHeader.chunksOffset )
{
  vector< char > data( idxHeader.nameSize );
  idx.seek( sizeof( idxHeader ) );
  idx.read( &data.front(), idxHeader.nameSize );
  bookName = string( &data.front(), idxHeader.nameSize );

  // Initialize eBook

  eBook.setBook( dictionaryFiles[ 0 ] );
  eBook.setSubBook( subBook );

  // Initialize the index

  openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
             idx, idxMutex );

  eBook.setDictID( getId() );

  cacheDirectory = QDir::tempPath() + QDir::separator()
                   + QString::fromUtf8( getId().c_str() )
                   + ".cache";
  eBook.setCacheDirectory( cacheDirectory );

  // Full-text search parameters

  can_FTS = true;

  ftsIdxName = indexFile + "_FTS";

  if( !Dictionary::needToRebuildIndex( dictionaryFiles, ftsIdxName )
      && !FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName ) )
    FTS_index_completed.ref();
}

EpwingDictionary::~EpwingDictionary()
{
  removeDirectory( cacheDirectory );
}

void EpwingDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString dirName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );
  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 1 ].c_str() ) );

  int pos = fileName.indexOf( '/', dirName.size(), Qt::CaseSensitive );

  if( pos > 0 )
  {
    fileName = fileName.left( pos );
    loadIconFromFile( fileName );
  }

  if( dictionaryIcon.isNull() )
  {
    // Load failed -- use default icons
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/icon32_epwing.png");
  }

  dictionaryIconLoaded = true;
}

void EpwingDictionary::removeDirectory( QString const & directory )
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

void EpwingDictionary::loadArticle( quint32 address,
                                    string & articleHeadword,
                                    string & articleText )
{
  vector< char > chunk;

  char * articleProps;

  {
    Mutex::Lock _( idxMutex );
    articleProps = chunks.getBlock( address, chunk );
  }

  uint32_t articlePage, articleOffset;

  memcpy( &articlePage, articleProps, sizeof( articlePage ) );
  memcpy( &articleOffset, articleProps + sizeof( articlePage ),
          sizeof( articleOffset ) );

  QString headword, text;

  try
  {
    Mutex::Lock _( eBook.getLibMutex() );
    eBook.getArticle( headword, text, articlePage, articleOffset, false );
  }
  catch( std::exception & e )
  {
    text = QString( "Article reading error: %1")
           .arg( QString::fromUtf8( e.what() ) );
  }

  articleHeadword = string( headword.toUtf8().data() );
  articleText = string( text.toUtf8().data() );

  string prefix( "<div class=\"epwing_text\">" );

  articleText = prefix + articleText + "</div>";
}

QString const& EpwingDictionary::getDescription()
{
  if( !dictionaryDescription.isEmpty() )
    return dictionaryDescription;

  dictionaryDescription = "NONE";

  QString str;
  {
    Mutex::Lock _( eBook.getLibMutex() );
    str = eBook.copyright();
  }

  if( !str.isEmpty() )
    dictionaryDescription = str;

  return dictionaryDescription;
}

void EpwingDictionary::makeFTSIndex( QAtomicInt & isCancelled, bool firstIteration )
{
  if( !( Dictionary::needToRebuildIndex( getDictionaryFilenames(), ftsIdxName )
         || FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName ) ) )
    FTS_index_completed.ref();


  if( haveFTSIndex() )
    return;

  if( firstIteration && getArticleCount() > FTS::MaxDictionarySizeForFastSearch )
    return;

  gdDebug( "Epwing: Building the full-text index for dictionary: %s\n",
           getName().c_str() );

  try
  {
    FtsHelpers::makeFTSIndex( this, isCancelled );
    FTS_index_completed.ref();
  }
  catch( std::exception &ex )
  {
    gdWarning( "DSL: Failed building full-text search index for \"%s\", reason: %s\n", getName().c_str(), ex.what() );
    QFile::remove( FsEncoding::decode( ftsIdxName.c_str() ) );
  }
}

void EpwingDictionary::getArticleText( uint32_t articleAddress, QString & headword, QString & text )
{
  headword.clear();
  text.clear();

  vector< char > chunk;
  char * articleProps;

  {
    Mutex::Lock _( idxMutex );
    articleProps = chunks.getBlock( articleAddress, chunk );
  }

  uint32_t articlePage, articleOffset;

  memcpy( &articlePage, articleProps, sizeof( articlePage ) );
  memcpy( &articleOffset, articleProps + sizeof( articlePage ),
          sizeof( articleOffset ) );

  try
  {
    Mutex::Lock _( eBook.getLibMutex() );
    eBook.getArticle( headword, text, articlePage, articleOffset, true );
  }
  catch( std::exception & e )
  {
    text = QString( "Article reading error: %1")
           .arg( QString::fromUtf8( e.what() ) );
  }
}

/// EpwingDictionary::getArticle()

class EpwingArticleRequest;

class EpwingArticleRequestRunnable: public QRunnable
{
  EpwingArticleRequest & r;
  QSemaphore & hasExited;

public:

  EpwingArticleRequestRunnable( EpwingArticleRequest & r_,
                                QSemaphore & hasExited_ ): r( r_ ),
                                                           hasExited( hasExited_ )
  {}

  ~EpwingArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class EpwingArticleRequest: public Dictionary::DataRequest
{
  friend class EpwingArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  EpwingDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  EpwingArticleRequest( wstring const & word_,
                        vector< wstring > const & alts_,
                        EpwingDictionary & dict_ ):
    word( word_ ), alts( alts_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new EpwingArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by EpwingArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~EpwingArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void EpwingArticleRequestRunnable::run()
{
  r.run();
}

void EpwingArticleRequest::run()
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

    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    // Now grab that article

    string headword, articleText;

    try
    {
      dict.loadArticle( chain[ x ].articleOffset, headword, articleText );
    }
    catch(...)
    {
    }

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

  string result = "<span class=\"epwing_article\">";

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

  result += "</span>";

  Mutex::Lock _( dataMutex );

  data.resize( result.size() );

  memcpy( &data.front(), result.data(), result.size() );

  hasAnyData = true;

  finish();
}

sptr< Dictionary::DataRequest > EpwingDictionary::getArticle( wstring const & word,
                                                              vector< wstring > const & alts,
                                                              wstring const & )
  throw( std::exception )
{
  return new EpwingArticleRequest( word, alts, *this );
}

//// EpwingDictionary::getResource()

class EpwingResourceRequest;

class EpwingResourceRequestRunnable: public QRunnable
{
  EpwingResourceRequest & r;
  QSemaphore & hasExited;

public:

  EpwingResourceRequestRunnable( EpwingResourceRequest & r_,
                              QSemaphore & hasExited_ ): r( r_ ),
                                                         hasExited( hasExited_ )
  {}

  ~EpwingResourceRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class EpwingResourceRequest: public Dictionary::DataRequest
{
  friend class EpwingResourceRequestRunnable;

  EpwingDictionary & dict;

  string resourceName;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  EpwingResourceRequest( EpwingDictionary & dict_,
                      string const & resourceName_ ):
    dict( dict_ ),
    resourceName( resourceName_ )
  {
    QThreadPool::globalInstance()->start(
      new EpwingResourceRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by EpwingResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~EpwingResourceRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void EpwingResourceRequestRunnable::run()
{
  r.run();
}

void EpwingResourceRequest::run()
{
  // Some runnables linger enough that they are cancelled before they start
  if ( isCancelled )
  {
    finish();
    return;
  }

  QString cacheDir;
  {
    Mutex::Lock _( dict.eBook.getLibMutex() );
    if( Filetype::isNameOfPicture( resourceName ) )
      cacheDir = dict.getImagesCacheDir();
    else
    if( Filetype::isNameOfSound( resourceName ) )
      cacheDir = dict.getSoundsCacheDir();
    else
    if( Filetype::isNameOfVideo( resourceName ) )
      cacheDir = dict.getMoviesCacheDir();
  }

  try
  {
    if( cacheDir.isEmpty() )
    {
      finish();
      return;
    }

    QString fullName = cacheDir + QDir::separator()
                       + FsEncoding::decode( resourceName.c_str() );

    QFile f( fullName );
    if( f.open( QFile::ReadOnly ) )
    {
      QByteArray buffer = f.readAll();

      Mutex::Lock _( dataMutex );

      data.resize( buffer.size() );

      memcpy( &data.front(), buffer.data(), data.size() );

      hasAnyData = true;
    }
  }
  catch( std::exception &ex )
  {
    gdWarning( "Epwing: Failed loading resource \"%s\" for \"%s\", reason: %s\n",
               resourceName.c_str(), dict.getName().c_str(), ex.what() );
    // Resource not loaded -- we don't set the hasAnyData flag then
  }

  finish();
}

sptr< Dictionary::DataRequest > EpwingDictionary::getResource( string const & name )
  throw( std::exception )
{
  return new EpwingResourceRequest( *this, name );
}


sptr< Dictionary::DataRequest > EpwingDictionary::getSearchResults( QString const & searchString,
                                                                    int searchMode, bool matchCase,
                                                                    int distanceBetweenWords,
                                                                    int maxResults )
{
  return new FtsHelpers::FTSResultsRequest( *this, searchString,searchMode, matchCase, distanceBetweenWords, maxResults );
}

} // anonymous namespace

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & initializing )
  throw( std::exception )
{
  vector< sptr< Dictionary::Class > > dictionaries;

  vector< string > dictFiles;
  QByteArray catName;
  catName += QDir::separator();
  catName += "catalogs";

  for( vector< string >::const_iterator i = fileNames.begin(); i != fileNames.end();
       ++i )
  {
      // Skip files other than "catalogs" to speed up the scanning

      if ( i->size() < (unsigned)catName.size() ||
          strcasecmp( i->c_str() + ( i->size() - catName.size() ), catName.data() ) != 0 )
        continue;

      int ndir = i->size() - catName.size();
      if( ndir < 1 )
        ndir = 1;

      string mainDirectory = i->substr( 0, ndir );

      Epwing::Book::EpwingBook dict;

      int subBooksNumber = dict.setBook( mainDirectory );

      for( int sb = 0; sb < subBooksNumber; sb++ )
      {
        QString dir;

        try
        {
          dictFiles.clear();
          dictFiles.push_back( mainDirectory );
          dictFiles.push_back( *i );

          dict.setSubBook( sb );

          dir = FsEncoding::decode( mainDirectory.c_str() )
                + FsEncoding::separator()
                + dict.getCurrentSubBookDirectory();

          Epwing::Book::EpwingBook::collectFilenames( dir, dictFiles );

          // Check if we need to rebuid the index

          string dictId = Dictionary::makeDictionaryId( dictFiles );

          string indexFile = indicesDir + dictId;

          if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
                 indexIsOldOrBad( indexFile ) )
          {
            gdDebug( "Epwing: Building the index for dictionary in directory %s\n", dir.toUtf8().data() );

            QString str = dict.title();
//            QByteArray nameData = dict.title().toUtf8();
            QByteArray nameData = str.toUtf8();
            initializing.indexingDictionary( nameData.data() );

            File::Class idx( indexFile, "wb" );

            IdxHeader idxHeader;

            memset( &idxHeader, 0, sizeof( idxHeader ) );

            // We write a dummy header first. At the end of the process the header
            // will be rewritten with the right values.

            idx.write( idxHeader );

            idx.write( nameData.data(), nameData.size() );
            idxHeader.nameSize = nameData.size();

            IndexedWords indexedWords;

            ChunkedStorage::Writer chunks( idx );

            Epwing::Book::EpwingHeadword head;

            dict.getFirstHeadword( head );

            int wordCount = 0;

            for( ; ; )
            {
              uint32_t offset = chunks.startNewBlock();
              chunks.addToBlock( &head.page, sizeof( head.page ) );
              chunks.addToBlock( &head.offset, sizeof( head.offset ) );

              indexedWords.addWord( gd::toWString( head.headword ), offset );

              wordCount++;

              if( !dict.getNextHeadword( head ) )
                break;
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

            idxHeader.wordCount = wordCount;
            idxHeader.articleCount = wordCount;

            idx.rewind();

            idx.write( &idxHeader, sizeof( idxHeader ) );


          } // If need to rebuild

          dictionaries.push_back( new EpwingDictionary( dictId,
                                                        indexFile,
                                                        dictFiles,
                                                        sb ) );
        }
        catch( std::exception & e )
        {
          gdWarning( "Epwing dictionary initializing failed: %s, error: %s\n",
                     dir.toUtf8().data(), e.what() );
          continue;
        }
      }

  }
  return dictionaries;
}

} // namespace Epwing
