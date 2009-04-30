/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __LOADDICTIONARIES_HH_INCLUDED__
#define __LOADDICTIONARIES_HH_INCLUDED__

#include "initializing.hh"
#include "config.hh"
#include "dictionary.hh"

#include <QThread>
#include <QNetworkAccessManager>

/// Use loadDictionaries() function below -- this is a helper thread class
class LoadDictionaries: public QThread, public Dictionary::Initializing
{
  Q_OBJECT

  Config::Paths const & paths;
  Config::SoundDirs const & soundDirs;
  Config::Hunspell const & hunspell;
  std::vector< sptr< Dictionary::Class > > dictionaries;
  std::string exceptionText;

public:

  LoadDictionaries( Config::Class const & cfg );

  virtual void run();

  std::vector< sptr< Dictionary::Class > > const & getDictionaries() const
  { return dictionaries; }

  /// Empty string means to exception occured
  std::string const & getExceptionText() const
  { return exceptionText; }

signals:

  void indexingDictionarySignal( QString const & dictionaryName );

public:

  virtual void indexingDictionary( std::string const & dictionaryName ) throw();

private:

  void handlePath( Config::Path const & );
};

/// Loads all dictionaries mentioned in the configuration passed, into the
/// supplied array. When necessary, a window would pop up describing the process.
/// If showInitially is passed as true, the window will always popup.
void loadDictionaries( QWidget * parent, bool showInitially,
                       Config::Class const & cfg,
                       std::vector< sptr< Dictionary::Class > > &,
                       QNetworkAccessManager & dictNetMgr );

#endif

