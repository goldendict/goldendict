/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __WORDFINDER_HH_INCLUDED__
#define __WORDFINDER_HH_INCLUDED__

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "dictionary.hh"


/// This struct represents results of a WordFinder match operation.
/// We need to separate this since it's needed to register is as a metatype
/// for the signal-slot connections to be able to pass it in queued mode.
struct WordFinderResults
{
  /// The initial request parameters. They are passed back so that it'd be
  /// possible to tell the results apart from any previously cancelled
  /// operations.
  QString requestStr;
  std::vector< sptr< Dictionary::Class > > const * requestDicts;

  /// The results themselves
  std::vector< QString > results;

  WordFinderResults()
  {}

  WordFinderResults( QString const & requestStr_,
                     std::vector< sptr< Dictionary::Class > > const * requestDicts_ ):
    requestStr( requestStr_ ), requestDicts( requestDicts_ )
  {}
};

/// This component takes care of finding words in dictionaries asyncronously,
/// in another thread. This means the GUI doesn't get blocked during the
/// sometimes lenghtly process of finding words.
class WordFinder: QThread
{
  Q_OBJECT

public:

  WordFinder( QObject * parent );
  ~WordFinder();

  /// Allows accessing the otherwise inaccessible QObject indirect base,
  /// which is QThread's base. This is needed to connect signals of the object.
  QObject * qobject()
  { return this; }

  /// Do the standard prefix-match search in the given list of dictionaries.
  /// Some dictionaries might only support exact matches -- for them, only
  /// the exact matches would be found. All search results are put into a single
  /// list containing the exact matches first, then the prefix ones. Duplicate
  /// matches from different dictionaries are merged together.
  /// If there already was a prefixMatch operation underway, it gets cancelled
  /// and the new one replaces it.
  /// The dictionaries and the containing vector get locked during the search
  /// using the DictLock object. Be sure you do that too in case you'd want
  /// to work with them during the search underway.
  void prefixMatch( QString const &,
                    std::vector< sptr< Dictionary::Class > > const * );

signals:

  /// This singal gets emitted from another thread and indicates that the
  /// previously requested prefixMatch() operation was finished. See the
  /// WordFinderResults structure description for further details.
  void prefixMatchComplete( WordFinderResults );

private:

  QMutex opMutex;
  QWaitCondition opCondition;

  enum Op
  {
    NoOp,
    QuitOp,
    DoPrefixMatch
  } op;

  QString prefixMatchString;
  std::vector< sptr< Dictionary::Class > > const * prefixMatchDicts;

  virtual void run();
};

#endif

