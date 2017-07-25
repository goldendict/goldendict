#ifndef __FTSHELPERS_HH_INCLUDED__
#define __FTSHELPERS_HH_INCLUDED__

#include <QString>
#include <QRegExp>
#include <QRunnable>
#include <QSemaphore>
#include <QList>

#include "dictionary.hh"
#include "btreeidx.hh"
#include "fulltextsearch.hh"
#include "chunkedstorage.hh"

#include <string>

namespace FtsHelpers
{

enum
{
  FtsSignature = 0x58535446, // FTSX on little-endian, XSTF on big-endian
  CurrentFtsFormatVersion = 2 + BtreeIndexing::FormatVersion,
};

#pragma pack(push,1)

struct FtsIdxHeader
{
  uint32_t signature; // First comes the signature, FTSX
  uint32_t formatVersion; // File format version
  uint32_t chunksOffset; // The offset to chunks' storage
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
  uint32_t wordCount; // Number of unique words this dictionary has
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

#pragma pack(pop)

bool ftsIndexIsOldOrBad( std::string const & indexFile,
                         BtreeIndexing::BtreeDictionary * dict );

bool parseSearchString( QString const & str, QStringList & IndexWords,
                        QStringList & searchWords,
                        QRegExp & searchRegExp, int searchMode,
                        bool matchCase,
                        int distanceBetweenWords,
                        bool & hasCJK );

void parseArticleForFts( uint32_t articleAddress, QString & articleText,
                         QMap< QString, QVector< uint32_t > > & words,
                         bool handleRoundBrackets = false );

void makeFTSIndex( BtreeIndexing::BtreeDictionary * dict, QAtomicInt & isCancelled );

bool isCJKChar( ushort ch );

class FTSResultsRequest;

class FTSResultsRequestRunnable : public QRunnable
{
  FTSResultsRequest & r;
  QSemaphore & hasExited;

public:

  FTSResultsRequestRunnable( FTSResultsRequest & r_,
                             QSemaphore & hasExited_ ) : r( r_ ),
                                                         hasExited( hasExited_ )
  {}

  ~FTSResultsRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class FTSResultsRequest : public Dictionary::DataRequest
{
  BtreeIndexing::BtreeDictionary & dict;

  QString searchString;
  int searchMode;
  bool matchCase;
  int distanceBetweenWords;
  int maxResults;
  bool hasCJK;
  bool ignoreWordsOrder;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

  QList< FTS::FtsHeadword > * foundHeadwords;

  void checkArticles( QVector< uint32_t > const & offsets,
                      QStringList const & words,
                      QRegExp const & searchRegexp = QRegExp() );

  void indexSearch( BtreeIndexing::BtreeIndex & ftsIndex,
                    sptr< ChunkedStorage::Reader > chunks,
                    QStringList & indexWords,
                    QStringList & searchWords );

  void combinedIndexSearch( BtreeIndexing::BtreeIndex & ftsIndex,
                            sptr< ChunkedStorage::Reader > chunks,
                            QStringList & indexWords,
                            QStringList & searchWords,
                            QRegExp & regexp );

  void fullIndexSearch( BtreeIndexing::BtreeIndex & ftsIndex,
                        sptr< ChunkedStorage::Reader > chunks,
                        QStringList & indexWords,
                        QStringList & searchWords,
                        QRegExp & regexp );

  void fullSearch( QStringList & searchWords, QRegExp & regexp );

public:

  FTSResultsRequest( BtreeIndexing::BtreeDictionary & dict_, QString const & searchString_,
                     int searchMode_, bool matchCase_, int distanceBetweenWords_, int maxResults_,
                     bool ignoreWordsOrder_ ):
    dict( dict_ ),
    searchString( searchString_ ),
    searchMode( searchMode_ ),
    matchCase( matchCase_ ),
    distanceBetweenWords( distanceBetweenWords_ ),
    maxResults( maxResults_ ),
    hasCJK( false ),
    ignoreWordsOrder( ignoreWordsOrder_ )
  {
    foundHeadwords = new QList< FTS::FtsHeadword >;
    QThreadPool::globalInstance()->start(
      new FTSResultsRequestRunnable( *this, hasExited ), -100 );
  }

  void run(); // Run from another thread by DslResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~FTSResultsRequest()
  {
    isCancelled.ref();
    if( foundHeadwords )
      delete foundHeadwords;
    hasExited.acquire();
  }
};

} // namespace

#endif // __FTSHELPERS_HH_INCLUDED__
