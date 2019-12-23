/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
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
  Config::Class const & cfg_;
  QStringList nameFilters;
  std::vector< sptr< Dictionary::Class > > dictionaries;
  std::string exceptionText;

protected:
  virtual void run();

signals:
  void showMessage(const QString &msg, int alignment = Qt::AlignCenter,
                   const QColor &color = Qt::darkMagenta);

public:
  virtual void indexingDictionary( std::string const & dictionaryName ) throw();

private:
  LoadDictionaries( Config::Class const & cfg );

  std::vector< sptr< Dictionary::Class > > const & getDictionaries() const
  { return dictionaries; }

  /// Empty string means to exception occurred
  std::string const & getExceptionText() const
  { return exceptionText; }
  void handlePath( Config::Path const & );

public:
  /// Loads all dictionaries mentioned in the configuration passed, into the
  /// supplied array. When necessary, a window would pop up describing the process.
  /// If showInitially is passed as true, the window will always popup.
  /// If doDeferredInit is true (default), doDeferredInit() is done on all
  /// dictionaries at the end.
  static void loadDictionaries( QWidget * parent, bool canHideParent,
                         Config::Class const & cfg,
                         std::vector< sptr< Dictionary::Class > > &,
                         QNetworkAccessManager & dictNetMgr,
                         bool doDeferredInit = true );

  /// Runs deferredInit() on all the given dictionaries. Useful when
  /// loadDictionaries() was previously called with doDeferredInit = false.
  static void doDeferredInit( std::vector< sptr< Dictionary::Class > > & );

};

#endif

