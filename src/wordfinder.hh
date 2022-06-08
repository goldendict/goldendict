/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __WORDFINDER_HH_INCLUDED__
#define __WORDFINDER_HH_INCLUDED__

#include <list>
#include <map>
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QRunnable>
#include "dict/dictionary.hh"

/// This component takes care of finding words. The search is asynchronous.
/// This means the GUI doesn't get blocked during the sometimes lenghtly
/// process of finding words.
class WordFinder: public QObject
{
  Q_OBJECT

public:

  typedef std::vector< std::pair< QString, bool > > SearchResults; // bool is a "was suggested" flag

private:

  SearchResults searchResults;
  QString searchErrorString;
  bool searchResultsUncertain;
  std::list< sptr< Dictionary::WordSearchRequest > > queuedRequests,
                                                     finishedRequests;
  bool searchInProgress;

  QTimer updateResultsTimer;

  // Saved search params
  bool searchQueued;
  QString inputWord;
  enum SearchType
  {
    PrefixMatch,
    StemmedMatch,
    ExpressionMatch
  } searchType;
  unsigned long requestedMaxResults;
  Dictionary::Features requestedFeatures;
  unsigned stemmedMinLength;
  unsigned stemmedMaxSuffixVariation;

  std::vector< sptr< Dictionary::Class > > const * inputDicts;

  std::vector< gd::wstring > allWordWritings; // All writings of the inputWord
  
  struct OneResult
  {
    gd::wstring word;
    int rank;
    bool wasSuggested;
  };

  // Maps lowercased string to the original one. This catches all duplicates
  // without case sensitivity. Made as an array and a map indexing that array.
  typedef std::list< OneResult > ResultsArray;
  typedef std::map< gd::wstring, ResultsArray::iterator > ResultsIndex;
  ResultsArray resultsArray;
  ResultsIndex resultsIndex;
    
public:

  WordFinder( QObject * parent );
  ~WordFinder();

  /// Do the standard prefix-match search in the given list of dictionaries.
  /// Some dictionaries might only support exact matches -- for them, only
  /// the exact matches would be found. All search results are put into a single
  /// list containing the exact matches first, then the prefix ones. Duplicate
  /// matches from different dictionaries are merged together.
  /// If a list of features is specified, the search will only be performed in
  /// the dictionaries which possess all the features requested.
  /// If there already was a prefixMatch operation underway, it gets cancelled
  /// and the new one replaces it.
  void prefixMatch( QString const &,
                    std::vector< sptr< Dictionary::Class > > const &,
                    unsigned long maxResults = 40,
                    Dictionary::Features = Dictionary::NoFeatures );

  /// Do a stemmed-match search in the given list of dictionaries. All comments
  /// from prefixMatch() generally apply as well.
  void stemmedMatch( QString const &,
                     std::vector< sptr< Dictionary::Class > > const &,
                     unsigned minLength = 3,
                     unsigned maxSuffixVariation = 3,
                     unsigned long maxResults = 30,
                     Dictionary::Features = Dictionary::NoFeatures );
  
  /// Do the expression-match search in the given list of dictionaries.
  /// Function find exact matches for one of spelling suggestions.
  void expressionMatch( QString const &,
                        std::vector< sptr< Dictionary::Class > > const &,
                        unsigned long maxResults = 40,
                        Dictionary::Features = Dictionary::NoFeatures );

  /// Returns the vector containing search results from the last operation.
  /// If it didn't finish yet, the result is not final and may be changing
  /// over time.
  SearchResults const & getResults() const
  { return searchResults; }

  /// Returns a human-readable error string for the last finished request. Empty
  /// string means it finished without any error.
  QString const & getErrorString()
  { return searchErrorString; }

  /// Returns true if the search was inconclusive -- that is, there may be more
  /// results than the ones returned.
  bool wasSearchUncertain() const
  { return searchResultsUncertain; }

  /// Cancels any pending search operation, if any.
  void cancel();

  /// Cancels any pending search operation, if any, and makes sure no pending
  /// requests exist, and hence no dictionaries are used anymore. Unlike
  /// cancel(), this may take some time to finish.
  void clear();

signals:

  /// Indicates that the search has got some more results, and continues
  /// searching.
  void updated();

  /// Indicates that the search has finished.
  void finished();

private slots:

  /// Called each time one of the requests gets finished
  void requestFinished();

  /// Called by updateResultsTimer to update searchResults and signal updated()
  void updateResults();

private:

  // Starts the previously queued search.
  void startSearch();

  // Cancels all searches. Useful to do before destroying them all, since they
  // would cancel in parallel.
  void cancelSearches();

  /// Compares results based on their ranks
  struct SortByRank
  {
    bool operator () ( OneResult const & first, OneResult const & second )
    {
      if ( first.rank < second.rank )
        return true;
  
      if ( first.rank > second.rank )
        return false;
  
      // Do any sort of collation here in the future. For now we just put the
      // strings sorted lexicographically.
      return first.word < second.word;
    }
  };

  /// Compares results based on their ranks and lengths
  struct SortByRankAndLength
  {
    bool operator () ( OneResult const & first, OneResult const & second )
    {
      if ( first.rank < second.rank )
        return true;

      if ( first.rank > second.rank )
        return false;

      if ( first.word.size() < second.word.size() )
        return true;

      if ( first.word.size() > second.word.size() )
        return false;

      // Do any sort of collation here in the future. For now we just put the
      // strings sorted lexicographically.
      return first.word < second.word;
    }
  };
};

#endif

