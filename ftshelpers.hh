#ifndef __FTSHELPERS_HH_INCLUDED__
#define __FTSHELPERS_HH_INCLUDED__

#include <QString>
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtCore5Compat/QRegExp>
#else
#include <QRegExp>
#endif
#include <QRunnable>
#include <QSemaphore>
#include <QList>

#include "dictionary.hh"
#include "btreeidx.hh"
#include "fulltextsearch.hh"
#include "chunkedstorage.hh"
#include "folding.hh"
#include "wstring_qt.hh"

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
                        bool & hasCJK,
                        bool ignoreWordsOrder = false );

void parseArticleForFts( uint32_t articleAddress, QString & articleText,
                         QMap< QString, QVector< uint32_t > > & words,
                         bool handleRoundBrackets = false );

void makeFTSIndex( BtreeIndexing::BtreeDictionary * dict, QAtomicInt & isCancelled );

bool isCJKChar( ushort ch );

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
  bool ignoreDiacritics;
  int wordsInIndex;

  QAtomicInt isCancelled;

  QAtomicInt results;

  QList< FTS::FtsHeadword > * foundHeadwords;

  void checkArticles( QVector< uint32_t > const & offsets,
                      QStringList const & words,
                      QRegExp const & searchRegexp = QRegExp() );
  QRegularExpression createMatchRegex( QRegExp const & searchRegexp );

  void checkSingleArticle( uint32_t offset,
                           QStringList const & words,
                           QRegularExpression const & searchRegexp = QRegularExpression() );

  void indexSearch( BtreeIndexing::BtreeIndex & ftsIndex,
                    sptr< ChunkedStorage::Reader > chunks,
                    QStringList & indexWords,
                    QStringList & searchWords, QRegExp & regexp );

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
                     bool ignoreWordsOrder_, bool ignoreDiacritics_ ):
    dict( dict_ ),
    searchString( searchString_ ),
    searchMode( searchMode_ ),
    matchCase( matchCase_ ),
    distanceBetweenWords( distanceBetweenWords_ ),
    maxResults( maxResults_ ),
    hasCJK( false ),
    ignoreWordsOrder( ignoreWordsOrder_ ),
    ignoreDiacritics( ignoreDiacritics_ ),
    wordsInIndex( 0 )
  {
    if( ignoreDiacritics_ )
      searchString = gd::toQString( Folding::applyDiacriticsOnly( gd::toWString( searchString_ ) ) );

    foundHeadwords = new QList< FTS::FtsHeadword >;
    results         = 0;
    QThreadPool::globalInstance()->start( [ this ]() { this->run(); }, -100 );
  }

  void run();

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~FTSResultsRequest()
  {
    isCancelled.ref();
    if( foundHeadwords )
      delete foundHeadwords;
  }
};

} // namespace

#endif // __FTSHELPERS_HH_INCLUDED__
