/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __MAINWINDOW_HH_INCLUDED__
#define __MAINWINDOW_HH_INCLUDED__

#include <QMainWindow>
#include <QThread>
#include <QToolButton>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include "ui_mainwindow.h"
#include "folding.hh"
#include "config.hh"
#include "dictionary.hh"
#include "article_netmgr.hh"
#include "instances.hh"
#include "article_maker.hh"
#include "scanpopup.hh"
#include "articleview.hh"
#include "wordfinder.hh"
#include "dictionarybar.hh"
#include "history.hh"
#include "hotkeywrapper.hh"
#include "mainstatusbar.hh"
#include "mruqmenu.hh"
#include "translatebox.hh"
#include "wordlist.hh"
#include "dictheadwords.hh"
#include "fulltextsearch.hh"
#include "helpwindow.hh"

#ifdef HAVE_X11
#include <fixx11h.h>
#endif

using std::string;
using std::vector;

class ExpandableToolBar : public QToolBar
{
  Q_OBJECT

public:
  explicit ExpandableToolBar(QString const & title, QWidget * parent = 0)
    : QToolBar(title, parent) {}
  virtual QSize sizeHint() const
  {
    if ( !isFloating() && parentWidget() )
    {
      return QSize( parentWidget()->width(), QToolBar::sizeHint().height() );
    }
    else
    {
      return QToolBar::sizeHint();
    }
  }
};

class MainWindow: public QMainWindow, public DataCommitter
{
  Q_OBJECT

public:

  MainWindow( Config::Class & cfg );
  ~MainWindow();

  virtual void commitData( QSessionManager & );

  void showGDHelpForID( QString const & id );
  void closeGDHelp();
  QString getTranslateLineText() const
  { return translateLine->text(); }

public slots:

  void messageFromAnotherInstanceReceived( QString const & );
  void showStatusBarMessage ( QString const &, int, QPixmap const & );
  void wordReceived( QString const & );
  void headwordReceived( QString const &, QString const & );
  void setExpandMode( bool expand );

private:
  void addGlobalAction( QAction * action, const char * slot );
  void addGlobalActionsToDialog( QDialog * dialog );

  void commitData();
  bool commitDataCompleted;

  QSystemTrayIcon * trayIcon;

  Ui::MainWindow ui;

  /// This widget is used as a title bar for the searchPane dock, and
  /// incorporates the next three objects inside
  QWidget searchPaneTitleBar;
  QHBoxLayout searchPaneTitleBarLayout;
  QLabel groupLabel;
  GroupComboBox * groupList, * groupListInToolbar, * groupListInDock;

  // Needed to be able to show/hide the translate box in the toolbar, since hiding
  // the list expilictily doesn't work, see docs for QToolBar::addWidget().
  QAction * translateBoxToolBarAction;

  QWidget dictsPaneTitleBar;
  QHBoxLayout dictsPaneTitleBarLayout;
  QLabel foundInDictsLabel;

  TranslateBox * translateBox;

  /// Fonts saved before words zooming is in effect, so it could be reset back.
  QFont wordListDefaultFont, translateLineDefaultFont;

  QAction escAction, focusTranslateLineAction, addTabAction, closeCurrentTabAction,
          closeAllTabAction, closeRestTabAction,
          switchToNextTabAction, switchToPrevTabAction,
          showDictBarNamesAction, useSmallIconsInToolbarsAction, toggleMenuBarAction,
          switchExpandModeAction, focusHeadwordsDlgAction, focusArticleViewAction;
  QToolBar * navToolbar;
  MainStatusBar * mainStatusBar;
  QAction * navBack, * navForward, * navPronounce, * enableScanPopup;
  QAction * beforeScanPopupSeparator, * afterScanPopupSeparator, * beforeOptionsSeparator;
  QAction * zoomIn, * zoomOut, * zoomBase;
  QAction * wordsZoomIn, * wordsZoomOut, * wordsZoomBase;
  QMenu trayIconMenu;
  QMenu * tabMenu;
  QAction * menuButtonAction;
  QToolButton * menuButton;
  MRUQMenu *tabListMenu;
  //List that contains indexes of tabs arranged in a most-recently-used order
  QList<QWidget*> mruList;
  QToolButton addTab, *tabListButton;
  Config::Class & cfg;
  Config::Events configEvents;
  History history;
  DictionaryBar dictionaryBar;
  vector< sptr< Dictionary::Class > > dictionaries;
  /// Here we store unmuted dictionaries when the dictionary bar is active
  vector< sptr< Dictionary::Class > > dictionariesUnmuted;
  Instances::Groups groupInstances;
  ArticleMaker articleMaker;
  ArticleNetworkAccessManager articleNetMgr;
  QNetworkAccessManager dictNetMgr; // We give dictionaries a separate manager,
                                    // since their requests can be destroyed
                                    // in a separate thread

  WordList * wordList;
  QLineEdit * translateLine;

  WordFinder wordFinder;

  sptr< ScanPopup > scanPopup;

  sptr< HotkeyWrapper > hotkeyWrapper;

  QTimer newReleaseCheckTimer; // Countdown to a check for the new program
                               // release, if enabled
  QNetworkReply *latestReleaseReply;

  sptr< QPrinter > printer; // The printer we use for all printing operations

  bool wordListSelChanged;

  bool wasMaximized; // Window state before minimization

  bool blockUpdateWindowTitle;

  QPrinter & getPrinter(); // Creates a printer if it's not there and returns it

  DictHeadwords * headwordsDlg;

  FTS::FtsIndexing ftsIndexing;

  FTS::FullTextSearchDialog * ftsDlg;

  Help::HelpWindow * helpWindow;

  /// Applies the qt's stylesheet, given the style's name.
  void applyQtStyleSheet( QString const & displayStyle, QString const & addonStyle );

  /// Creates, destroys or otherwise updates tray icon, according to the
  /// current configuration and situation.
  void updateTrayIcon();

  void wheelEvent( QWheelEvent * );
  void closeEvent( QCloseEvent * );

  void applyProxySettings();
  void applyWebSettings();
  void makeDictionaries();
  void updateStatusLine();
  void updateGroupList();
  void updateDictionaryBar();
  void makeScanPopup();

  void updatePronounceAvailability();

  void updateFoundInDictsList();

  void updateBackForwardButtons();

  void updateWindowTitle();

  /// Updates word search request and active article view in response to
  /// muting or unmuting dictionaries, or showing/hiding dictionary bar.
  void applyMutedDictionariesState();

  virtual bool eventFilter( QObject *, QEvent * );

  /// Returns the reference to dictionaries stored in the currently active
  /// group, or to all dictionaries if there are no groups.
  vector< sptr< Dictionary::Class > > const & getActiveDicts();

  /// Brings the main window to front if it's not currently, or hides it
  /// otherwise. The hiding part is omitted if onlyShow is true.
#ifdef HAVE_X11
  void toggleMainWindow( bool onlyShow = false, bool byIconClick = false );
#else
  void toggleMainWindow( bool onlyShow = false );
#endif

  /// Creates hotkeyWrapper and hooks the currently set keys for it
  void installHotKeys();

  void applyZoomFactor();

  void mousePressEvent ( QMouseEvent * event );

  void updateCurrentGroupProperty();

  /// Handles backward and forward mouse buttons and
  /// returns true if the event is handled.
  bool handleBackForwardMouseButtons(QMouseEvent *ev);

  ArticleView * getCurrentArticleView();
  void ctrlTabPressed();

  void fillWordListFromHistory();

  void showDictionaryHeadwords( QWidget * owner, Dictionary::Class * dict );

private slots:

  void hotKeyActivated( int );

  /// If new release checks are on, santizies the next check time and starts
  /// the timer. Does nothing otherwise.
  void prepareNewReleaseChecks();

private slots:

  /// Does the new release check.
  void checkForNewRelease();

  /// Signalled when the lastestReleaseReply is finished()
  void latestReleaseReplyReady();

  /// Receive click on "Found in:" pane
  void foundDictsPaneClicked( QListWidgetItem * item );

  /// Receive right click on "Found in:" pane
  void foundDictsContextMenuRequested( const QPoint & pos );

  void showDictionaryInfo( QString const & id );

  void showDictionaryHeadwords( QString const & id );

  void openDictionaryFolder( QString const & id );

  void editDictionary ( Dictionary::Class * dict );

  void showFTSIndexingName( QString const & name );

private slots:

  // Executed in response to a user click on an 'add tab' tool button
  void addNewTab();
  // Executed in response to a user click on an 'close' button on a tab
  void tabCloseRequested( int );
  // Closes current tab.
  void closeCurrentTab();
  void closeAllTabs();
  void closeRestTabs();
  void switchToNextTab();
  void switchToPrevTab();
  void ctrlReleased();

  // Switch optional parts expand mode for current tab
  void switchExpandOptionalPartsMode();

  // Handling of active tab list
  void createTabList();
  void fillWindowsMenu();
  void switchToWindow(QAction *act);

  /// Triggered by the actions in the nav toolbar
  void backClicked();
  void forwardClicked();

  /// ArticleView's title has changed
  void titleChanged( ArticleView *, QString const & );
  /// ArticleView's icon has changed
  void iconChanged( ArticleView *, QIcon const & );

  void pageLoaded( ArticleView * );
  void tabSwitched( int );
  void tabMenuRequested(QPoint pos);

  void dictionaryBarToggled( bool checked );

  /// Pronounces the currently displayed word by playing its first audio
  /// reference, if it has any.
  /// If view is 0, the operation is done for the currently open tab.
  void pronounce( ArticleView * view = 0 );

  void zoomin();
  void zoomout();
  void unzoom();

  void doWordsZoomIn();
  void doWordsZoomOut();
  void doWordsZoomBase();

  void applyWordsZoomLevel();

  /// If editDictionaryGroup is specified, the dialog positions on that group
  /// initially.
  void editDictionaries( unsigned editDictionaryGroup = Instances::Group::NoGroupId );
  /// Edits current group when triggered from the dictionary bar.
  void editCurrentGroup();
  void editPreferences();

  void currentGroupChanged( QString const & );
  void translateInputChanged( QString const & );
  void translateInputFinished( bool checkModifiers = true, QString const & dictID = QString() );

  /// Closes any opened search in the article view, and focuses the translateLine/close main window to tray.
  void handleEsc();

  /// Gives the keyboard focus to the translateLine and selects all the text
  /// it has.
  void focusTranslateLine();

  void wordListItemActivated( QListWidgetItem * );
  void wordListSelectionChanged();

  void dictsListItemActivated( QListWidgetItem * );
  void dictsListSelectionChanged();

  void jumpToDictionary( QListWidgetItem *, bool force = false );

  void showDictsPane( );
  void dictsPaneVisibilityChanged ( bool );

  /// Creates a new tab, which is to be populated then with some content.
  ArticleView * createNewTab( bool switchToIt,
                              QString const & name );

  void openLinkInNewTab( QUrl const &, QUrl const &, QString const &,
                         ArticleView::Contexts const & contexts );
  void showDefinitionInNewTab( QString const & word, unsigned group,
                               QString const & fromArticle,
                               ArticleView::Contexts const & contexts );
  void typingEvent( QString const & );

  void activeArticleChanged( ArticleView const *, QString const & id );

  void mutedDictionariesChanged();

  void showTranslationFor( QString const &, unsigned inGroup = 0,
                           QString const & dictID = QString() );

  void showTranslationFor( QString const &, QStringList const & dictIDs,
                           QRegExp const & searchRegExp );

  void showHistoryItem( QString const & );

  void trayIconActivated( QSystemTrayIcon::ActivationReason );

  void scanEnableToggled( bool );

  void setAutostart( bool );

  void showMainWindow();

  void visitHomepage();
  void visitForum();
  void openConfigFolder();
  void showAbout();

  void showDictBarNamesTriggered();
  void useSmallIconsInToolbarsTriggered();
  void toggleMenuBarTriggered( bool announce = true );

  void on_clearHistory_triggered();

  void on_newTab_triggered();

  void on_actionCloseToTray_triggered();

  void on_pageSetup_triggered();
  void on_printPreview_triggered();
  void on_print_triggered();
  void printPreviewPaintRequested( QPrinter * );

  void on_saveArticle_triggered();

  void on_rescanFiles_triggered();

  void on_showHideHistory_triggered();
  void on_exportHistory_triggered();
  void on_importHistory_triggered();
  void on_alwaysOnTop_triggered( bool checked );
  void focusWordList();

  void updateSearchPaneAndBar( bool searchInDock );

  void updateHistoryMenu();

  /// Add word to history
  void addWordToHistory( const QString & word );
  /// Add word to history even if history is disabled in options
  void forceAddWordToHistory( const QString & word);

  void sendWordToInputLine( QString const & word );

  void storeResourceSavePath( QString const & );

  void closeHeadwordsDialog();

  void focusHeadwordsDialog();

  void focusArticleView();

  void proxyAuthentication( const QNetworkProxy & proxy, QAuthenticator * authenticator );

  void showFullTextSearchDialog();
  void closeFullTextSearchDialog();

  void showGDHelp();
  void hideGDHelp();

signals:
  /// Set optional parts expand mode for all tabs
  void setExpandOptionalParts( bool expand );

  /// Retranslate Ctrl(Shift) + Click on dictionary pane to dictionary toolbar
  void clickOnDictPane( QString const & id );

#ifdef Q_OS_WIN32
  /// For receiving message from scan libraries
protected:
  unsigned gdAskMessage;
public:
  bool handleGDMessage( MSG * message, long * result );

private slots:
  /// Return true while scanning GoldenDict window
  bool isGoldenDictWindow( HWND hwnd );
#endif
};

class ArticleSaveProgressDialog : public QProgressDialog
{
Q_OBJECT

public:
  explicit ArticleSaveProgressDialog( QWidget * parent = 0,  Qt::WindowFlags f = 0 ):
    QProgressDialog( parent, f )
  {
    setAutoReset( false );
    setAutoClose( false );
  }

public slots:
  void perform()
  {
    int progress = value() + 1;
    if ( progress == maximum() )
    {
      emit close();
      deleteLater();
    }
    setValue( progress );
  }
};

#endif
