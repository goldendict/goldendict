/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __MAINWINDOW_HH_INCLUDED__
#define __MAINWINDOW_HH_INCLUDED__

#include <QMainWindow>
#include <QThread>
#include <QToolButton>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
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

class LoadDictionaries: public QThread, public Dictionary::Initializing
{
  Q_OBJECT

  Config::Paths const & paths;
  Config::SoundDirs const & soundDirs;
  Config::Hunspell const & hunspell;
  vector< sptr< Dictionary::Class > > dictionaries;
  string exceptionText;

public:

  LoadDictionaries( Config::Class const & cfg );

  virtual void run();

  vector< sptr< Dictionary::Class > > const & getDictionaries() const
  { return dictionaries; }

  /// Empty string means to exception occured
  string const & getExceptionText() const
  { return exceptionText; }

signals:

  void indexingDictionarySignal( QString dictionaryName );

public:

  virtual void indexingDictionary( string const & dictionaryName ) throw();

private:

  void handlePath( Config::Path const & );
};


class MainWindow: public QMainWindow
{
  Q_OBJECT

public:

  MainWindow( Config::Class & cfg );
  ~MainWindow();

private:

  QSystemTrayIcon * trayIcon;

  Ui::MainWindow ui;
  QAction focusTranslateLineAction, addTabAction, closeCurrentTabAction,
          switchToNextTabAction, switchToPrevTabAction;
  QToolBar * navToolbar;
  QAction * navBack, * navForward, * navPronounce, * enableScanPopup;
  QMenu trayIconMenu;
  QToolButton addTab;
  Config::Class & cfg;
  vector< sptr< Dictionary::Class > > dictionaries;
  vector< Instances::Group > groupInstances;
  ArticleMaker articleMaker;
  ArticleNetworkAccessManager articleNetMgr;
  QNetworkAccessManager dictNetMgr; // We give dictionaries a separate manager,
                                    // since their requests can be destroyed
                                    // in a separate thread

  WordFinder wordFinder;

  sptr< ScanPopup > scanPopup;

  ::Initializing * initializing;

  class HotkeyWrapper * hotkeyWrapper;

  /// Creates, destroys or otherwise updates tray icon, according to the
  /// current configuration and situation.
  void updateTrayIcon();

  void closeEvent( QCloseEvent * );

  void applyProxySettings();
  void makeDictionaries();
  void updateStatusLine();
  void updateGroupList();
  void makeScanPopup();

  void updateMatchResults( bool finished );

  void updatePronounceAvailability();

  virtual bool eventFilter( QObject *, QEvent * );

  /// Returns the reference to dictionaries stored in the currently active
  /// group, or to all dictionaries if there are no groups.
  vector< sptr< Dictionary::Class > > const & getActiveDicts();

  /// Brings the main window to front if it's not currently, or hides it
  /// otherwise. The hiding part is omitted if onlyShow is true.
  void toggleMainWindow( bool onlyShow = false );

private slots:

  // Executed in response to a user click on an 'add tab' tool button
  void addNewTab();
  // Executed in response to a user click on an 'close' button on a tab
  void tabCloseRequested( int );
  // Closes current tab.
  void closeCurrentTab();
  void switchToNextTab();
  void switchToPrevTab();

  /// Triggered by the actions in the nav toolbar
  void backClicked();
  void forwardClicked();

  /// ArticleView's title has changed
  void titleChanged( ArticleView *, QString const & );
  /// ArticleView's icon has changed
  void iconChanged( ArticleView *, QIcon const & );

  void pageLoaded();
  void tabSwitched( int );

  /// Pronounces the currently displayed word by playing its first audio
  /// reference, if it has any.
  void pronounce();

  void editSources();
  void editGroups();
  void editPreferences();
  void indexingDictionary( QString dictionaryName );

  void currentGroupChanged( QString const & );
  void translateInputChanged( QString const & );
  void translateInputFinished();

  /// Gives the keyboard focus to the translateLine and selects all the text
  /// it has.
  void focusTranslateLine();

  void prefixMatchUpdated();
  void prefixMatchFinished();

  void wordListItemActivated( QListWidgetItem * );
  void wordListSelectionChanged();

  /// Creates a new tab, which is to be populated then with some content.
  ArticleView * createNewTab( bool switchToIt,
                              QString const & name );

  void openLinkInNewTab( QUrl const &, QUrl const & );
  void showDefinitionInNewTab( QString const & word, unsigned group );

  void showTranslationFor( QString const & );

  void trayIconActivated( QSystemTrayIcon::ActivationReason );

  void scanEnableToggled( bool );

  void setAutostart( bool );

  void showMainWindow();

  void visitHomepage();
  void visitForum();
  void showAbout();

  void on_actionCloseToTray_activated();
};

#endif
