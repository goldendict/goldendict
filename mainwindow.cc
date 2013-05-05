/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mainwindow.hh"
#include "editdictionaries.hh"
#include "loaddictionaries.hh"
#include "preferences.hh"
#include "about.hh"
#include "mruqmenu.hh"
#include <limits.h>
#include <QDir>
#include <QMessageBox>
#include <QIcon>
#include <QList>
#include <QToolBar>
#include <QCloseEvent>
#include <QDesktopServices>
#include <set>
#include <map>
#include "dprintf.hh"
#include <QDebug>
#include <QTextStream>
#include "dictinfo.hh"
#include "fsencoding.hh"
#include <QProcess>
#include "historypanewidget.hh"

#ifdef Q_OS_MAC
#include "lionsupport.h"
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
#include "mouseover_win32/GDDataTranfer.h"
#include "wstring.hh"
#include "wstring_qt.hh"
#endif

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#endif

using std::set;
using std::wstring;
using std::map;
using std::pair;

MainWindow::MainWindow( Config::Class & cfg_ ):
  commitDataCompleted( false ),
  trayIcon( 0 ),
  groupLabel( &searchPaneTitleBar ),
  foundInDictsLabel( &dictsPaneTitleBar ),
  escAction( this ),
  focusTranslateLineAction( this ),
  addTabAction( this ),
  closeCurrentTabAction( this ),
  closeAllTabAction( this ),
  closeRestTabAction( this ),
  switchToNextTabAction( this ),
  switchToPrevTabAction( this ),
  showDictBarNamesAction( tr( "Show Names in Dictionary &Bar" ), this ),
  useSmallIconsInToolbarsAction( tr( "Show Small Icons in &Toolbars" ), this ),
  toggleMenuBarAction( tr( "&Menubar" ), this ),
  switchExpandModeAction( this ),
  trayIconMenu( this ),
  addTab( this ),
  cfg( cfg_ ),
  history( History::Load(), cfg_.preferences.maxStringsInHistory, cfg_.maxHeadwordSize ),
  dictionaryBar( this, configEvents, cfg.editDictionaryCommandLine ),
  articleMaker( dictionaries, groupInstances, cfg.preferences.displayStyle,
                cfg.preferences.addonStyle ),
  articleNetMgr( this, dictionaries, articleMaker,
                 cfg.preferences.disallowContentFromOtherSites ),
  dictNetMgr( this ),
  wordFinder( this ),
  newReleaseCheckTimer( this ),
  wordListSelChanged( false )
, wasMaximized( false )
, blockUpdateWindowTitle( false )
#ifdef Q_OS_WIN32
, gdAskMessage( 0xFFFFFFFF )
#endif
{
  applyQtStyleSheet( cfg.preferences.displayStyle, cfg.preferences.addonStyle );

  ui.setupUi( this );

  // use our own, cutsom statusbar
  setStatusBar(0);
  mainStatusBar = new MainStatusBar( this );

  // Make the toolbar
  navToolbar = addToolBar( tr( "&Navigation" ) );
  navToolbar->setObjectName( "navToolbar" );

  navBack = navToolbar->addAction( QIcon( ":/icons/previous.png" ), tr( "Back" ) );
  navToolbar->widgetForAction( navBack )->setObjectName( "backButton" );
  navForward = navToolbar->addAction( QIcon( ":/icons/next.png" ), tr( "Forward" ) );
  navToolbar->widgetForAction( navForward )->setObjectName( "forwardButton" );

  QWidget * translateBoxWidget = new QWidget( this );
  QHBoxLayout * translateBoxLayout = new QHBoxLayout( translateBoxWidget );
  translateBoxWidget->setLayout( translateBoxLayout );
  translateBoxLayout->setContentsMargins( 0, 0, 0, 0 );
  translateBoxLayout->setSpacing( 0 );

  // translate box
  groupListInToolbar = new GroupComboBox( navToolbar );
  groupListInToolbar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
  groupListInToolbar->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  translateBoxLayout->addWidget( groupListInToolbar );

  translateBox = new TranslateBox( navToolbar );
  translateBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  translateBoxLayout->addWidget( translateBox );
  translateBoxToolBarAction = navToolbar->addWidget( translateBoxWidget );

  // scan popup
  beforeScanPopupSeparator = navToolbar->addSeparator();
  beforeScanPopupSeparator->setVisible( cfg.preferences.enableScanPopup );
  navToolbar->widgetForAction( beforeScanPopupSeparator )->setObjectName( "beforeScanPopupSeparator" );

  enableScanPopup = navToolbar->addAction( QIcon( ":/icons/wizard.png" ), tr( "Scan Popup" ) );
  enableScanPopup->setCheckable( true );
  enableScanPopup->setVisible( cfg.preferences.enableScanPopup );
  navToolbar->widgetForAction( enableScanPopup )->setObjectName( "scanPopupButton" );
  if ( cfg.preferences.enableScanPopup && cfg.preferences.startWithScanPopupOn )
    enableScanPopup->setChecked( true );

  connect( enableScanPopup, SIGNAL( toggled( bool ) ),
           this, SLOT( scanEnableToggled( bool ) ) );

  afterScanPopupSeparator = navToolbar->addSeparator();
  afterScanPopupSeparator->setVisible( cfg.preferences.enableScanPopup );
  navToolbar->widgetForAction( afterScanPopupSeparator )->setObjectName( "afterScanPopupSeparator" );

  // sound
  navPronounce = navToolbar->addAction( QIcon( ":/icons/playsound.png" ), tr( "Pronounce Word (Alt+S)" ) );
  navPronounce->setShortcut( QKeySequence( "Alt+S" ) );
  navPronounce->setEnabled( false );
  navToolbar->widgetForAction( navPronounce )->setObjectName( "soundButton" );

  connect( navPronounce, SIGNAL( triggered() ),
           this, SLOT( pronounce() ) );

  // zooming
  // named separator (to be able to hide it via CSS)
  navToolbar->widgetForAction( navToolbar->addSeparator() )->setObjectName( "separatorBeforeZoom" );

  zoomIn = navToolbar->addAction( QIcon( ":/icons/icon32_zoomin.png" ), tr( "Zoom In" ) );
  zoomIn->setShortcuts( QList< QKeySequence >() <<
                       QKeySequence::ZoomIn <<
                       QKeySequence( "Ctrl+=" ) );
  navToolbar->widgetForAction( zoomIn )->setObjectName( "zoomInButton" );

  zoomOut = navToolbar->addAction( QIcon( ":/icons/icon32_zoomout.png" ), tr( "Zoom Out" ) );
  zoomOut->setShortcut( QKeySequence::ZoomOut );
  navToolbar->widgetForAction( zoomOut )->setObjectName( "zoomOutButton" );

  zoomBase = navToolbar->addAction( QIcon( ":/icons/icon32_zoombase.png" ), tr( "Normal Size" ) );
  zoomBase->setShortcut( QKeySequence( "Ctrl+0" ) );
  navToolbar->widgetForAction( zoomBase )->setObjectName( "zoomBaseButton" );

  // named separator (to be able to hide it via CSS)
  navToolbar->widgetForAction( navToolbar->addSeparator() )->setObjectName( "separatorBeforeSave" );

  navToolbar->addAction( ui.saveArticle );
  navToolbar->widgetForAction( ui.saveArticle )->setObjectName( "saveArticleButton" );

  navToolbar->addAction( ui.print );
  navToolbar->widgetForAction( ui.print )->setObjectName( "printButton" );

  beforeOptionsSeparator = navToolbar->addSeparator();
  navToolbar->widgetForAction( beforeOptionsSeparator )->setObjectName( "beforeOptionsSeparator" );
  beforeOptionsSeparator->setVisible( cfg.preferences.hideMenubar);

  QMenu * buttonMenu = new QMenu( this );
  buttonMenu->addAction( ui.dictionaries );
  buttonMenu->addAction( ui.preferences );
  buttonMenu->addSeparator();
  buttonMenu->addMenu( ui.menuHistory );
  buttonMenu->addSeparator();
  buttonMenu->addMenu( ui.menuFile );
  buttonMenu->addMenu( ui.menuView );
  buttonMenu->addMenu( ui.menu_Help );

  menuButton = new QToolButton( navToolbar );
  menuButton->setPopupMode( QToolButton::InstantPopup );
  menuButton->setMenu( buttonMenu );
  menuButton->setIcon( QIcon (":/icons/menu_button.png") );
  menuButton->addAction( ui.menuOptions );
  menuButton->setToolTip( tr( "Menu Button" ) );
  menuButton->setObjectName( "menuButton" );
  menuButton->setFocusPolicy( Qt::NoFocus );

  menuButtonAction = navToolbar->addWidget(menuButton);
  menuButtonAction->setVisible( cfg.preferences.hideMenubar );

  // Make the search pane's titlebar
  groupLabel.setText( tr( "Look up in:" ) );
  groupListInDock = new GroupComboBox( &searchPaneTitleBar );

  searchPaneTitleBarLayout.setContentsMargins( 8, 5, 8, 4 );
  searchPaneTitleBarLayout.addWidget( &groupLabel );
  searchPaneTitleBarLayout.addWidget( groupListInDock );
  searchPaneTitleBarLayout.addStretch();

  searchPaneTitleBar.setLayout( &searchPaneTitleBarLayout );

  ui.searchPane->setTitleBarWidget( &searchPaneTitleBar );
  connect( ui.searchPane->toggleViewAction(), SIGNAL( triggered( bool ) ),
           this, SLOT( updateSearchPaneAndBar( bool ) ) );

  if ( cfg.preferences.searchInDock )
  {
    groupList = groupListInDock;
    translateLine = ui.translateLine;
    wordList = ui.wordList;
  }
  else
  {
    groupList = groupListInToolbar;
    translateLine = translateBox->translateLine();
    wordList = translateBox->wordList();
  }
  wordList->attachFinder( &wordFinder );

  // for the old UI:
  ui.wordList->setTranslateLine( ui.translateLine );

  groupList->setFocusPolicy(Qt::ClickFocus);
  wordList->setFocusPolicy(Qt::ClickFocus);

  wordListDefaultFont = wordList->font();
  translateLineDefaultFont = translateLine->font();

  // Make the dictionaries pane's titlebar
  foundInDictsLabel.setText( tr( "Found in Dictionaries:" ) );
  dictsPaneTitleBarLayout.addWidget( &foundInDictsLabel );
  dictsPaneTitleBarLayout.setContentsMargins(5, 5, 5, 5);
  dictsPaneTitleBar.setLayout( &dictsPaneTitleBarLayout );
  dictsPaneTitleBar.setObjectName("dictsPaneTitleBar");
  ui.dictsPane->setTitleBarWidget( &dictsPaneTitleBar );
  ui.dictsList->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( ui.dictsPane, SIGNAL( visibilityChanged( bool ) ),
           this, SLOT( dictsPaneVisibilityChanged ( bool ) ) );

  connect( ui.dictsList, SIGNAL( itemClicked( QListWidgetItem * ) ),
           this, SLOT( foundDictsPaneClicked( QListWidgetItem * ) ) );

  connect( ui.dictsList, SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this, SLOT( foundDictsContextMenuRequested( const QPoint & ) ) );

  connect( zoomIn, SIGNAL( triggered() ),
           this, SLOT( zoomin() ) );
  connect( zoomOut, SIGNAL( triggered() ),
           this, SLOT( zoomout() ) );
  connect( zoomBase, SIGNAL( triggered() ),
           this, SLOT( unzoom() ) );

  ui.menuZoom->addAction( zoomIn );
  ui.menuZoom->addAction( zoomOut );
  ui.menuZoom->addAction( zoomBase );

  ui.menuZoom->addSeparator();

  wordsZoomIn = ui.menuZoom->addAction( QIcon( ":/icons/icon32_zoomin.png" ), tr( "Words Zoom In" ) );
  wordsZoomIn->setShortcuts( QList< QKeySequence >() <<
                            QKeySequence( "Alt++" ) <<
                            QKeySequence( "Alt+=" ) );
  wordsZoomOut = ui.menuZoom->addAction( QIcon( ":/icons/icon32_zoomout.png" ), tr( "Words Zoom Out" ) );
  wordsZoomOut->setShortcut( QKeySequence( "Alt+-" ) );
  wordsZoomBase = ui.menuZoom->addAction( QIcon( ":/icons/icon32_zoombase.png" ), tr( "Words Normal Size" ) );
  wordsZoomBase->setShortcut( QKeySequence( "Alt+0" ) );

  connect( wordsZoomIn, SIGNAL(triggered()), this, SLOT(doWordsZoomIn()) );
  connect( wordsZoomOut, SIGNAL(triggered()), this, SLOT(doWordsZoomOut()) );
  connect( wordsZoomBase, SIGNAL(triggered()), this, SLOT(doWordsZoomBase()) );

  // tray icon
  connect( trayIconMenu.addAction( tr( "Show &Main Window" ) ), SIGNAL( triggered() ),
           this, SLOT( showMainWindow() ) );
  trayIconMenu.addAction( enableScanPopup );
  trayIconMenu.addSeparator();
  connect( trayIconMenu.addAction( tr( "&Quit" ) ), SIGNAL( triggered() ),
           qApp, SLOT( quit() ) );

  escAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  escAction.setShortcut( QKeySequence( "Esc" ) );
  connect( &escAction, SIGNAL( triggered() ),
           this, SLOT( handleEsc() ) );

  focusTranslateLineAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  focusTranslateLineAction.setShortcuts( QList< QKeySequence >() <<
                                         QKeySequence( "Alt+D" ) <<
                                         QKeySequence( "Ctrl+L" ) );

  connect( &focusTranslateLineAction, SIGNAL( triggered() ),
           this, SLOT( focusTranslateLine() ) );

  ui.centralWidget->addAction( &escAction );
  ui.dictsPane->addAction( &escAction );
  ui.searchPaneWidget->addAction( &escAction );
  ui.historyPane->addAction( &escAction );
  groupList->addAction( &escAction );
  translateBox->addAction( &escAction );

  ui.centralWidget->addAction( &focusTranslateLineAction );
  ui.dictsPane->addAction( &focusTranslateLineAction );
  ui.searchPaneWidget->addAction( &focusTranslateLineAction );
  ui.historyPane->addAction( &focusTranslateLineAction );
  groupList->addAction( &focusTranslateLineAction );

  addTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  addTabAction.setShortcut( QKeySequence( "Ctrl+T" ) );

  // Tab management
  tabListMenu = new MRUQMenu(tr("Opened tabs"), ui.tabWidget);

  connect (tabListMenu, SIGNAL(ctrlReleased()), this, SLOT(ctrlReleased()));

  connect( &addTabAction, SIGNAL( triggered() ),
           this, SLOT( addNewTab() ) );

  addAction( &addTabAction );

  closeCurrentTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  closeCurrentTabAction.setShortcut( QKeySequence( "Ctrl+W" ) );
  closeCurrentTabAction.setText( tr("Close current tab") );
  closeCurrentTabAction.setIcon( QIcon(":/icons/closetab.png") );

  connect( &closeCurrentTabAction, SIGNAL( triggered() ),
           this, SLOT( closeCurrentTab() ) );

  addAction( &closeCurrentTabAction );

  closeAllTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  closeAllTabAction.setShortcut( QKeySequence( "Ctrl+Shift+W" ) );
  closeAllTabAction.setText( tr("Close all tabs") );

  connect( &closeAllTabAction, SIGNAL( triggered() ),
           this, SLOT( closeAllTabs() ) );

  addAction( &closeAllTabAction );

  closeRestTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  closeRestTabAction.setText( tr("Close all tabs except current") );

  connect( &closeRestTabAction, SIGNAL( triggered() ),
           this, SLOT( closeRestTabs() ) );

  addAction( &closeRestTabAction );

  switchToNextTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  switchToNextTabAction.setShortcut( QKeySequence( "Ctrl+PgDown" ) );

  connect( &switchToNextTabAction, SIGNAL( triggered() ),
           this, SLOT( switchToNextTab() ) );

  addAction( &switchToNextTabAction );

  switchToPrevTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  switchToPrevTabAction.setShortcut( QKeySequence( "Ctrl+PgUp" ) );

  connect( &switchToPrevTabAction, SIGNAL( triggered() ),
           this, SLOT( switchToPrevTab() ) );

  addAction( &switchToPrevTabAction );

  switchExpandModeAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  switchExpandModeAction.setShortcuts( QList< QKeySequence >() <<
                                       QKeySequence( Qt::CTRL + Qt::Key_8 ) <<
                                       QKeySequence( Qt::CTRL + Qt::Key_Asterisk ) <<
                                       QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_8 ) );

  connect( &switchExpandModeAction, SIGNAL( triggered() ),
           this, SLOT(switchExpandOptionalPartsMode() ) );

  addAction( &switchExpandModeAction );

  tabMenu = new QMenu(this);
  tabMenu->addAction( &closeCurrentTabAction );
  tabMenu->addAction( &closeRestTabAction );
  tabMenu->addSeparator();
  tabMenu->addAction( &closeAllTabAction );

  // Dictionary bar names

  showDictBarNamesAction.setCheckable( true );
  showDictBarNamesAction.setChecked( cfg.showingDictBarNames );

  connect( &showDictBarNamesAction, SIGNAL( triggered() ),
           this, SLOT( showDictBarNamesTriggered() ) );

  // Use small icons in toolbars

  useSmallIconsInToolbarsAction.setCheckable( true );
  useSmallIconsInToolbarsAction.setChecked( cfg.usingSmallIconsInToolbars );

  connect( &useSmallIconsInToolbarsAction, SIGNAL( triggered() ),
           this, SLOT( useSmallIconsInToolbarsTriggered() ) );

  // Toggle Menubar
  toggleMenuBarAction.setCheckable( true );
  toggleMenuBarAction.setChecked( !cfg.preferences.hideMenubar );
  toggleMenuBarAction.setShortcut( QKeySequence( "Ctrl+M" ) );

  connect( &toggleMenuBarAction, SIGNAL( triggered() ),
           this, SLOT( toggleMenuBarTriggered() ) );

  // Populate 'View' menu

  ui.menuView->addAction( &toggleMenuBarAction );
  ui.menuView->addSeparator();
  ui.menuView->addAction( ui.searchPane->toggleViewAction() );
  ui.searchPane->toggleViewAction()->setShortcut( QKeySequence( "Ctrl+S" ) );
  ui.menuView->addAction( ui.dictsPane->toggleViewAction() );
  ui.dictsPane->toggleViewAction()->setShortcut( QKeySequence( "Ctrl+R" ) );
  ui.menuView->addAction( ui.historyPane->toggleViewAction() );
  ui.historyPane->toggleViewAction()->setShortcut( QKeySequence( "Ctrl+H" ) );
  ui.menuView->addSeparator();
  ui.menuView->addAction( dictionaryBar.toggleViewAction() );
  ui.menuView->addAction( navToolbar->toggleViewAction() );
  ui.menuView->addSeparator();
  ui.menuView->addAction( &showDictBarNamesAction );
  ui.menuView->addAction( &useSmallIconsInToolbarsAction );
  ui.menuView->addSeparator();
  ui.alwaysOnTop->setChecked( cfg.preferences.alwaysOnTop );
  ui.menuView->addAction( ui.alwaysOnTop );

  // Dictionary bar

  Instances::Group const * igrp = groupInstances.findGroup( cfg.lastMainGroupId );
  if( cfg.lastMainGroupId == Instances::Group::AllGroupId )
  {
    if( igrp )
      igrp->checkMutedDictionaries( &cfg.mutedDictionaries );
    dictionaryBar.setMutedDictionaries( &cfg.mutedDictionaries );
  }
  else
  {
    Config::Group * grp = cfg.getGroup( cfg.lastMainGroupId );
    if( igrp && grp )
      igrp->checkMutedDictionaries( &grp->mutedDictionaries );
    dictionaryBar.setMutedDictionaries( grp ? &grp->mutedDictionaries : 0 );
  }

  showDictBarNamesTriggered(); // Make update its state according to initial
                               // setting

  useSmallIconsInToolbarsTriggered();

  connect( this, SIGNAL( clickOnDictPane( QString const & ) ),
           &dictionaryBar, SLOT( dictsPaneClicked( QString const & ) ) );

  addToolBar( &dictionaryBar );

  connect( dictionaryBar.toggleViewAction(), SIGNAL(triggered(bool)),
           this, SLOT(dictionaryBarToggled(bool)) );
  // This one will be disconnected once the slot is activated. It exists
  // only to handle the initial appearance of the dictionary bar.
  connect( dictionaryBar.toggleViewAction(), SIGNAL(toggled(bool)),
           this, SLOT(dictionaryBarToggled(bool)) );

  connect( &dictionaryBar, SIGNAL(editGroupRequested()),
           this, SLOT(editCurrentGroup()) );

  connect( &dictionaryBar, SIGNAL( showDictionaryInfo( QString const & ) ),
           this, SLOT( showDictionaryInfo( QString const & ) ) );

  // History
  ui.historyPaneWidget->setUp( &cfg, &history, ui.menuHistory );
  history.enableAdd( cfg.preferences.storeHistory );

  connect( ui.historyPaneWidget, SIGNAL( historyItemRequested( QString const & ) ),
           this, SLOT( showHistoryItem( QString const & ) ) );

  connect( ui.historyPane, SIGNAL( visibilityChanged( bool ) ),
           this, SLOT( updateHistoryMenu() ) );

  connect( ui.menuHistory, SIGNAL( aboutToShow() ),
           this, SLOT( updateHistoryMenu() ) );

  // Show tray icon early so the user would be happy. It won't be functional
  // though until the program inits fully.

  if ( cfg.preferences.enableTrayIcon )
  {
    trayIcon = new QSystemTrayIcon( QIcon( ":/icons/programicon.png" ), this );
    trayIcon->setToolTip( tr( "Loading..." ) );
    trayIcon->show();
  }

  connect( navBack, SIGNAL( triggered() ),
           this, SLOT( backClicked() ) );
  connect( navForward, SIGNAL( triggered() ),
           this, SLOT( forwardClicked() ) );

  addTab.setAutoRaise( true );
  addTab.setToolTip( tr( "New Tab"  ) );
  addTab.setFocusPolicy(Qt::ClickFocus);
  addTab.setIcon( QIcon( ":/icons/addtab.png" ) );

  ui.tabWidget->setHideSingleTab(cfg.preferences.hideSingleTab);
  ui.tabWidget->clear();

  ui.tabWidget->setCornerWidget( &addTab, Qt::TopLeftCorner );
  //ui.tabWidget->setCornerWidget( &closeTab, Qt::TopRightCorner );

#if QT_VERSION >= 0x040500
  ui.tabWidget->setMovable( true );
#endif

#ifndef Q_OS_WIN32
  ui.tabWidget->setDocumentMode( true );
#endif

  ui.tabWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( &addTab, SIGNAL( clicked() ),
           this, SLOT( addNewTab() ) );

  connect( ui.tabWidget, SIGNAL( doubleClicked() ),
           this, SLOT( addNewTab() ) );

  connect( ui.tabWidget, SIGNAL( tabCloseRequested( int ) ),
           this, SLOT( tabCloseRequested( int ) ) );

  connect( ui.tabWidget, SIGNAL( currentChanged( int ) ),
           this, SLOT( tabSwitched( int ) ) );

  connect( ui.tabWidget, SIGNAL( customContextMenuRequested(QPoint)) ,
           this, SLOT( tabMenuRequested(QPoint)) );

#if QT_VERSION >= 0x040500
  ui.tabWidget->setTabsClosable( true );
#endif

  connect( ui.quit, SIGNAL( triggered() ),
           qApp, SLOT( quit() ) );

  connect( ui.dictionaries, SIGNAL( triggered() ),
           this, SLOT( editDictionaries() ) );

  connect( ui.preferences, SIGNAL( triggered() ),
           this, SLOT( editPreferences() ) );

  connect( ui.visitHomepage, SIGNAL( triggered() ),
           this, SLOT( visitHomepage() ) );
  connect( ui.visitForum, SIGNAL( triggered() ),
           this, SLOT( visitForum() ) );
  connect( ui.openConfigFolder, SIGNAL( triggered() ),
           this, SLOT( openConfigFolder() ) );
  connect( ui.about, SIGNAL( triggered() ),
           this, SLOT( showAbout() ) );

  connect( groupListInDock, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );

  connect( groupListInToolbar, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );

  connect( ui.translateLine, SIGNAL( textChanged( QString const & ) ),
           this, SLOT( translateInputChanged( QString const & ) ) );

  connect( translateBox->translateLine(), SIGNAL( textChanged( QString const & ) ),
           this, SLOT( translateInputChanged( QString const & ) ) );

  connect( ui.translateLine, SIGNAL( returnPressed() ),
           this, SLOT( translateInputFinished() ) );

  connect( translateBox->translateLine(), SIGNAL( returnPressed() ),
           this, SLOT( translateInputFinished() ) );

  connect( ui.wordList, SIGNAL( itemSelectionChanged() ),
           this, SLOT( wordListSelectionChanged() ) );

  connect( translateBox->wordList(), SIGNAL( itemDoubleClicked ( QListWidgetItem * ) ),
           this, SLOT( wordListItemActivated( QListWidgetItem * ) ) );

  connect( ui.wordList, SIGNAL( itemClicked( QListWidgetItem * ) ),
           this, SLOT( wordListItemActivated( QListWidgetItem * ) ) );

  connect( ui.wordList, SIGNAL( statusBarMessage( QString const &, int, QPixmap const & ) ),
           this, SLOT( showStatusBarMessage( QString const &, int, QPixmap const & ) ) );

  connect( translateBox->wordList(), SIGNAL( statusBarMessage( QString const &, int, QPixmap const & ) ),
           this, SLOT( showStatusBarMessage( QString const &, int, QPixmap const & ) ) );

  connect( ui.dictsList, SIGNAL( itemSelectionChanged() ),
           this, SLOT( dictsListSelectionChanged() ) );

  connect( &configEvents, SIGNAL( mutedDictionariesChanged() ),
           this, SLOT( mutedDictionariesChanged() ) );

  this->installEventFilter( this );

  ui.translateLine->installEventFilter( this );
  translateBox->translateLine()->installEventFilter( this );

  ui.wordList->installEventFilter( this );
  translateBox->wordList()->installEventFilter( this );

  ui.wordList->viewport()->installEventFilter( this );
  translateBox->wordList()->viewport()->installEventFilter( this );

  ui.dictsList->installEventFilter( this );
  ui.dictsList->viewport()->installEventFilter( this );
  //tabWidget doesn't propagate Ctrl+Tab to the parent widget unless event filter is installed
  ui.tabWidget->installEventFilter( this );

  ui.historyList->installEventFilter( this );

#ifdef Q_OS_WIN
  QRect baseGeometry;
  if( cfg.maximizedMainWindowGeometry.width() > 0 )
  {
    if ( cfg.mainWindowGeometry.size() )
      restoreGeometry( cfg.mainWindowGeometry );
    baseGeometry = geometry();
    setGeometry( cfg.maximizedMainWindowGeometry );
    if ( cfg.mainWindowState.size() )
      restoreState( cfg.mainWindowState, 1 );
    setWindowState( windowState() | Qt::WindowMaximized );
  }
  else
#endif
  {
    if ( cfg.mainWindowGeometry.size() )
      restoreGeometry( cfg.mainWindowGeometry );
    if ( cfg.mainWindowState.size() )
      restoreState( cfg.mainWindowState, 1 );
  }

  updateSearchPaneAndBar( cfg.preferences.searchInDock );
  ui.searchPane->setVisible( cfg.preferences.searchInDock );

  applyProxySettings();
  applyWebSettings();

  makeDictionaries();

  // After we have dictionaries and groups, we can populate history
//  historyChanged();

  setWindowTitle( "GoldenDict" );

  blockUpdateWindowTitle = true;
  addNewTab();

  // Create tab list menu
  createTabList();

  // Show the initial welcome text

  {
    ArticleView *view = getCurrentArticleView();

    history.enableAdd( false );

    blockUpdateWindowTitle = true;

    view->showDefinition( tr( "Welcome!" ), Instances::Group::HelpGroupId );

    history.enableAdd( cfg.preferences.storeHistory );
  }

  translateLine->setFocus();

  if ( trayIcon )
  {
    // Upgrade existing dummy tray icon into a full-functional one

    trayIcon->setContextMenu( &trayIconMenu );
    trayIcon->show();

    connect( trayIcon, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
             this, SLOT( trayIconActivated( QSystemTrayIcon::ActivationReason ) ) );
  }

  updateTrayIcon();

  // Update zoomers
  applyZoomFactor();
  applyWordsZoomLevel();

  // Update autostart info
  setAutostart(cfg.preferences.autoStart);

  // Initialize global hotkeys
  installHotKeys();

  if ( cfg.preferences.alwaysOnTop )
  {
    on_alwaysOnTop_triggered( true );
  }

  // Only show window initially if it wasn't configured differently
  if ( !cfg.preferences.enableTrayIcon || !cfg.preferences.startToTray )
  {
    show();
    focusTranslateLine();
#ifdef Q_OS_WIN
    if( baseGeometry.width() > 0 )
    {
      hide();
      setGeometry( baseGeometry );
      showMaximized();
      activateWindow();
    }
#endif
  }

  connect( &newReleaseCheckTimer, SIGNAL( timeout() ),
           this, SLOT( checkForNewRelease() ) );

  if ( cfg.preferences.hideMenubar )
  {
    toggleMenuBarTriggered( false );
  }

  prepareNewReleaseChecks();

  // makeDictionaries() didn't do deferred init - we do it here, at the end.
  doDeferredInit( dictionaries );

  updateStatusLine();

#ifdef Q_OS_MACX
  if( cfg.preferences.startWithScanPopupOn && !AXAPIEnabled() )
      mainStatusBar->showMessage( tr( "Accessibility API is not enabled" ), 10000,
                                      QPixmap( ":/icons/error.png" ) );
#endif

  wasMaximized = isMaximized();

  history.setSaveInterval( cfg.preferences.historyStoreInterval );

  #ifdef Q_OS_MAC
    LionSupport::addFullscreen(this);
  #endif
#ifdef Q_OS_WIN32
  gdAskMessage = RegisterWindowMessage( GD_MESSAGE_NAME );
#endif
}

void MainWindow::ctrlTabPressed()
{
    emit fillWindowsMenu();
    tabListButton->click();
}

void MainWindow::updateSearchPaneAndBar( bool searchInDock )
{
  QString text = translateLine->text();

  if ( searchInDock )
  {
    cfg.preferences.searchInDock = true;

    navToolbar->setAllowedAreas( Qt::AllToolBarAreas );

    groupList = groupListInDock;
    translateLine = ui.translateLine;
    wordList = ui.wordList;

    translateBoxToolBarAction->setVisible( false );
  }
  else
  {
    cfg.preferences.searchInDock = false;

    // handle the main toolbar, it must not be on the side, since it should
    // contain the group widget and the translate line. Valid locations: Top and Bottom.
    navToolbar->setAllowedAreas( Qt::BottomToolBarArea | Qt::TopToolBarArea );
    if ( toolBarArea( navToolbar ) & ( Qt::LeftToolBarArea | Qt::RightToolBarArea ) )
    {
      if ( toolBarArea( &dictionaryBar ) == Qt::TopToolBarArea )
      {
        insertToolBar( &dictionaryBar, navToolbar );
      }
      else
      {
        addToolBar( Qt::TopToolBarArea, navToolbar );
      }
    }

    groupList = groupListInToolbar;
    translateLine = translateBox->translateLine();
    wordList = translateBox->wordList();

    translateBoxToolBarAction->setVisible( true );
  }

  // reset the flag when switching UI modes
  wordListSelChanged = false;

  wordList->attachFinder( &wordFinder );

  updateGroupList();
  applyWordsZoomLevel();

  if ( cfg.preferences.searchInDock )
    translateLine->setText( text );
  else
    translateBox->setText( text, false );

  focusTranslateLine();
}

void MainWindow::mousePressEvent( QMouseEvent *event)
{

  if (handleBackForwardMouseButtons( event ) )
  {
    return;
  }

  if (event->button() != Qt::MidButton)
    return QMainWindow::mousePressEvent(event);

  // middle clicked
  QString subtype = "plain";

    QString str = QApplication::clipboard()->text(subtype,
      QClipboard::Selection);
  translateLine->setText(str);

        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Enter,
           Qt::NoModifier);
        QApplication::sendEvent(translateLine, &ev);
}

MainWindow::~MainWindow()
{
#ifdef Q_OS_WIN
  if( isMaximized() )
    cfg.maximizedMainWindowGeometry = geometry();
  else
    cfg.maximizedMainWindowGeometry = QRect();
#endif

  commitData();

  // Close all tabs -- they should be destroyed before network managers
  // do.
  while( ui.tabWidget->count() )
  {
    QWidget * w = ui.tabWidget->widget( 0 );

    ui.tabWidget->removeTab( 0 );

    delete w;
  }

  history.save();
}

void MainWindow::commitData( QSessionManager & )
{
  commitData();
}

void MainWindow::commitData()
{
  if ( !commitDataCompleted )
  {
    commitDataCompleted = true;

    // Save MainWindow state and geometry
    cfg.mainWindowState = saveState( 1 );
    cfg.mainWindowGeometry = saveGeometry();

    // Close the popup, so it would save its geometry to config

    scanPopup.reset();

    // Save any changes in last chosen groups etc
    Config::save( cfg );
  }
}

QPrinter & MainWindow::getPrinter()
{
  if ( printer.get() )
    return *printer;

  printer = new QPrinter( QPrinter::HighResolution );

  return *printer;
}

void MainWindow::applyQtStyleSheet( QString const & displayStyle, QString const & addonStyle )
{
  QFile builtInCssFile( ":/qt-style.css" );
  builtInCssFile.open( QFile::ReadOnly );
  QByteArray css = builtInCssFile.readAll();

#if defined(Q_OS_MAC)
  QFile macCssFile( ":/qt-style-macos.css" );
  macCssFile.open( QFile::ReadOnly );
  css += macCssFile.readAll();
#endif

  if ( displayStyle.size() )
  {
    // Load an additional stylesheet
    QFile builtInCssFile( QString( ":/qt-style-st-%1.css" ).arg( displayStyle ) );
    builtInCssFile.open( QFile::ReadOnly );
    css += builtInCssFile.readAll();
  }

  // Try loading a style sheet if there's one
  QFile cssFile( Config::getUserQtCssFileName() );

  if ( cssFile.open( QFile::ReadOnly ) )
    css += cssFile.readAll();

  if( !addonStyle.isEmpty() )
  {
    QString name = Config::getStylesDir() + addonStyle
                   + QDir::separator() + "qt-style.css";
    QFile addonCss( name );
    if( addonCss.open( QFile::ReadOnly ) )
      css += addonCss.readAll();
  }

  setStyleSheet( css );
}

void MainWindow::updateTrayIcon()
{
  if ( !trayIcon && cfg.preferences.enableTrayIcon )
  {
    // Need to show it
    trayIcon = new QSystemTrayIcon( QIcon( ":/icons/programicon.png" ), this );
    trayIcon->setContextMenu( &trayIconMenu );
    trayIcon->show();

    connect( trayIcon, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
             this, SLOT( trayIconActivated( QSystemTrayIcon::ActivationReason ) ) );
  }
  else
  if ( trayIcon && !cfg.preferences.enableTrayIcon )
  {
    // Need to hide it
    delete trayIcon;

    trayIcon = 0;
  }
  if ( trayIcon )
  {
    // Update the icon to reflect the scanning mode
    trayIcon->setIcon( QIcon(
      enableScanPopup->isChecked() ?
        ":/icons/programicon_scan.png" :
        ":/icons/programicon.png" ) );

    trayIcon->setToolTip( "GoldenDict" );
  }

  // The 'Close to tray' action is associated with the tray icon, so we hide
  // or show it here.
  ui.actionCloseToTray->setVisible( cfg.preferences.enableTrayIcon );
}

void MainWindow::wheelEvent( QWheelEvent *ev )
{
  if ( ev->modifiers().testFlag( Qt::ControlModifier ) )
  {
    if ( ev->delta() > 0 )
    {
        zoomin();
    }
    else if ( ev->delta() < 0 )
    {
        zoomout();
    }
    ev->accept();
  }
  else
  {
    ev->ignore();
  }
}

void MainWindow::closeEvent( QCloseEvent * ev )
{
  if ( cfg.preferences.enableTrayIcon && cfg.preferences.closeToTray )
  {
    ev->ignore();
    hide();
  }
  else
  {
    ev->accept();
    qApp->quit();
  }
}

void MainWindow::applyProxySettings()
{
  QNetworkProxy::ProxyType type = QNetworkProxy::NoProxy;

  if ( cfg.preferences.proxyServer.enabled )
  {
    switch( cfg.preferences.proxyServer.type )
    {
      case Config::ProxyServer::Socks5:
        type = QNetworkProxy::Socks5Proxy;
      break;
      case Config::ProxyServer::HttpConnect:
        type = QNetworkProxy::HttpProxy;
      break;
      case Config::ProxyServer::HttpGet:
        type = QNetworkProxy::HttpCachingProxy;
      break;

      default:
      break;
    }
  }

  QNetworkProxy proxy( type );

  if ( cfg.preferences.proxyServer.enabled )
  {
   proxy.setHostName( cfg.preferences.proxyServer.host );
   proxy.setPort( cfg.preferences.proxyServer.port );

   if ( cfg.preferences.proxyServer.user.size() )
     proxy.setUser( cfg.preferences.proxyServer.user );

   if ( cfg.preferences.proxyServer.password.size() )
     proxy.setPassword( cfg.preferences.proxyServer.password );
  }

  QNetworkProxy::setApplicationProxy( proxy );
}

void MainWindow::applyWebSettings()
{
  QWebSettings *defaultSettings = QWebSettings::globalSettings();
  defaultSettings->setAttribute(QWebSettings::PluginsEnabled, cfg.preferences.enableWebPlugins);
}

void MainWindow::makeDictionaries()
{
  scanPopup.reset();

  wordFinder.clear();

  dictionariesUnmuted.clear();

  loadDictionaries( this, isVisible(), cfg, dictionaries, dictNetMgr, false );

  updateStatusLine();
  updateGroupList();
  makeScanPopup();
}

void MainWindow::updateStatusLine()
{
  unsigned articleCount = 0, wordCount = 0;

  for( unsigned x = dictionaries.size(); x--; )
  {
    articleCount += dictionaries[ x ]->getArticleCount();
    wordCount += dictionaries[ x ]->getWordCount();
  }

  mainStatusBar->showMessage( tr( "%1 dictionaries, %2 articles, %3 words" ).
                              arg( dictionaries.size() ).arg( articleCount ).
                              arg( wordCount ), 10000 );
}

void MainWindow::updateGroupList()
{
  bool haveGroups = cfg.groups.size();

  groupList->setVisible( haveGroups );

  groupLabel.setText( haveGroups ? tr( "Look up in:" ) : tr( "Look up:" ) );

  // currentIndexChanged() signal is very trigger-happy. To avoid triggering
  // it, we disconnect it while we're clearing and filling back groups.
  disconnect( groupList, SIGNAL( currentIndexChanged( QString const & ) ),
              this, SLOT( currentGroupChanged( QString const & ) ) );

  groupInstances.clear();

  // Add dictionaryOrder first, as the 'All' group.
  {
    Instances::Group g( cfg.dictionaryOrder, dictionaries );

    // Add any missing entries to dictionary order
    Instances::complementDictionaryOrder( g,
                                          Instances::Group( cfg.inactiveDictionaries, dictionaries ),
                                          dictionaries );

    g.name = tr( "All" );
    g.id = Instances::Group::AllGroupId;
    g.icon = "folder.png";

    groupInstances.push_back( g );
  }

  for( int x  = 0; x < cfg.groups.size(); ++x )
    groupInstances.push_back( Instances::Group( cfg.groups[ x ], dictionaries ) );

  // Update names for dictionaries that are present, so that they could be
  // found in case they got moved.
  Instances::updateNames( cfg, dictionaries );

  groupList->fill( groupInstances );
  groupList->setCurrentGroup( cfg.lastMainGroupId );
  updateCurrentGroupProperty();

  updateDictionaryBar();

  connect( groupList, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );
}

void MainWindow::updateDictionaryBar()
{
  if ( !dictionaryBar.toggleViewAction()->isChecked() )
    return; // It's not enabled, therefore hidden -- don't waste time

  unsigned currentId = groupList -> getCurrentGroup();
  Instances::Group * grp = groupInstances.findGroup( currentId );

  dictionaryBar.setMutedDictionaries( 0 );
  if ( grp ) { // Should always be !0, but check as a safeguard
    if( currentId == Instances::Group::AllGroupId )
      dictionaryBar.setMutedDictionaries( &cfg.mutedDictionaries );
    else
    {
      Config::Group * grp = cfg.getGroup( currentId );
      dictionaryBar.setMutedDictionaries( grp ? &grp->mutedDictionaries : 0 );
    }

    dictionaryBar.setDictionaries( grp->dictionaries );

    if ( useSmallIconsInToolbarsAction.isChecked() ) {
      int extent = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
      dictionaryBar.setDictionaryIconSize( extent );
    }
  }
}

void MainWindow::makeScanPopup()
{
  scanPopup.reset();

  if ( !cfg.preferences.enableScanPopup &&
       !cfg.preferences.enableClipboardHotkey )
    return;

  scanPopup = new ScanPopup( 0, cfg, articleNetMgr, dictionaries, groupInstances,
                             history );

  scanPopup->setStyleSheet( styleSheet() );

  if ( cfg.preferences.enableScanPopup && enableScanPopup->isChecked() )
    scanPopup->enableScanning();

  connect( scanPopup.get(), SIGNAL(editGroupRequested( unsigned ) ),
           this, SLOT(editDictionaries( unsigned )), Qt::QueuedConnection );

  connect( scanPopup.get(), SIGNAL(sendWordToMainWindow( QString const & ) ),
           this, SLOT(wordReceived( QString const & )), Qt::QueuedConnection );

  connect( this, SIGNAL( setExpandOptionalParts( bool ) ),
           scanPopup.get(), SIGNAL( setViewExpandMode( bool ) ) );

  connect( scanPopup.get(), SIGNAL( setExpandMode( bool ) ),
           this, SLOT( setExpandMode( bool ) ) );

  connect( scanPopup.get(), SIGNAL( forceAddWordToHistory( const QString & ) ),
           this, SLOT( forceAddWordToHistory( const QString & ) ) );

  connect( scanPopup.get(), SIGNAL( showDictionaryInfo( const QString & ) ),
           this, SLOT( showDictionaryInfo( const QString & ) ) );

  connect( scanPopup.get(), SIGNAL( sendWordToHistory( QString ) ),
           this, SLOT( addWordToHistory( QString ) ) );

#ifdef Q_OS_WIN32
  connect( scanPopup.get(), SIGNAL( isGoldenDictWindow( HWND ) ),
           this, SLOT( isGoldenDictWindow( HWND ) ) );
#endif
}

vector< sptr< Dictionary::Class > > const & MainWindow::getActiveDicts()
{
  if ( groupInstances.empty() )
    return dictionaries;

  int current = groupList->currentIndex();

  if ( current < 0 || current >= (int) groupInstances.size() )
  {
    // This shouldn't ever happen
    return dictionaries;
  }

  Config::MutedDictionaries const * mutedDictionaries = dictionaryBar.getMutedDictionaries();
  if ( !dictionaryBar.toggleViewAction()->isChecked() || mutedDictionaries == 0 )
    return groupInstances[ current ].dictionaries;
  else
  {
    vector< sptr< Dictionary::Class > > const & activeDicts =
      groupInstances[ current ].dictionaries;

    // Populate the special dictionariesUnmuted array with only unmuted
    // dictionaries

    dictionariesUnmuted.clear();
    dictionariesUnmuted.reserve( activeDicts.size() );

    for( unsigned x = 0; x < activeDicts.size(); ++x )
      if ( !mutedDictionaries->contains(
              QString::fromStdString( activeDicts[ x ]->getId() ) ) )
        dictionariesUnmuted.push_back( activeDicts[ x ] );

    return dictionariesUnmuted;
  }
}

void MainWindow::createTabList()
{
  tabListMenu->setIcon(QIcon(":/icons/windows-list.png"));
  connect(tabListMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowsMenu()));
  connect(tabListMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchToWindow(QAction*)));

  tabListButton = new QToolButton(ui.tabWidget);
  tabListButton->setAutoRaise(true);
  tabListButton->setIcon(tabListMenu->icon());
  tabListButton->setMenu(tabListMenu);
  tabListButton->setToolTip( tr( "Open Tabs List" ) );
  tabListButton->setPopupMode(QToolButton::InstantPopup);
  ui.tabWidget->setCornerWidget(tabListButton);
  tabListButton->setFocusPolicy(Qt::ClickFocus);
}

void MainWindow::fillWindowsMenu()
{
  tabListMenu->clear();

  if(cfg.preferences.mruTabOrder)
  {
    for (int i = 0; i < mruList.count(); i++)
    {
      QAction *act = tabListMenu->addAction(ui.tabWidget->tabIcon(ui.tabWidget->indexOf(mruList.at(i))), ui.tabWidget->tabText(ui.tabWidget->indexOf(mruList.at(i))));

      //remember the index of the Tab to be later used in ctrlReleased()
      act->setData(ui.tabWidget->indexOf(mruList.at(i)));

      if (ui.tabWidget->currentIndex() == ui.tabWidget->indexOf(mruList.at(i)))
      {
        QFont f( act->font() );
        f.setBold( true );
        act->setFont( f );
      }
    }
    if (tabListMenu->actions().size() > 1)
    {
      tabListMenu->setActiveAction(tabListMenu->actions().at(1));
    }
  }
  else
  {
    for (int i = 0; i < ui.tabWidget->count(); i++)
    {
      QAction *act = tabListMenu->addAction( ui.tabWidget->tabIcon( i ),
      ui.tabWidget->tabText( i ) );
      act->setData( i );
      if (ui.tabWidget->currentIndex() == i)
      {
        QFont f( act->font() );
        f.setBold( true );
        act->setFont( f );
      }
    }
  }
  return;
}

void MainWindow::switchToWindow(QAction *act)
{
  int idx = act->data().toInt();
  ui.tabWidget->setCurrentIndex(idx);
}


void MainWindow::addNewTab()
{
  createNewTab( true, tr( "(untitled)" ) );
}

ArticleView * MainWindow::createNewTab( bool switchToIt,
                                        QString const & name )
{
  ArticleView * view = new ArticleView( this, articleNetMgr, dictionaries,
                                        groupInstances, false, cfg,
                                        dictionaryBar.toggleViewAction(),
                                        groupList );

  connect( view, SIGNAL( titleChanged(  ArticleView *, QString const & ) ),
           this, SLOT( titleChanged(  ArticleView *, QString const & ) ) );

  connect( view, SIGNAL( iconChanged( ArticleView *, QIcon const & ) ),
           this, SLOT( iconChanged( ArticleView *, QIcon const & ) ) );

  connect( view, SIGNAL( pageLoaded( ArticleView * ) ),
           this, SLOT( pageLoaded( ArticleView * ) ) );

  connect( view, SIGNAL( openLinkInNewTab( QUrl const &, QUrl const &, QString const &, ArticleView::Contexts const & ) ),
           this, SLOT( openLinkInNewTab( QUrl const &, QUrl const &, QString const &, ArticleView::Contexts const & ) ) );

  connect( view, SIGNAL( showDefinitionInNewTab( QString const &, unsigned, QString const &, ArticleView::Contexts const & ) ),
           this, SLOT( showDefinitionInNewTab( QString const &, unsigned, QString const &, ArticleView::Contexts const & ) ) );

  connect( view, SIGNAL( typingEvent( QString const & ) ),
           this, SLOT( typingEvent( QString const & ) ) );

  connect( view, SIGNAL( activeArticleChanged( const QString & ) ),
           this, SLOT( activeArticleChanged( const QString & ) ) );

  connect( view, SIGNAL( statusBarMessage( QString const &, int, QPixmap const & ) ),
           this, SLOT( showStatusBarMessage( QString const &, int, QPixmap const & ) ) );

  connect( view, SIGNAL( showDictsPane( ) ), this, SLOT( showDictsPane( ) ) );

  connect( view, SIGNAL( forceAddWordToHistory( const QString & ) ),
           this, SLOT( forceAddWordToHistory( const QString & ) ) );

  connect( this, SIGNAL( setExpandOptionalParts( bool ) ),
           view, SLOT( receiveExpandOptionalParts( bool ) ) );

  connect( view, SIGNAL( setExpandMode( bool ) ), this, SLOT( setExpandMode( bool ) ) );

  connect( view, SIGNAL( sendWordToHistory( QString ) ),
           this, SLOT( addWordToHistory( QString ) ) );

  connect( view, SIGNAL( sendWordToInputLine( QString const & ) ),
           this, SLOT( sendWordToInputLine( QString const & ) ) );

  connect( view, SIGNAL( storeResourceSavePath( QString const & ) ),
           this, SLOT( storeResourceSavePath( QString const & ) ) );

  view->setSelectionBySingleClick( cfg.preferences.selectWordBySingleClick );

  int index = cfg.preferences.newTabsOpenAfterCurrentOne ?
              ui.tabWidget->currentIndex() + 1 : ui.tabWidget->count();

  QString escaped = name;
  escaped.replace( "&", "&&" );

  ui.tabWidget->insertTab( index, view, escaped );
  mruList.append(dynamic_cast<QWidget*>(view));

  if ( switchToIt )
    ui.tabWidget->setCurrentIndex( index );

  view->setZoomFactor( cfg.preferences.zoomFactor );

#ifdef Q_OS_WIN32
  view->installEventFilter( this );
#endif
  return view;
}

void MainWindow::tabCloseRequested( int x )
{
//  if ( ui.tabWidget->count() < 2 )
//    return; // We should always have at least one open tab

  QWidget * w = ui.tabWidget->widget( x );

  if (cfg.preferences.mruTabOrder)
  {
    //removeTab activates next tab and emits currentChannged SIGNAL
    //This is not what we want for MRU, so disable the signal for a moment

    disconnect( ui.tabWidget, SIGNAL( currentChanged( int ) ),
             this, SLOT( tabSwitched( int ) ) );
  }

  ui.tabWidget->removeTab( x );

  if (cfg.preferences.mruTabOrder)
  {
    connect( ui.tabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( tabSwitched( int ) ) );
  }

  mruList.removeOne(w);

  delete w;

  //activate a tab in accordance with MRU
  if ( mruList.size() > 0 ) {
    ui.tabWidget->setCurrentWidget(mruList.at(0));
  }

  // if everything is closed, add new tab
  if ( ui.tabWidget->count() == 0 )
    addNewTab();
}

void MainWindow::closeCurrentTab()
{
  tabCloseRequested( ui.tabWidget->currentIndex() );
}

void MainWindow::closeAllTabs()
{
  while (ui.tabWidget->count() > 1)
    closeCurrentTab();

  // close last tab
  closeCurrentTab();
}

void MainWindow::closeRestTabs()
{
  if ( ui.tabWidget->count() < 2 )
    return;

  int idx = ui.tabWidget->currentIndex();

  for (int i = 0; i < idx; i++)
    tabCloseRequested(0);

  ui.tabWidget->setCurrentIndex(0);

  while (ui.tabWidget->count() > 1)
    tabCloseRequested(1);
}

void MainWindow::switchToNextTab()
{
  if ( ui.tabWidget->count() < 2 )
    return;

  ui.tabWidget->setCurrentIndex( ( ui.tabWidget->currentIndex() + 1 ) % ui.tabWidget->count() );
}

void MainWindow::switchToPrevTab()
{
  if ( ui.tabWidget->count() < 2 )
    return;

  if ( !ui.tabWidget->currentIndex() )
    ui.tabWidget->setCurrentIndex( ui.tabWidget->count() - 1 );
  else
    ui.tabWidget->setCurrentIndex( ui.tabWidget->currentIndex() - 1 );
}

//emitted by tabListMenu when user releases Ctrl
void MainWindow::ctrlReleased()
{
    if (tabListMenu->actions().size() > 1)
    {
	ui.tabWidget->setCurrentIndex(tabListMenu->activeAction()->data().toInt());
    }
    tabListMenu->hide();
}

void MainWindow::backClicked()
{
  DPRINTF( "Back\n" );

  ArticleView *view = getCurrentArticleView();

  view->back();
}

void MainWindow::forwardClicked()
{
  DPRINTF( "Forward\n" );

  ArticleView *view = getCurrentArticleView();

  view->forward();
}

void MainWindow::titleChanged( ArticleView * view, QString const & title )
{
  QString escaped = title;
  escaped.replace( "&", "&&" );

  ui.tabWidget->setTabText( ui.tabWidget->indexOf( view ), escaped );
  updateWindowTitle();
}

void MainWindow::iconChanged( ArticleView * view, QIcon const & icon )
{
  ui.tabWidget->setTabIcon( ui.tabWidget->indexOf( view ), groupInstances.size() > 1 ? icon : QIcon() );
}

void MainWindow::updateWindowTitle()
{
  ArticleView *view = getCurrentArticleView();
  if ( view )
  {
    QString str = view->getTitle();
    if( !str.isEmpty() )
    {
      if( !blockUpdateWindowTitle )
        setWindowTitle( tr( "%1 - %2" ).arg( str, "GoldenDict" ) );
      blockUpdateWindowTitle = false;
    }
  }
}

void MainWindow::pageLoaded( ArticleView * view )
{
  updateBackForwardButtons();

  updatePronounceAvailability();

  if ( cfg.preferences.pronounceOnLoadMain )
    pronounce( view );

  updateFoundInDictsList();
}

void MainWindow::showStatusBarMessage( QString const & message, int timeout, QPixmap const & icon )
{
  mainStatusBar->showMessage( message, timeout, icon );
}

void MainWindow::tabSwitched( int )
{
  translateBox->setPopupEnabled( false );
  updateBackForwardButtons();
  updatePronounceAvailability();
  updateFoundInDictsList();
  updateWindowTitle();
  if (mruList.size() > 1)
  {
    mruList.move(mruList.indexOf(ui.tabWidget->widget(ui.tabWidget->currentIndex())),0);
  }
}

void MainWindow::tabMenuRequested(QPoint pos)
{
//  // dont show this menu for single tab
//  if ( ui.tabWidget->count() < 2 )
//    return;

  tabMenu->popup(ui.tabWidget->mapToGlobal(pos));
}

void MainWindow::dictionaryBarToggled( bool )
{
  // From now on, only the triggered() signal is interesting to us
  disconnect( dictionaryBar.toggleViewAction(), SIGNAL(toggled(bool)),
              this, SLOT(dictionaryBarToggled(bool)) );

  updateDictionaryBar(); // Updates dictionary bar contents if it's shown
  applyMutedDictionariesState(); // Visibility change affects searches and results
}

void MainWindow::pronounce( ArticleView * view )
{
  if ( view )
    view->playSound();
  else
    getCurrentArticleView()->playSound();
}

void MainWindow::showDictsPane( )
{
  if( !ui.dictsPane->isVisible() )
    ui.dictsPane->show();
}

void MainWindow::dictsPaneVisibilityChanged( bool visible )
{
  if (visible) {
    updateFoundInDictsList();
  }
}

void MainWindow::updateFoundInDictsList()
{
  if (!ui.dictsList->isVisible())
  {
    // nothing to do, the list is not visible
    return;
  }

  ui.dictsList->clear();

  ArticleView *view = getCurrentArticleView();

  if ( view )
  {
    QStringList ids = view->getArticlesList();
    QString activeId = view->getActiveArticleId();

    for( QStringList::const_iterator i = ids.constBegin(); i != ids.constEnd(); ++i)
    {
      // Find this dictionary

      for( unsigned x = dictionaries.size(); x--; )
      {
        if ( dictionaries[ x ]->getId() == i->toUtf8().data() )
        {
          QString dictName = QString::fromUtf8( dictionaries[ x ]->getName().c_str() );
          QString dictId = QString::fromUtf8( dictionaries[ x ]->getId().c_str() );
          QListWidgetItem * item =
              new QListWidgetItem(
                dictionaries[ x ]->getIcon().pixmap(32).scaledToHeight( 21, Qt::SmoothTransformation ),
                dictName,
                ui.dictsList, QListWidgetItem::Type );
          item->setData(Qt::UserRole, QVariant( dictId ) );
          item->setToolTip(dictName);

          ui.dictsList->addItem( item );
          if (dictId == activeId)
          {
            ui.dictsList->setCurrentItem(item);
          }
          break;
        }
      }
    }
  }
}

void MainWindow::updateBackForwardButtons()
{
  ArticleView *view = getCurrentArticleView();

  if ( view )
  {
    navBack->setEnabled(view->canGoBack());
    navForward->setEnabled(view->canGoForward());
  }
}

void MainWindow::updatePronounceAvailability()
{
  bool pronounceEnabled = ui.tabWidget->count() > 0 &&
    getCurrentArticleView()->hasSound();

  navPronounce->setEnabled( pronounceEnabled );
}

void MainWindow::editDictionaries( unsigned editDictionaryGroup )
{
  hotkeyWrapper.reset(); // No hotkeys while we're editing dictionaries
  scanPopup.reset(); // No scan popup either. No one should use dictionaries.

  wordFinder.clear();
  dictionariesUnmuted.clear();

  { // Limit existence of newCfg

  Config::Class newCfg = cfg;
  EditDictionaries dicts( this, newCfg, dictionaries, groupInstances, dictNetMgr );

  if ( editDictionaryGroup != Instances::Group::NoGroupId )
    dicts.editGroup( editDictionaryGroup );

  dicts.exec();

  if ( dicts.areDictionariesChanged() || dicts.areGroupsChanged() )
  {

    // Set muted dictionaries from old groups
    for( int x = 0; x < newCfg.groups.size(); x++ )
    {
      unsigned id = newCfg.groups[ x ].id;
      if( id != Instances::Group::NoGroupId )
      {
        Config::Group const * grp = cfg.getGroup( id );
        if( grp )
        {
          newCfg.groups[ x ].mutedDictionaries = grp->mutedDictionaries;
          newCfg.groups[ x ].popupMutedDictionaries = grp->popupMutedDictionaries;
        }
      }
    }

    cfg = newCfg;

    updateGroupList();

    Config::save( cfg );
  }

  }

  makeScanPopup();
  installHotKeys();
}

void MainWindow::editCurrentGroup()
{
  editDictionaries( groupList->getCurrentGroup() );
}

void MainWindow::editPreferences()
{
  hotkeyWrapper.reset(); // So we could use the keys it hooks

  Preferences preferences( this, cfg.preferences );

  preferences.show();

  if ( preferences.exec() == QDialog::Accepted )
  {
    Config::Preferences p = preferences.getPreferences();

    // These parameters are not set in dialog
    p.zoomFactor = cfg.preferences.zoomFactor;
    p.wordsZoomLevel = cfg.preferences.wordsZoomLevel;
    p.hideMenubar = cfg.preferences.hideMenubar;
    p.searchInDock = cfg.preferences.searchInDock;
    p.alwaysOnTop = cfg.preferences.alwaysOnTop;

    bool needReload = false;

    // See if we need to reapply stylesheets
    if ( cfg.preferences.displayStyle != p.displayStyle || cfg.preferences.addonStyle != p.addonStyle )
    {
      applyQtStyleSheet( p.displayStyle, p.addonStyle );
      articleMaker.setDisplayStyle( p.displayStyle, p.addonStyle );
      needReload = true;
    }

    // See if we need to reapply expand optional parts mode
    if( cfg.preferences.alwaysExpandOptionalParts != p.alwaysExpandOptionalParts )
    {
      emit setExpandOptionalParts( p.alwaysExpandOptionalParts );
      // Signal setExpandOptionalParts reload all articles
      needReload = false;
    }

    for( int x = 0; x < ui.tabWidget->count(); ++x )
    {
      ArticleView & view =
        dynamic_cast< ArticleView & >( *( ui.tabWidget->widget( x ) ) );

      view.setSelectionBySingleClick( p.selectWordBySingleClick );

      if( needReload )
        view.reload();
    }

    if( cfg.preferences.historyStoreInterval != p.historyStoreInterval )
      history.setSaveInterval( p.historyStoreInterval );

    cfg.preferences = p;

    beforeScanPopupSeparator->setVisible( cfg.preferences.enableScanPopup );
    enableScanPopup->setVisible( cfg.preferences.enableScanPopup );
    afterScanPopupSeparator->setVisible( cfg.preferences.enableScanPopup );

    if ( !cfg.preferences.enableScanPopup )
      enableScanPopup->setChecked( false );

    updateTrayIcon();
    applyProxySettings();
    applyWebSettings();
    makeScanPopup();

    ui.tabWidget->setHideSingleTab(cfg.preferences.hideSingleTab);

    setAutostart( cfg.preferences.autoStart );

    prepareNewReleaseChecks();

    history.enableAdd( cfg.preferences.storeHistory );
    history.setMaxSize( cfg.preferences.maxStringsInHistory );
    ui.historyPaneWidget->updateHistoryCounts();

    Config::save( cfg );
  }

  installHotKeys();
}

void MainWindow::currentGroupChanged( QString const & )
{
  cfg.lastMainGroupId = groupList->getCurrentGroup();
  Instances::Group const * igrp = groupInstances.findGroup( cfg.lastMainGroupId );
  if( cfg.lastMainGroupId == Instances::Group::AllGroupId )
  {
    if( igrp )
      igrp->checkMutedDictionaries( &cfg.mutedDictionaries );
    dictionaryBar.setMutedDictionaries( &cfg.mutedDictionaries );
  }
  else
  {
    Config::Group * grp = cfg.getGroup( cfg.lastMainGroupId );
    if( grp )
    {
      if( igrp )
        igrp->checkMutedDictionaries( &grp->mutedDictionaries );
      dictionaryBar.setMutedDictionaries( &grp->mutedDictionaries );
    }
    else
      dictionaryBar.setMutedDictionaries( 0 );
  }

  updateDictionaryBar();

  // Update word search results
  translateBox->setPopupEnabled( false );
  translateInputChanged( translateLine->text() );
  translateInputFinished( false );

  updateCurrentGroupProperty();
}

void MainWindow::updateCurrentGroupProperty()
{
  // We maintain currentGroup property so styles could use that to change
  // fonts based on group names
  Instances::Group * grp =
      groupInstances.findGroup( groupList->getCurrentGroup() );

  if ( grp && translateLine->property( "currentGroup" ).toString() !=
       grp->name )
  {
    translateLine->setProperty( "currentGroup", grp->name );
    wordList->setProperty( "currentGroup", grp->name );
    QString ss = styleSheet();

    // Only update stylesheet if it mentions currentGroup, as updating the
    // stylesheet is a slow operation
    if ( ss.contains("currentGroup") )
      setStyleSheet( ss );
  }
}

void MainWindow::translateInputChanged( QString const & newValue )
{
  // If there's some status bar message present, clear it since it may be
  // about the previous search that has failed.
  if ( !mainStatusBar->currentMessage().isEmpty() )
  {
    mainStatusBar->clearMessage();
  }

  // If some word is selected in the word list, unselect it. This prevents
  // triggering a set of spurious activation signals when the list changes.

  if ( wordList->selectionModel()->hasSelection() )
    wordList->setCurrentItem( 0, QItemSelectionModel::Clear );

  QString req = newValue.trimmed();

  if ( !req.size() )
  {
    // An empty request always results in an empty result
    wordFinder.cancel();
    wordList->clear();
    wordList->unsetCursor();

    // Reset the noResults mark if it's on right now
    if ( translateLine->property( "noResults" ).toBool() )
    {
      translateLine->setProperty( "noResults", false );
      setStyleSheet( styleSheet() );
    }
    return;
  }

  wordList->setCursor( Qt::WaitCursor );

  wordFinder.prefixMatch( req, getActiveDicts() );
}

void MainWindow::translateInputFinished( bool checkModifiers )
{
  QString word = translateLine->text();

  if ( word.size() )
  {
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if ( checkModifiers && ( mods & (Qt::ControlModifier | Qt::ShiftModifier) ) )
      addNewTab();

    showTranslationFor( word );

    if ( cfg.preferences.searchInDock )
    {
      if ( ui.searchPane->isFloating() )
        activateWindow();
    }

    getCurrentArticleView()->focus();
  }
}

void MainWindow::handleEsc()
{
  ArticleView *view = getCurrentArticleView();
  if ( view && view->closeSearch() )
    return;

  if( cfg.preferences.escKeyHidesMainWindow )
  {
    toggleMainWindow();
  }
  else
    focusTranslateLine();
}

void MainWindow::focusTranslateLine()
{
  if ( cfg.preferences.searchInDock )
  {
    if ( ui.searchPane->isFloating() )
      ui.searchPane->activateWindow();
  }
  else
  {
    if ( !isActiveWindow() )
      activateWindow();
  }

  translateLine->setFocus();
  translateLine->selectAll();
}

void MainWindow::applyMutedDictionariesState()
{
  translateBox->setPopupEnabled( false );

  // Redo the current search request
  translateInputChanged( translateLine->text() );

  ArticleView *view = getCurrentArticleView();

  if ( view )
  {
    // Update active article view
    view->updateMutedContents();
  }
}

bool MainWindow::handleBackForwardMouseButtons ( QMouseEvent * event) {
  if ( event->button() == Qt::XButton1 ) {
    backClicked();
    return true;
  }
  else
  if ( event->button() == Qt::XButton2 ) {
    forwardClicked();
    return true;
  }
  else
    return false;
}

bool MainWindow::eventFilter( QObject * obj, QEvent * ev )
{
  if ( ev->type() == QEvent::ShortcutOverride ) {
    // Handle Ctrl+H to show the History Pane.
    QKeyEvent * ke = static_cast<QKeyEvent*>( ev );
    if ( ke->key() == Qt::Key_H && ke->modifiers() == Qt::ControlModifier )
    {
      on_showHideHistory_triggered();
      ev->accept();
      return true;
    }

    // Handle F3/Shift+F3 shortcuts
    if ( ke->key() == Qt::Key_F3 )
    {
      ArticleView  * view = getCurrentArticleView();
      if ( view  && view->handleF3( obj, ev ) )
        return true;
    }
  }

  // when the main window is moved or resized, hide the word list suggestions
  if ( obj == this && ( ev->type() == QEvent::Move || ev->type() == QEvent::Resize ) )
  {
    if ( !cfg.preferences.searchInDock )
    {
        translateBox->setPopupEnabled( false );
        return false;
    }
  }

  if ( obj == this && ev->type() == QEvent::WindowStateChange )
  {
    QWindowStateChangeEvent *stev = static_cast< QWindowStateChangeEvent *>( ev );
    wasMaximized = ( stev->oldState() == Qt::WindowMaximized && isMinimized() );
  }

  if ( ev->type() == QEvent::MouseButtonPress ) {
    QMouseEvent * event = static_cast< QMouseEvent * >( ev );

    // clicks outside of the word list should hide it.
    if (obj != translateBox->wordList() && obj != translateBox->wordList()->viewport()) {
      translateBox->setPopupEnabled( false );
    }

    return handleBackForwardMouseButtons( event );
  }

  if (ev->type() == QEvent::KeyPress)
  {
    QKeyEvent *keyevent = static_cast<QKeyEvent*>(ev);
    if (keyevent->modifiers() == Qt::ControlModifier && keyevent->key() == Qt::Key_Tab)
    {
      if (cfg.preferences.mruTabOrder)
      {
        ctrlTabPressed();
        return true;
      }
      return false;
    }
  }

  if ( obj == translateLine )
  {
    if ( ev->type() == QEvent::KeyPress )
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( cfg.preferences.searchInDock )
      {
        if ( keyEvent->matches( QKeySequence::MoveToNextLine ) && wordList->count() )
        {
          wordList->setFocus( Qt::ShortcutFocusReason );
          wordList->setCurrentRow( 0, QItemSelectionModel::ClearAndSelect );
          return true;
        }
      }
    }

    if ( ev->type() == QEvent::FocusIn ) {
      QFocusEvent * focusEvent = static_cast< QFocusEvent * >( ev );

      // select all on mouse click
      if ( focusEvent->reason() == Qt::MouseFocusReason ) {
        QTimer::singleShot(0, this, SLOT(focusTranslateLine()));
      }
      return false;
    }

    if ( ev->type() == QEvent::Resize ) {
      QResizeEvent * resizeEvent = static_cast< QResizeEvent * >( ev );
      groupList->setFixedHeight( resizeEvent->size().height() );
      return false;
    }
  }
  else
  if ( obj == wordList )
  {
    if ( ev->type() == QEvent::KeyPress )
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( keyEvent->matches( QKeySequence::MoveToPreviousLine ) &&
           !wordList->currentRow() )
      {
        wordList->setCurrentRow( 0, QItemSelectionModel::Clear );
        translateLine->setFocus( Qt::ShortcutFocusReason );
        return true;
      }

      if ( keyEvent->matches( QKeySequence::InsertParagraphSeparator ) &&
           wordList->selectedItems().size() )
      {
        if ( cfg.preferences.searchInDock )
        {
          if ( ui.searchPane->isFloating() )
            activateWindow();
        }

        getCurrentArticleView()->focus();

        return cfg.preferences.searchInDock;
      }

      // Handle typing events used to initiate new lookups
      // TODO: refactor to eliminate duplication (see below)

      if ( keyEvent->modifiers() &
           ( Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier ) )
        return false; // A non-typing modifier is pressed

      if ( keyEvent->key() == Qt::Key_Space ||
           keyEvent->key() == Qt::Key_Backspace ||
           keyEvent->key() == Qt::Key_Tab ||
           keyEvent->key() == Qt::Key_Backtab )
        return false; // Those key have other uses than to start typing
                      // or don't make sense

      QString text = keyEvent->text();

      if ( text.size() )
      {
        typingEvent( text );
        return true;
      }
    }
  }
  else
  if (obj == ui.dictsList) {
    if ( ev->type() == QEvent::KeyPress )
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      // Handle typing events used to initiate new lookups
      // TODO: refactor to eliminate duplication (see above)

      if ( keyEvent->modifiers() &
           ( Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier ) )
        return false; // A non-typing modifier is pressed

      if ( keyEvent->key() == Qt::Key_Space ||
           keyEvent->key() == Qt::Key_Backspace ||
           keyEvent->key() == Qt::Key_Tab ||
           keyEvent->key() == Qt::Key_Backtab )
        return false; // Those key have other uses than to start typing
                      // or don't make sense

      QString text = keyEvent->text();

      if ( text.size() )
      {
        typingEvent( text );
        return true;
      }
    }
  }
    return QMainWindow::eventFilter( obj, ev );

  return false;
}

void MainWindow::wordListItemActivated( QListWidgetItem * item )
{
  if( wordListSelChanged )
    wordListSelChanged = false;
  else {
    // TODO: code duplication with translateInputFinished!

    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if ( mods & (Qt::ControlModifier | Qt::ShiftModifier) )
      addNewTab();

    showTranslationFor( item->text() );
    getCurrentArticleView()->focus();
  }
}

void MainWindow::wordListSelectionChanged()
{
  QList< QListWidgetItem * > selected = wordList->selectedItems();

  if ( selected.size() )
  {
    wordListSelChanged = true;
    showTranslationFor( selected.front()->text() );
  }
}

void MainWindow::dictsListItemActivated( QListWidgetItem * item )
{
  QString id = item->data( Qt::UserRole ).toString();
  getCurrentArticleView()->jumpToDictionary( id );
}

void MainWindow::dictsListSelectionChanged()
{
  QList< QListWidgetItem * > selected = ui.dictsList->selectedItems();

  if ( selected.size() )
    dictsListItemActivated( selected.front() );
}

void MainWindow::openLinkInNewTab( QUrl const & url,
                                   QUrl const & referrer,
                                   QString const & fromArticle,
                                   ArticleView::Contexts const & contexts )
{
  createNewTab( !cfg.preferences.newTabsOpenInBackground, "" )->
      openLink( url, referrer, fromArticle, contexts );
}

void MainWindow::showDefinitionInNewTab( QString const & word,
                                         unsigned group,
                                         QString const & fromArticle,
                                         ArticleView::Contexts const & contexts )
{
  createNewTab( !cfg.preferences.newTabsOpenInBackground, word )->
      showDefinition( word, group, fromArticle, contexts );
}

void MainWindow::activeArticleChanged( QString const & id )
{
  // select the row with the corresponding id
  for (int i = 0; i < ui.dictsList->count(); ++i) {
    QListWidgetItem * w = ui.dictsList->item( i );
    QString dictId = w->data( Qt::UserRole ).toString();

    if ( dictId == id )
    {
      // update the current row, but only if necessary
      if ( i != ui.dictsList->currentRow() )
      {
        ui.dictsList->setCurrentRow(i);
      }
      return;
    }
  }
}

void MainWindow::typingEvent( QString const & t )
{
  if ( t == "\n" || t == "\r" )
  {
    if( translateLine->isEnabled() )
      focusTranslateLine();
  }
  else
  {
    if ( ( cfg.preferences.searchInDock && ui.searchPane->isFloating() ) || ui.dictsPane->isFloating() )
      ui.searchPane->activateWindow();

    if( translateLine->isEnabled() )
    {
      translateLine->setFocus();
      if ( cfg.preferences.searchInDock )
        translateLine->setText( t );
      else
        translateBox->setText( t, true );
      translateLine->setCursorPosition( t.size() );
    }
  }
}

void MainWindow::mutedDictionariesChanged()
{
  if ( dictionaryBar.toggleViewAction()->isChecked() )
    applyMutedDictionariesState();
}

void MainWindow::showHistoryItem( QString const & word )
{
  // qDebug() << "Showing history item" << word;

  history.enableAdd( false );

  if ( cfg.preferences.searchInDock )
    translateLine->setText( word );
  else
    translateBox->setText( word, false );

  showTranslationFor( word );

  history.enableAdd( cfg.preferences.storeHistory );
}

void MainWindow::showTranslationFor( QString const & inWord,
                                     unsigned inGroup )
{
  ArticleView *view = getCurrentArticleView();

  navPronounce->setEnabled( false );

  unsigned group = inGroup ? inGroup :
                   ( groupInstances.empty() ? 0 :
                        groupInstances[ groupList->currentIndex() ].id );

  view->showDefinition( inWord, group );

  updatePronounceAvailability();
  updateFoundInDictsList();

  // Add to history

  addWordToHistory( inWord );

  updateBackForwardButtons();

  #if 0
  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  req.addQueryItem( "word", inWord );
  req.addQueryItem( "group",
                    cfg.groups.empty() ? "" :
                      groupInstances[ groupList->currentIndex() ].name );

  ui.definition->load( req );

  return;
#endif

  #if 0
  wstring word = inWord.trimmed().toStdWString();

  // Where to look?

  vector< sptr< Dictionary::Class > > const & activeDicts = getActiveDicts();

  // Accumulate main forms

  vector< wstring > alts;

  {
    set< wstring > altsSet;

    for( unsigned x = 0; x < activeDicts.size(); ++x )
    {
      vector< wstring > found = activeDicts[ x ]->findHeadwordsForSynonym( word );

      altsSet.insert( found.begin(), found.end() );
    }

    alts.insert( alts.begin(), altsSet.begin(), altsSet.end() );
  }

  for( unsigned x = 0; x < alts.size(); ++x )
  {
    DPRINTF( "Alt: %ls\n", alts[ x ].c_str() );
  }


  string result =
    "<html><head>"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";

  QFile cssFile( Config::getUserCssFileName() );

  if ( cssFile.open( QFile::ReadOnly ) )
  {
    result += "<style type=\"text/css\">\n";
    result += cssFile.readAll().data();
    result += "</style>\n";
  }

  result += "</head><body>";

  for( unsigned x = 0; x < activeDicts.size(); ++x )
  {
    try
    {
      string body = activeDicts[ x ]->getArticle( word, alts );

      DPRINTF( "From %s: %s\n", activeDicts[ x ]->getName().c_str(), body.c_str() );

      result += "<div class=\"gddictname\">From " + activeDicts[ x ]->getName() + "</div>" + body;
    }
    catch( Dictionary::exNoSuchWord & )
    {
      continue;
    }
  }

  result += "</body></html>";

  ArticleMaker am( dictionaries, groupInstances );

  string result = am.makeDefinitionFor( inWord, "En" );

  ui.definition->setContent( result.c_str(), QString() );

  #endif

  //ui.tabWidget->setTabText( ui.tabWidget->indexOf(ui.tab), inWord.trimmed() );
}

#ifdef Q_WS_X11
void MainWindow::toggleMainWindow( bool onlyShow, bool byIconClick )
#else
void MainWindow::toggleMainWindow( bool onlyShow )
#endif
{
  bool shown = false;

  if ( !isVisible() )
  {
    show();
    qApp->setActiveWindow( this );
    activateWindow();
    raise();
    shown = true;
  }
  else
  if ( isMinimized() )
  {
    if( wasMaximized )
      showMaximized();
    else
      showNormal();
    activateWindow();
    raise();
    shown = true;
  }
  else
  if ( !isActiveWindow() )
  {
    activateWindow();
    raise();
    shown = true;
  }
  else
  if ( !onlyShow )
  {
    if (cfg.preferences.enableTrayIcon)
      hide();
    else
      showMinimized();
  }

  if ( shown )
  {
    focusTranslateLine();
#ifdef Q_WS_X11
    Window wh = 0;
    int rev = 0;
    XGetInputFocus( QX11Info::display(), &wh, &rev );
    if( wh != translateLine->internalWinId() && !byIconClick )
    {
        QPoint p( 1, 1 );
        mapToGlobal( p );
        XEvent event;
        memset( &event, 0, sizeof( event) );
        event.type = ButtonPress;
        event.xbutton.x = 1;
        event.xbutton.y = 1;
        event.xbutton.x_root = p.x();
        event.xbutton.y_root = p.y();
        event.xbutton.window = internalWinId();
        event.xbutton.root = QX11Info::appRootWindow( QX11Info::appScreen() );
        event.xbutton.state = Button1Mask;
        event.xbutton.button = Button1;
        event.xbutton.same_screen = true;
        event.xbutton.time = CurrentTime;

        XSendEvent( QX11Info::display(), internalWinId(), true, 0xfff, &event );
        XFlush( QX11Info::display() );
        event.type = ButtonRelease;
        XSendEvent( QX11Info::display(), internalWinId(), true, 0xfff, &event );
        XFlush( QX11Info::display() );
    }
#endif
  }
}

void MainWindow::installHotKeys()
{
  hotkeyWrapper.reset(); // Remove the old one

  if ( cfg.preferences.enableMainWindowHotkey ||
       cfg.preferences.enableClipboardHotkey )
  {
    try
    {
      hotkeyWrapper = new HotkeyWrapper( this );
    }
    catch( HotkeyWrapper::exInit & )
    {
      QMessageBox::critical( this, "GoldenDict",
        tr( "Failed to initialize hotkeys monitoring mechanism.<br>"
            "Make sure your XServer has RECORD extension turned on." ) );

      return;
    }

    if ( cfg.preferences.enableMainWindowHotkey )
      hotkeyWrapper->setGlobalKey( cfg.preferences.mainWindowHotkey.key1,
                                   cfg.preferences.mainWindowHotkey.key2,
                                   cfg.preferences.mainWindowHotkey.modifiers,
                                   0 );

    if ( cfg.preferences.enableClipboardHotkey && scanPopup.get() )
    {
      hotkeyWrapper->setGlobalKey( cfg.preferences.clipboardHotkey.key1,
                                   cfg.preferences.clipboardHotkey.key2,
                                   cfg.preferences.clipboardHotkey.modifiers,
                                   1 );
    }

    connect( hotkeyWrapper.get(), SIGNAL( hotkeyActivated( int ) ),
             this, SLOT( hotKeyActivated( int ) ) );
  }
}

void MainWindow::hotKeyActivated( int hk )
{
  if ( !hk )
    toggleMainWindow();
  else
  if ( scanPopup.get() )
    scanPopup->translateWordFromClipboard();
}

void MainWindow::prepareNewReleaseChecks()
{
  if ( cfg.preferences.checkForNewReleases )
  {
    QDateTime now = QDateTime::currentDateTime();

    if ( !cfg.timeForNewReleaseCheck.isValid() ||
         now.daysTo( cfg.timeForNewReleaseCheck ) > 2 )
    {
      // The date is invalid, or the check is set to happen more than 2 days
      // in the future -- fix that.
      cfg.timeForNewReleaseCheck = now.addDays( 2 );
    }

    int secsToCheck = now.secsTo( cfg.timeForNewReleaseCheck );

    if ( secsToCheck < 1 )
      secsToCheck = 1;

    newReleaseCheckTimer.setSingleShot( true );
    newReleaseCheckTimer.start( secsToCheck * 1000 );
  }
  else
    newReleaseCheckTimer.stop(); // In case it was started before
}

void MainWindow::checkForNewRelease()
{
  latestReleaseReply.reset();

  QNetworkRequest req(
    QUrl( "http://goldendict.org/latest_release.php?current="
          PROGRAM_VERSION "&platform="
#ifdef Q_WS_X11
          "x11"
#endif
#ifdef Q_WS_MAC
          "mac"
#endif
#ifdef Q_WS_QWS
          "qws"
#endif
#ifdef Q_WS_WIN
          "win"
#endif
          ) );

  latestReleaseReply = articleNetMgr.get( req );

  connect( latestReleaseReply.get(), SIGNAL( finished() ),
           this, SLOT( latestReleaseReplyReady() ), Qt::QueuedConnection );
}

void MainWindow::latestReleaseReplyReady()
{
  if ( !latestReleaseReply.get() )
    return; // Some stray signal

  bool success = false;
  QString latestVersion, downloadUrl;

  // See if we succeeded

  if ( latestReleaseReply->error() == QNetworkReply::NoError )
  {
    QString latestReleaseInfo = QString::fromUtf8( latestReleaseReply->readLine() ).trimmed();
    QStringList parts = latestReleaseInfo.split( ' ' );
    if ( parts.size() == 2 )
    {
      latestVersion = parts[ 0 ];
      downloadUrl = parts[ 1 ];
      success = true;
    }
  }

  latestReleaseReply.reset();

  if ( !success )
  {
    // Failed -- reschedule to check in two hours
    newReleaseCheckTimer.start( 2 * 60 * 60 * 1000 );

    DPRINTF( "Failed to check program version, retry in two hours\n" );
  }
  else
  {
    // Success -- reschedule for a normal check and save config
    cfg.timeForNewReleaseCheck = QDateTime();

    prepareNewReleaseChecks();

    Config::save( cfg );

    DPRINTF( "Program version's check successful, current version is %ls\n",
            latestVersion.toStdWString().c_str() );
  }

  if ( success && latestVersion > PROGRAM_VERSION && latestVersion != cfg.skippedRelease )
  {
    QMessageBox msg( QMessageBox::Information,
                     tr( "New Release Available" ),
                     tr( "Version <b>%1</b> of GoldenDict is now available for download.<br>"
                         "Click <b>Download</b> to get to the download page." ).arg( latestVersion ),
                     QMessageBox::NoButton,
                     this );

    QPushButton * dload = msg.addButton( tr( "Download" ), QMessageBox::AcceptRole );
    QPushButton * skip = msg.addButton( tr( "Skip This Release" ), QMessageBox::DestructiveRole );
    msg.addButton( QMessageBox::Cancel );

    msg.exec();

    if ( msg.clickedButton() == dload )
      QDesktopServices::openUrl( QUrl( downloadUrl ) );
    else
    if ( msg.clickedButton() == skip )
    {
      cfg.skippedRelease = latestVersion;
      Config::save( cfg );
    }
  }
}

void MainWindow::trayIconActivated( QSystemTrayIcon::ActivationReason r )
{
  switch(r) {
    case QSystemTrayIcon::Trigger:
      // Left click toggles the visibility of main window
#ifdef Q_WS_X11
      toggleMainWindow( false, true );
#else
      toggleMainWindow();
#endif
      break;

    case QSystemTrayIcon::MiddleClick:
      // Middle mouse click on Tray translates selection
      // it is functional like as stardict
      if ( scanPopup.get() ) {
        scanPopup->translateWordFromSelection();
      }
      break;
    default:
      break;

  }
}

void MainWindow::scanEnableToggled( bool on )
{
  if ( !cfg.preferences.enableScanPopup )
    return;

  if ( scanPopup )
  {
    if ( on )
    {
      scanPopup->enableScanning();
#ifdef Q_OS_MACX
      if( !AXAPIEnabled() )
          mainStatusBar->showMessage( tr( "Accessibility API is not enabled" ), 10000,
                                          QPixmap( ":/icons/error.png" ) );
#endif
    }
    else
      scanPopup->disableScanning();
  }

  updateTrayIcon();
}

void MainWindow::showMainWindow()
{
  toggleMainWindow( true );
}

void MainWindow::visitHomepage()
{
  QDesktopServices::openUrl( QUrl( "http://goldendict.org/" ) );
}

void MainWindow::openConfigFolder()
{
  QDesktopServices::openUrl( QUrl::fromLocalFile( Config::getConfigDir() ) );
}

void MainWindow::visitForum()
{
  QDesktopServices::openUrl( QUrl( "http://goldendict.org/forum/" ) );
}

void MainWindow::showAbout()
{
  About about( this );

  about.show();
  about.exec();
}

void MainWindow::showDictBarNamesTriggered()
{
  bool show = showDictBarNamesAction.isChecked();

  dictionaryBar.setToolButtonStyle( show ? Qt::ToolButtonTextBesideIcon :
                                           Qt::ToolButtonIconOnly );
  cfg.showingDictBarNames = show;
}

void MainWindow::useSmallIconsInToolbarsTriggered()
{
  bool useSmallIcons = useSmallIconsInToolbarsAction.isChecked();

  int extent = useSmallIcons ? QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize) :
                               QApplication::style()->pixelMetric(QStyle::PM_ToolBarIconSize);

  navToolbar->setIconSize( QSize( extent, extent ) );

  // additional fix for #176
  menuButton->setIconSize( QSize( extent, extent ) );

  updateDictionaryBar();

  cfg.usingSmallIconsInToolbars = useSmallIcons;

  if( scanPopup.get() )
    scanPopup->setDictionaryIconSize();
}

void MainWindow::toggleMenuBarTriggered(bool announce)
{
  cfg.preferences.hideMenubar = !toggleMenuBarAction.isChecked();

  if ( announce )
  {
    if ( cfg.preferences.hideMenubar )
    {
      mainStatusBar->showMessage(
            tr( "You have chosen to hide a menubar. Use %1 to show it back." )
            .arg( QString( "<b>%1</b>" ) ).arg( tr( "Ctrl+M" ) ),
            10000,
            QPixmap( ":/icons/warning.png" ) );
    }
    else
    {
      mainStatusBar->clearMessage();
    }
  }

  // Obtain from the menubar all the actions with shortcuts
  // and either add them to the main window or remove them,
  // depending on the menubar state.

  QList<QMenu *> allMenus = menuBar()->findChildren<QMenu *>();
  QListIterator<QMenu *> menuIter( allMenus );
  while( menuIter.hasNext() )
  {
    QMenu * menu = menuIter.next();
    QList<QAction *> allMenuActions = menu->actions();
    QListIterator<QAction *> actionsIter( allMenuActions );
    while( actionsIter.hasNext() )
    {
      QAction * action = actionsIter.next();
      if ( !action->shortcut().isEmpty() )
      {
        if ( cfg.preferences.hideMenubar )
        {
          // add all menubar actions to the main window,
          // before we hide the menubar
          addAction( action );
        }
        else
        {
          // remove all menubar actions from the main window
          removeAction( action );
        }
      }
    }
  }

  menuBar()->setVisible( !cfg.preferences.hideMenubar );
  beforeOptionsSeparator->setVisible( cfg.preferences.hideMenubar);
  menuButtonAction->setVisible( cfg.preferences.hideMenubar );
}

void MainWindow::on_clearHistory_triggered()
{
  history.clear();
  history.save();
}

void MainWindow::on_newTab_triggered()
{
  addNewTab();
}

void MainWindow::setAutostart(bool autostart)
{
#ifdef Q_OS_WIN32
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    if (autostart) {
        QString app_fname = QString("\"%1\"").arg( QCoreApplication::applicationFilePath() );
        app_fname.replace("/", "\\");
        reg.setValue(QCoreApplication::applicationName(), app_fname);
    }
    else {
        reg.remove(QCoreApplication::applicationName());
    }
    reg.sync();
#else
    // this is for KDE
    QString app_fname = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
    QString lnk(QDir::homePath()+"/.kde/Autostart/"+app_fname);
    if (autostart) {
        QFile f(QCoreApplication::applicationFilePath());
        f.link(lnk);
    } else {
        QFile::remove(lnk);
    }
#endif
}

void MainWindow::on_actionCloseToTray_triggered()
{
  toggleMainWindow( !cfg.preferences.enableTrayIcon );
}

void MainWindow::on_pageSetup_triggered()
{
  if ( getPrinter().isValid() )
  {
    QPageSetupDialog dialog( &getPrinter(), this );

    dialog.exec();
  }
  else
    QMessageBox::critical( this, tr( "Page Setup" ),
                           tr( "No printer is available. Please install one first." ) );
}

void MainWindow::on_printPreview_triggered()
{
  QPrintPreviewDialog dialog( &getPrinter(), this );

  connect( &dialog, SIGNAL( paintRequested( QPrinter * ) ),
           this, SLOT( printPreviewPaintRequested( QPrinter * ) ) );

  dialog.showMaximized();
  dialog.exec();
}

void MainWindow::on_print_triggered()
{
  QPrintDialog dialog( &getPrinter(), this );

  dialog.setWindowTitle( tr( "Print Article") );

  if ( dialog.exec() != QDialog::Accepted )
   return;

  ArticleView *view = getCurrentArticleView();

  view->print( &getPrinter() );
}

void MainWindow::printPreviewPaintRequested( QPrinter * printer )
{
  ArticleView *view = getCurrentArticleView();

  view->print( printer );
}

void MainWindow::on_saveArticle_triggered()
{
  ArticleView *view = getCurrentArticleView();

  QString fileName = view->getTitle() + ".html";
  QString savePath;

  if ( cfg.articleSavePath.isEmpty() )
    savePath = QDir::homePath();
  else
  {
    savePath = QDir::fromNativeSeparators( cfg.articleSavePath );
    if ( !QDir( savePath ).exists() )
      savePath = QDir::homePath();
  }

  fileName = savePath + "/" + fileName;
  fileName = QFileDialog::getSaveFileName( this, tr(  "Save Article As" ),
                                                   fileName,
                                                   tr( "Html files (*.html *.htm)" ) );

  if ( !fileName.isEmpty() )
  {

    QFile file( fileName );

    if ( !file.open( QIODevice::WriteOnly ) )
    {
      QMessageBox::critical( this, tr( "Error" ),
                             tr( "Can't save article: %1" ).arg( file.errorString() ) );
    }
    else
    {
      file.write( view->toHtml().toUtf8() );
      cfg.articleSavePath = QDir::toNativeSeparators( QFileInfo( fileName ).absoluteDir().absolutePath() );
    }
  }
}

void MainWindow::on_rescanFiles_triggered()
{
  hotkeyWrapper.reset(); // No hotkeys while we're editing dictionaries
  scanPopup.reset(); // No scan popup either. No one should use dictionaries.

  groupInstances.clear(); // Release all the dictionaries they hold
  dictionaries.clear();
  dictionariesUnmuted.clear();
  dictionaryBar.setDictionaries( dictionaries );

  loadDictionaries( this, true, cfg, dictionaries, dictNetMgr );

  updateGroupList();

  makeScanPopup();
  installHotKeys();
}

void MainWindow::on_alwaysOnTop_triggered( bool checked )
{
    cfg.preferences.alwaysOnTop = checked;

    bool wasVisible = isVisible();

    Qt::WindowFlags flags = this->windowFlags();
    if (checked)
    {
        setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
        mainStatusBar->showMessage(
              tr( "The main window is set to be always on top." ),
              10000,
              QPixmap( ":/icons/warning.png" ) );
    }
    else
    {
        setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
        mainStatusBar->clearMessage();
    }

    if ( wasVisible )
    {
      show();
    }

    installHotKeys();
}

void MainWindow::zoomin()
{
  cfg.preferences.zoomFactor += 0.1;
  applyZoomFactor();
}

void MainWindow::zoomout()
{
  cfg.preferences.zoomFactor -= 0.1;
  applyZoomFactor();
}

void MainWindow::unzoom()
{
  cfg.preferences.zoomFactor = 1;
  applyZoomFactor();
}

void MainWindow::applyZoomFactor()
{
  if ( cfg.preferences.zoomFactor >= 5 )
    cfg.preferences.zoomFactor = 5;
  else if ( cfg.preferences.zoomFactor <= 0.1 )
    cfg.preferences.zoomFactor = 0.1;

  zoomIn->setEnabled( cfg.preferences.zoomFactor < 5 );
  zoomOut->setEnabled( cfg.preferences.zoomFactor > 0.1 );
  zoomBase->setEnabled( cfg.preferences.zoomFactor != 1.0 );

  for ( int i = 0; i < ui.tabWidget->count(); i++ )
  {
    ArticleView & view =
      dynamic_cast< ArticleView & >( *( ui.tabWidget->widget(i) ) );
    view.setZoomFactor( cfg.preferences.zoomFactor );
  }

  if ( scanPopup.get() )
    scanPopup->applyZoomFactor();
}

void MainWindow::doWordsZoomIn()
{
  ++cfg.preferences.wordsZoomLevel;

  applyWordsZoomLevel();
}

void MainWindow::doWordsZoomOut()
{
  --cfg.preferences.wordsZoomLevel;

  applyWordsZoomLevel();
}

void MainWindow::doWordsZoomBase()
{
  cfg.preferences.wordsZoomLevel = 0;

  applyWordsZoomLevel();
}

void MainWindow::applyWordsZoomLevel()
{
  QFont font( wordListDefaultFont );

  int ps = font.pointSize();

  if ( cfg.preferences.wordsZoomLevel != 0 )
  {
    ps += cfg.preferences.wordsZoomLevel;

    if ( ps < 1 )
      ps = 1;

    font.setPointSize( ps );
  }

  if ( wordList->font().pointSize() != ps )
    wordList->setFont( font );

  font = translateLineDefaultFont;

  ps = font.pointSize();

  if ( cfg.preferences.wordsZoomLevel != 0 )
  {
    ps += cfg.preferences.wordsZoomLevel;

    if ( ps < 1 )
      ps = 1;

    font.setPointSize( ps );
  }

  if ( translateLine->font().pointSize() != ps )
    translateLine->setFont( font );

  groupList->setFont(font);

  wordsZoomBase->setEnabled( cfg.preferences.wordsZoomLevel != 0 );
  // groupList->setFixedHeight(translateLine->height());
  groupList->parentWidget()->layout()->activate();
}

void MainWindow::messageFromAnotherInstanceReceived( QString const & message )
{
  if ( message == "bringToFront" )
  {
    toggleMainWindow( true );
    return;
  }
  if( message.left( 15 ) == "translateWord: " )
  {
    if( scanPopup.get() )
      scanPopup->translateWord( message.mid( 15 ) );
    else
      wordReceived( message.mid( 15 ) );
  }
  else
    qWarning() << "Unknown message received from another instance: " << message;
}

ArticleView * MainWindow::getCurrentArticleView()
{
  if ( QWidget * cw = ui.tabWidget->currentWidget() )
  {
    return &( dynamic_cast< ArticleView & >( *( cw ) ) );
  }
  return 0;
}

void MainWindow::wordReceived( const QString & word)
{
    toggleMainWindow( true );
    translateLine->setText( word );
    translateInputFinished();
}

void MainWindow::updateHistoryMenu()
{
  if ( ui.historyPane->toggleViewAction()->isChecked() )
  {
    ui.showHideHistory->setText( tr( "&Hide" ) );
  }
  else
  {
    ui.showHideHistory->setText( tr( "&Show" ) );
  }
}

void MainWindow::on_showHideHistory_triggered()
{
  ui.historyPane->toggleViewAction()->trigger();
  ui.historyPane->raise(); // useful when the Pane is tabbed.
}

void MainWindow::on_exportHistory_triggered()
{
    QString exportPath;
    if( cfg.historyExportPath.isEmpty() )
        exportPath = QDir::homePath();
    else
    {
        exportPath = QDir::fromNativeSeparators( cfg.historyExportPath );
        if( !QDir( exportPath ).exists() )
            exportPath = QDir::homePath();
    }

    QString fileName = QFileDialog::getSaveFileName( this, tr( "Export history to file" ),
                                                     exportPath,
                                                     tr( "Text files (*.txt);;All files (*.*)" ) );
    if( fileName.size() == 0)
        return;

    cfg.historyExportPath = QDir::toNativeSeparators( QFileInfo( fileName ).absoluteDir().absolutePath() );
    QFile file( fileName );

    for(;;)
    {
        if ( !file.open( QFile::WriteOnly | QIODevice::Text ) )
          break;

        // Write UTF-8 BOM
        QByteArray line;
        line.append( 0xEF ).append( 0xBB ).append( 0xBF );
        if ( file.write( line ) != line.size() )
          break;

        // Write history
        QList< History::Item > const & items = history.getItems();

        QList< History::Item >::const_iterator i;
        for( i = items.constBegin(); i != items.constEnd(); ++i )
        {
          line = i->word.toUtf8();

          line.replace( '\n', ' ' );
          line.replace( '\r', ' ' );

          line += "\n";

          if ( file.write( line ) != line.size() )
            break;
        }

        if( i != items.constEnd() )
          break;

        file.close();
        mainStatusBar->showMessage( tr( "History export complete" ), 5000 );
        return;
    }
    QString errStr = QString( tr( "Export error: " ) ) + file.errorString();
    file.close();
    mainStatusBar->showMessage( errStr, 10000, QPixmap( ":/icons/error.png" ) );
}

// TODO: consider moving parts of this method into History class.
void MainWindow::on_importHistory_triggered()
{
    QString importPath;
    if( cfg.historyExportPath.isEmpty() )
        importPath = QDir::homePath();
    else
    {
        importPath = QDir::fromNativeSeparators( cfg.historyExportPath );
        if( !QDir( importPath ).exists() )
            importPath = QDir::homePath();
    }

    QString fileName = QFileDialog::getOpenFileName( this, tr( "Import history from file" ),
                                                     importPath,
                                                     tr( "Text files (*.txt);;All files (*.*)" ) );
    if( fileName.size() == 0)
        return;

    QFileInfo fileInfo( fileName );
    cfg.historyExportPath = QDir::toNativeSeparators( fileInfo.absoluteDir().absolutePath() );
    QString errStr;
    QFile file( fileName );

    for(;;)
    {
        if ( !file.open( QFile::ReadOnly | QIODevice::Text ) )
          break;

        QTextStream fileStream( & file );
        QString itemStr, trimmedStr;
        QList< QString > itemList;

        history.clear();

        do
        {
            itemStr = fileStream.readLine();
            if( fileStream.status() >= QTextStream::ReadCorruptData )
                break;

            trimmedStr = itemStr.trimmed();
            if( trimmedStr.isEmpty() )
                continue;

            if( (unsigned)trimmedStr.size() <= history.getMaxItemLength( ) )
                itemList.prepend( trimmedStr );

        } while( !fileStream.atEnd() && itemList.size() < (int)history.getMaxSize() );

        history.enableAdd( true );

        for( QList< QString >::const_iterator i = itemList.constBegin(); i != itemList.constEnd(); ++i )
            history.addItem( History::Item( 1, *i ) );

        history.enableAdd( cfg.preferences.storeHistory );

        if( file.error() != QFile::NoError )
            break;

        if( fileStream.status() >= QTextStream::ReadCorruptData )
        {
            errStr = QString ( tr( "Import error: invalid data in file" ) );
            mainStatusBar->showMessage( errStr, 10000, QPixmap( ":/icons/error.png" ) );
        }
        else
            mainStatusBar->showMessage( tr( "History import complete" ), 5000 );
        return;
    }
    errStr = QString( tr( "Import error: " ) ) + file.errorString();
    file.close();
    mainStatusBar->showMessage( errStr, 10000, QPixmap( ":/icons/error.png" ) );
}

void MainWindow::fillWordListFromHistory()
{
    ui.wordList->setUpdatesEnabled( false );
    ui.wordList->clear();

    QList< History::Item > const & items = history.getItems();
    for( int x = 0; x < items.size(); ++x )
    {
      History::Item const * i = &items[ x ];
      QListWidgetItem * s = new QListWidgetItem( i->word, ui.wordList );
      if (s->text().at(0).direction() == QChar::DirR)
          s->setTextAlignment(Qt::AlignRight);
      if (s->text().at(0).direction() == QChar::DirL)
          s->setTextAlignment(Qt::AlignLeft);
      ui.wordList->addItem( s );
    }

    ui.wordList->setUpdatesEnabled( true );
}

void MainWindow::focusWordList()
{
    if( ui.wordList->count() > 0 )
        ui.wordList->setFocus();
}

void MainWindow::addWordToHistory( const QString & word )
{
  history.addItem( History::Item( 1, word.trimmed() ) );
}

void MainWindow::forceAddWordToHistory( const QString & word )
{
    history.enableAdd( true );
    history.addItem( History::Item( 1, word.trimmed() ) );
    history.enableAdd( cfg.preferences.storeHistory );
}

void MainWindow::setExpandMode( bool expand )
{
  articleMaker.setExpandOptionalParts( expand );
}

void MainWindow::switchExpandOptionalPartsMode()
{
  ArticleView * view = getCurrentArticleView();
  if( view )
    view->switchExpandOptionalParts();
}

void MainWindow::foundDictsPaneClicked( QListWidgetItem * item )
{
  if ( QApplication::keyboardModifiers() &
       ( Qt::ControlModifier | Qt::ShiftModifier ) )
  {
    QString id = item->data( Qt::UserRole ).toString();
    emit clickOnDictPane( id );
  }
}

void MainWindow::showDictionaryInfo( const QString & id )
{
  for( unsigned x = 0; x < dictionaries.size(); x++ )
  {
    if( dictionaries[ x ]->getId() == id.toUtf8().data() )
    {
      DictInfo infoMsg( cfg, this );
      infoMsg.showInfo( dictionaries[ x ] );
      infoMsg.exec();
      break;
    }
  }
}

void MainWindow::foundDictsContextMenuRequested( const QPoint &pos )
{
  QListWidgetItem *item = ui.dictsList->itemAt( pos );
  if( item )
  {
    QString id = item->data( Qt::UserRole ).toString();
    QString dictFilename;

    if( !cfg.editDictionaryCommandLine.isEmpty() )
    {
      for( unsigned i = 0; i < dictionaries.size(); i++ )
      {
        if( id.compare( dictionaries[ i ]->getId().c_str() ) == 0 )
        {
          dictFilename = dictionaries[ i ]->getMainFilename();
          break;
        }
      }
    }

    if( cfg.editDictionaryCommandLine.isEmpty() || dictFilename.isEmpty() )
    {
      scanPopup.get()->blockSignals( true );
      showDictionaryInfo( id );
      scanPopup.get()->blockSignals( false );
    }
    else
    {
      QMenu menu( ui.dictsList );
      QAction * infoAction = menu.addAction( tr( "Dictionary info" ) );
      QAction * editAction = menu.addAction( tr( "Edit dictionary" ) );

      QAction * result = menu.exec( ui.dictsList->mapToGlobal( pos ) );

      if( result && result == infoAction )
      {
        scanPopup.get()->blockSignals( true );
        showDictionaryInfo( id );
        scanPopup.get()->blockSignals( false );
      }
      else
      if( result && result == editAction )
      {
        QString command( cfg.editDictionaryCommandLine );
        command.replace( "%GDDICT%", "\"" + dictFilename + "\"" );
        if( !QProcess::startDetached( command ) )
          QApplication::beep();
      }
    }
  }
}

void MainWindow::sendWordToInputLine( const QString & word )
{
  translateLine->clear();
  translateLine->setText( word );
}

void MainWindow::storeResourceSavePath( const QString & newPath )
{
  cfg.resourceSavePath = newPath;
}

#ifdef Q_OS_WIN32

bool MainWindow::winEvent( MSG * message, long * result )
{
  if( message->message != gdAskMessage )
    return false;
  *result = 0;
  ArticleView * view = getCurrentArticleView();
  if( !view )
    return true;

  LPGDDataStruct lpdata = ( LPGDDataStruct ) message->lParam;

  QString str = view->wordAtPoint( lpdata->Pt.x, lpdata->Pt.y );

  memset( lpdata->cwData, 0, lpdata->dwMaxLength * sizeof( WCHAR ) );
  if( str.isRightToLeft() )
  {
    gd::wstring wstr = gd::toWString( str );
    wstr.assign( wstr.rbegin(), wstr.rend() );
    str = gd::toQString( wstr );
  }
  str.truncate( lpdata->dwMaxLength - 1 );
  str.toWCharArray( lpdata->cwData );

  *result = 1;
  return true;
}

bool MainWindow::isGoldenDictWindow( HWND hwnd )
{
    return hwnd == (HWND)winId();
}

#endif
