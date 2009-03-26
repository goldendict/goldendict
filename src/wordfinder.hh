/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
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
#include "dictionary.hh"

/// This component takes care of finding words. The search is asyncronous.
/// This means the GUI doesn't get blocked during the sometimes lenghtly
/// process of finding words.
class WordFinder: public QObject
{
  Q_OBJECT

  std::vector< QString > searchResults;
  QString searchErrorString;
  std::list< sptr< Dictionary::WordSearchRequest > > queuedRequests,
                                                     finishedRequests;
  bool searchInProgress;

  QTimer updateResultsTimer;

  // Saved search params
  bool searchQueued;
  QString inputWord;
  std::vector< sptr< Dictionary::Class > > const * inputDicts;

  // Maps lowercased string to the original one. This catches all duplicates
  // without case sensitivity
  std::map< std::wstring, std::wstring > results;
    
public:

  WordFinder( QObject * parent );
  ~WordFinder();

  /// Do the standard prefix-match search in the given list of dictionaries.
  /// Some dictionaries might only support exact matches -- for them, only
  /// the exact matches would be found. All search results are put into a single
  /// list containing the exact matches first, then the prefix ones. Duplicate
  /// matches from different dictionaries are merged together.
  /// If there already was a prefixMatch operation underway, it gets cancelled
  /// and the new one replaces it.
  void prefixMatch( QString const &,
                    std::vector< sptr< Dictionary::Class > > const & );
  
  /// Returns the vector containing search results from the last prefixMatch()
  /// operation. If it didn't finish yet, the result is not final and may
  /// be changing over time.
  std::vector< QString > const & getPrefixMatchResults() const
  { return searchResults; }

  /// Returns a human-readable error string for the last finished request. Empty
  /// string means it finished without any error.
  QString const & getErrorString()
  { return searchErrorString; }

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

  /// Idicates that the search has finished.
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
};

#endif

