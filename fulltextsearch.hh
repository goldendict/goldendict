#ifndef __FULLTEXTSEARCH_HH_INCLUDED__
#define __FULLTEXTSEARCH_HH_INCLUDED__

#include <QSemaphore>
#include <QStringList>
#include <QRegExp>
#include <QAbstractListModel>
#include <QList>

#include "dictionary.hh"
#include "ui_fulltextsearch.h"
#include "mutex.hh"
#include "config.hh"
#include "instances.hh"
#include "delegate.hh"

namespace FTS
{

enum {
  // Minimum word length for indexing
  MinimumWordSize = 4,

  // Maximum dictionary size for first iteration of FTS indexing
  MaxDictionarySizeForFastSearch = 150000,

  // Maxumum match length for highlight search results
  // (QWebPage::findText() crashes on too long strings)
  MaxMatchLengthForHighlightResults = 500
};

enum SearchMode
{
  WholeWords = 0,
  PlainText,
  Wildcards,
  RegExp
};

struct FtsHeadword
{
  QString headword;
  QStringList dictIDs;
  QStringList foundHiliteRegExps;
  bool matchCase;

  FtsHeadword( QString const & headword_, QString const & dictid_,
               QStringList hilites, bool match_case ) :
    headword( headword_ ),
    foundHiliteRegExps( hilites ),
    matchCase( match_case )
  {
    dictIDs.append( dictid_ );
  }

  QString trimQuotes( QString const & ) const;

  bool operator <( FtsHeadword const & other ) const;

  bool operator ==( FtsHeadword const & other ) const
  { return headword.compare( other.headword, Qt::CaseInsensitive ) == 0; }

  bool operator !=( FtsHeadword const & other ) const
  { return headword.compare( other.headword, Qt::CaseInsensitive ) != 0; }
};

class Indexing : public QObject, public QRunnable
{
Q_OBJECT

  QAtomicInt & isCancelled;
  std::vector< sptr< Dictionary::Class > > const & dictionaries;
  QSemaphore & hasExited;

public:
  Indexing( QAtomicInt & cancelled, std::vector< sptr< Dictionary::Class > > const & dicts,
            QSemaphore & hasExited_):
    isCancelled( cancelled ),
    dictionaries( dicts ),
    hasExited( hasExited_ )
  {}

  ~Indexing()
  {
    hasExited.release();
  }

  virtual void run();

signals:
  void sendNowIndexingName( QString );
};

class FtsIndexing : public QObject
{
Q_OBJECT
public:
  FtsIndexing( std::vector< sptr< Dictionary::Class > > const & dicts );
  virtual ~FtsIndexing()
  { stopIndexing(); }

  void setDictionaries( std::vector< sptr< Dictionary::Class > > const & dicts )
  {
    clearDictionaries();
    dictionaries = dicts;
  }

  void clearDictionaries()
  { dictionaries.clear(); }

  /// Start dictionaries indexing for full-text search
  void doIndexing();

  /// Break indexing thread
  void stopIndexing();

  QString nowIndexingName();

protected:
  QAtomicInt isCancelled;
  QSemaphore indexingExited;
  std::vector< sptr< Dictionary::Class > > dictionaries;
  bool started;
  QString nowIndexing;
  Mutex nameMutex;

private slots:
  void setNowIndexedName( QString name );

signals:
  void newIndexingName( QString name );
};

/// A model to be projected into the view, according to Qt's MVC model
class HeadwordsListModel: public QAbstractListModel
{
  Q_OBJECT

public:

  HeadwordsListModel( QWidget * parent, QList< FtsHeadword > & headwords_,
                      std::vector< sptr< Dictionary::Class > > const & dicts ):
    QAbstractListModel( parent ), headwords( headwords_ ),
    dictionaries( dicts )
  {}

  int rowCount( QModelIndex const & parent ) const;
  QVariant data( QModelIndex const & index, int role ) const;

//  bool insertRows( int row, int count, const QModelIndex & parent );
//  bool removeRows( int row, int count, const QModelIndex & parent );
//  bool setData( QModelIndex const & index, const QVariant & value, int role );

  void addResults(const QModelIndex & parent, QList< FtsHeadword > const & headwords );
  bool clear();

private:

  QList< FtsHeadword > & headwords;
  std::vector< sptr< Dictionary::Class > > const & dictionaries;

  int getDictIndex( QString const & id ) const;

signals:
  void contentChanged();
};

class FullTextSearchDialog : public QDialog
{
  Q_OBJECT

  Config::Class & cfg;
  std::vector< sptr< Dictionary::Class > > const & dictionaries;
  std::vector< Instances::Group > const & groups;
  unsigned group;
  std::vector< sptr< Dictionary::Class > > activeDicts;
  bool ignoreWordsOrder;

  std::list< sptr< Dictionary::DataRequest > > searchReqs;

  FtsIndexing & ftsIdx;

  QRegExp searchRegExp;

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) ) && defined( Q_OS_WIN32 )
  QStyle * barStyle, * oldBarStyle;
#endif

public:
  FullTextSearchDialog( QWidget * parent,
                        Config::Class & cfg_,
                        std::vector< sptr< Dictionary::Class > > const & dictionaries_,
                        std::vector< Instances::Group > const & groups_,
                        FtsIndexing & ftsidx );
  virtual ~FullTextSearchDialog();

  void setCurrentGroup( unsigned group_ )
  { group = group_; updateDictionaries(); }

  void stopSearch();

protected:
  bool eventFilter( QObject * obj, QEvent * ev );

private:
  Ui::FullTextSearchDialog ui;
  QList< FtsHeadword > results;
  HeadwordsListModel * model;
  WordListItemDelegate * delegate;
  QAction helpAction;

  void showDictNumbers();

private slots:
  void setNewIndexingName( QString );
  void saveData();
  void accept();
  void setLimitsUsing();
  void ignoreWordsOrderClicked();
  void searchReqFinished();
  void reject();
  void itemClicked( QModelIndex const & idx );
  void updateDictionaries();
  void helpRequested();

signals:
  void showTranslationFor( QString const &, QStringList const & dictIDs,
                           QRegExp const & searchRegExp );
  void closeDialog();
};


} // namespace FTS

#endif // __FULLTEXTSEARCH_HH_INCLUDED__
