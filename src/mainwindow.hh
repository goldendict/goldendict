/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __MAINWINDOW_HH_INCLUDED__
#define __MAINWINDOW_HH_INCLUDED__

#include <QMainWindow>
#include <QThread>
#include <QToolButton>
#include <QSystemTrayIcon>
#include "ui_mainwindow.h"
#include "folding.hh"
#include "config.hh"
#include "dictionary.hh"
#include "initializing.hh"
#include "article_netmgr.hh"
#include "instances.hh"
#include "article_maker.hh"
#include "scanpopup.hh"
#include "articleview.hh"
#include "wordfinder.hh"

using std::string;
using std::vector;

class LoadDictionaries: public QThread, Dictionary::Initializing
{
  Q_OBJECT

  vector< string > const & allFiles;
  vector< sptr< Dictionary::Class > > dictionaries;

public:

  LoadDictionaries( vector< string > const & allFiles );

  virtual void run();

  vector< sptr< Dictionary::Class > > const & getDictionaries() const
  { return dictionaries; }

signals:

  void indexingDictionarySignal( QString dictionaryName );

private:

  virtual void indexingDictionary( string const & dictionaryName ) throw();
};


class MainWindow: public QMainWindow
{
  Q_OBJECT

public:

  MainWindow();
  ~MainWindow();

private:

  QSystemTrayIcon * trayIcon;

  Ui::MainWindow ui;
  QToolBar * navToolbar;
  QAction * navBack, * navForward, * enableScanPopup;
  QMenu trayIconMenu;
  QToolButton addTab;
  Config::Class cfg;
  vector< sptr< Dictionary::Class > > dictionaries;
  vector< Instances::Group > groupInstances;
  ArticleMaker articleMaker;
  ArticleNetworkAccessManager articleNetMgr;

  WordFinder wordFinder;

  sptr< ScanPopup > scanPopup;

  ::Initializing * initializing;

  /// Creates, destroys or otherwise updates tray icon, according to the
  /// current configuration and situation.
  void updateTrayIcon();

  void closeEvent( QCloseEvent * );

  void makeDictionaries();
  void updateStatusLine();
  void updateGroupList();
  void makeScanPopup();

  /// Returns the reference to dictionaries stored in the currently active
  /// group, or to all dictionaries if there are no groups.
  vector< sptr< Dictionary::Class > > const & getActiveDicts();

private slots:

  // Executed in response to a user click on an 'add tab' tool button
  void addNewTab();
  // Executed in response to a user click on an 'close' button on a tab
  void tabCloseRequested( int );

  /// Triggered by the actions in the nav toolbar
  void backClicked();
  void forwardClicked();

  /// ArticleView's title has changed
  void titleChanged( ArticleView *, QString const & );
  /// ArticleView's icon has changed
  void iconChanged( ArticleView *, QIcon const & );

  void editSources();
  void editGroups();
  void editPreferences();
  void indexingDictionary( QString dictionaryName );

  void currentGroupChanged( QString const & );
  void translateInputChanged( QString const & );
  void translateInputFinished();
  void prefixMatchComplete( WordFinderResults );
  void wordListItemActivated( QListWidgetItem * );
  void wordListSelectionChanged();

  void openLinkInNewTab( QUrl const &, QUrl const & );
  void showDefinitionInNewTab( QString const & word, QString const & group );
  
  void showTranslationFor( QString const & );

  void trayIconActivated( QSystemTrayIcon::ActivationReason );

  void scanEnableToggled( bool );

  void showMainWindow();
  
  void visitHomepage();
  void visitForum();
  void showAbout();
};

#endif
