/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mainwindow.hh"
#include "editdictionaries.hh"
#include "loaddictionaries.hh"
#include "preferences.hh"
#include "about.hh"
#include <limits.h>
#include <QDir>
#include <QMessageBox>
#include <QIcon>
#include <QToolBar>
#include <QCloseEvent>
#include <QDesktopServices>
#include <set>
#include <map>

using std::set;
using std::wstring;
using std::map;
using std::pair;

MainWindow::MainWindow( Config::Class & cfg_ ):
  commitDataCompleted( false ),
  trayIcon( 0 ),
  groupLabel( &searchPaneTitleBar ),
  groupList( &searchPaneTitleBar ),
  foundInDictsLabel( &dictsPaneTitleBar ),
  escAction( this ),
  focusTranslateLineAction( this ),
  addTabAction( this ),
  closeCurrentTabAction( this ),
  closeAllTabAction( this ),
  closeRestTabAction( this ),
  switchToNextTabAction( this ),
  switchToPrevTabAction( this ),
  showDictBarNamesAction( tr( "Show Names in Dictionary Bar" ), this ),
  useSmallIconsInToolbarsAction( tr( "Show Small Icons in Toolbars" ), this ),
  trayIconMenu( this ),
  addTab( this ),
  cfg( cfg_ ),
  history( History::Load() ),
  dictionaryBar( this, cfg.mutedDictionaries, configEvents ),
  articleMaker( dictionaries, groupInstances, cfg.preferences.displayStyle ),
  articleNetMgr( this, dictionaries, articleMaker,
                 cfg.preferences.disallowContentFromOtherSites ),
  dictNetMgr( this ),
  wordFinder( this ),
  newReleaseCheckTimer( this )
{
  applyQtStyleSheet( cfg.preferences.displayStyle );

  ui.setupUi( this );

  wordListDefaultFont = ui.wordList->font();
  translateLineDefaultFont = ui.translateLine->font();

  // Make the search pane's titlebar

  groupLabel.setText( tr( "Look up in:" ) );

  searchPaneTitleBarLayout.setContentsMargins( 8, 5, 8, 4 );
  searchPaneTitleBarLayout.addWidget( &groupLabel );
  searchPaneTitleBarLayout.addWidget( &groupList );
  searchPaneTitleBarLayout.addStretch();

  searchPaneTitleBar.setLayout( &searchPaneTitleBarLayout );

  ui.searchPane->setTitleBarWidget( &searchPaneTitleBar );

  // Make the dictionaries pane's titlebar
  foundInDictsLabel.setText( tr( "Found in Dictionaries:" ) );
  dictsPaneTitleBarLayout.addWidget( &foundInDictsLabel );
  dictsPaneTitleBar.setLayout( &dictsPaneTitleBarLayout );
  ui.dictsPane->setTitleBarWidget( &dictsPaneTitleBar );

  connect( ui.dictsPane, SIGNAL( visibilityChanged( bool ) ),
           this, SLOT( dictsPaneVisibilityChanged ( bool ) ) );

  // Make the toolbar
  navToolbar = addToolBar( tr( "Navigation" ) );
  navToolbar->setObjectName( "navToolbar" );

  navBack = navToolbar->addAction( QIcon( ":/icons/previous.png" ), tr( "Back" ) );
  navForward = navToolbar->addAction( QIcon( ":/icons/next.png" ), tr( "Forward" ) );

  navToolbar->addSeparator();
  navToolbar->addAction( ui.print );
  navToolbar->addAction( ui.saveArticle );

  scanPopupSeparator = navToolbar->addSeparator();
  scanPopupSeparator->setVisible( cfg.preferences.enableScanPopup );

  enableScanPopup = navToolbar->addAction( QIcon( ":/icons/wizard.png" ), tr( "Scan Popup" ) );
  enableScanPopup->setCheckable( true );
  enableScanPopup->setVisible( cfg.preferences.enableScanPopup );
  if ( cfg.preferences.enableScanPopup && cfg.preferences.startWithScanPopupOn )
    enableScanPopup->setChecked( true );

  connect( enableScanPopup, SIGNAL( toggled( bool ) ),
           this, SLOT( scanEnableToggled( bool ) ) );

  navToolbar->addSeparator();
  navPronounce = navToolbar->addAction( QIcon( ":/icons/playsound.png" ), tr( "Pronounce Word (Alt+S)" ) );
  navPronounce->setShortcut( QKeySequence( "Alt+S" ) );
  navPronounce->setEnabled( false );

  connect( navPronounce, SIGNAL( triggered() ),
           this, SLOT( pronounce() ) );

  // zooming
  navToolbar->addSeparator();
  zoomIn = navToolbar->addAction( QIcon( ":/icons/icon32_zoomin.png" ), tr( "Zoom In" ) );
  zoomIn->setShortcut( QKeySequence::ZoomIn );
  zoomOut = navToolbar->addAction( QIcon( ":/icons/icon32_zoomout.png" ), tr( "Zoom Out" ) );
  zoomOut->setShortcut( QKeySequence::ZoomOut );
  zoomBase = navToolbar->addAction( QIcon( ":/icons/icon32_zoombase.png" ), tr( "Normal Size" ) );
  zoomBase->setShortcut( QKeySequence( "Ctrl+0" ) );

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
  wordsZoomIn->setShortcut( QKeySequence( "Alt++" ) );
  wordsZoomOut = ui.menuZoom->addAction( QIcon( ":/icons/icon32_zoomout.png" ), tr( "Words Zoom Out" ) );
  wordsZoomOut->setShortcut( QKeySequence( "Alt+-" ) );
  wordsZoomBase = ui.menuZoom->addAction( QIcon( ":/icons/icon32_zoombase.png" ), tr( "Words Normal Size" ) );
  wordsZoomBase->setShortcut( QKeySequence( "Alt+0" ) );

  connect( wordsZoomIn, SIGNAL(triggered()), this, SLOT(doWordsZoomIn()) );
  connect( wordsZoomOut, SIGNAL(triggered()), this, SLOT(doWordsZoomOut()) );
  connect( wordsZoomBase, SIGNAL(triggered()), this, SLOT(doWordsZoomBase()) );

  // tray icon
  connect( trayIconMenu.addAction( tr( "Show &Main Window" ) ), SIGNAL( activated() ),
           this, SLOT( showMainWindow() ) );
  trayIconMenu.addAction( enableScanPopup );
  trayIconMenu.addSeparator();
  connect( trayIconMenu.addAction( tr( "&Quit" ) ), SIGNAL( activated() ),
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
  groupList.addAction( &escAction );

  ui.centralWidget->addAction( &focusTranslateLineAction );
  ui.dictsPane->addAction( &focusTranslateLineAction );
  ui.searchPaneWidget->addAction( &focusTranslateLineAction );
  groupList.addAction( &focusTranslateLineAction );

  addTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  addTabAction.setShortcut( QKeySequence( "Ctrl+T" ) );

  // Tab management

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

  // Populate 'View' menu

  ui.menuView->addAction( ui.searchPane->toggleViewAction() );
  ui.menuView->addAction( ui.dictsPane->toggleViewAction() );
  ui.menuView->addSeparator();
  ui.menuView->addAction( dictionaryBar.toggleViewAction() );
  ui.menuView->addAction( navToolbar->toggleViewAction() );
  ui.menuView->addSeparator();
  ui.menuView->addAction( &showDictBarNamesAction );
  ui.menuView->addAction( &useSmallIconsInToolbarsAction );

  // Dictionary bar

  showDictBarNamesTriggered(); // Make update its state according to initial
                               // setting

  useSmallIconsInToolbarsTriggered();

  addToolBar( &dictionaryBar );

  connect( dictionaryBar.toggleViewAction(), SIGNAL(triggered(bool)),
           this, SLOT(dictionaryBarToggled(bool)) );
  // This one will be disconnected once the slot is activated. It exists
  // only to handle the initial appearance of the dictionary bar.
  connect( dictionaryBar.toggleViewAction(), SIGNAL(toggled(bool)),
           this, SLOT(dictionaryBarToggled(bool)) );

  connect( &dictionaryBar, SIGNAL(editGroupRequested()),
           this, SLOT(editCurrentGroup()) );

  // History

  connect( &history, SIGNAL( itemsChanged() ),
           this, SLOT( historyChanged() ) );

  connect( ui.menuHistory, SIGNAL(triggered(QAction*)),
           this, SLOT(menuHistoryTriggered(QAction*)), Qt::QueuedConnection );

  // Show tray icon early so the user would be happy. It won't be functional
  // though until the program inits fully.

  if ( cfg.preferences.enableTrayIcon )
  {
    trayIcon = new QSystemTrayIcon( QIcon( ":/icons/programicon.png" ), this );
    trayIcon->setToolTip( tr( "Loading..." ) );
    trayIcon->show();
  }

  connect( navBack, SIGNAL( activated() ),
           this, SLOT( backClicked() ) );
  connect( navForward, SIGNAL( activated() ),
           this, SLOT( forwardClicked() ) );

  addTab.setAutoRaise( true );
  addTab.setIcon( QIcon( ":/icons/addtab.png" ) );

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

  connect( ui.tabWidget, SIGNAL( tabCloseRequested( int ) ),
           this, SLOT( tabCloseRequested( int ) ) );

  connect( ui.tabWidget, SIGNAL( currentChanged( int ) ),
           this, SLOT( tabSwitched( int ) ) );

  connect( ui.tabWidget, SIGNAL( customContextMenuRequested(QPoint)) ,
           this, SLOT( tabMenuRequested(QPoint)) );

#if QT_VERSION >= 0x040500
  ui.tabWidget->setTabsClosable( true );
#endif

  connect( ui.quit, SIGNAL( activated() ),
           qApp, SLOT( quit() ) );

  connect( ui.dictionaries, SIGNAL( activated() ),
           this, SLOT( editDictionaries() ) );

  connect( ui.preferences, SIGNAL( activated() ),
           this, SLOT( editPreferences() ) );

  connect( ui.visitHomepage, SIGNAL( activated() ),
           this, SLOT( visitHomepage() ) );
  connect( ui.visitForum, SIGNAL( activated() ),
           this, SLOT( visitForum() ) );
  connect( ui.about, SIGNAL( activated() ),
           this, SLOT( showAbout() ) );

  connect( &groupList, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );

  connect( ui.translateLine, SIGNAL( textChanged( QString const & ) ),
           this, SLOT( translateInputChanged( QString const & ) ) );

  connect( ui.translateLine, SIGNAL( returnPressed() ),
           this, SLOT( translateInputFinished() ) );

  connect( ui.wordList, SIGNAL( itemSelectionChanged() ),
           this, SLOT( wordListSelectionChanged() ) );

  connect( ui.dictsList, SIGNAL( itemSelectionChanged() ),
           this, SLOT( dictsListSelectionChanged() ) );

  connect( &wordFinder, SIGNAL( updated() ),
           this, SLOT( prefixMatchUpdated() ) );
  connect( &wordFinder, SIGNAL( finished() ),
           this, SLOT( prefixMatchFinished() ) );

  connect( &configEvents, SIGNAL( mutedDictionariesChanged() ),
           this, SLOT( mutedDictionariesChanged() ) );

  ui.translateLine->installEventFilter( this );
  ui.wordList->installEventFilter( this );
  ui.wordList->viewport()->installEventFilter( this );
  ui.dictsList->installEventFilter( this );
  ui.dictsList->viewport()->installEventFilter( this );

  if ( cfg.mainWindowGeometry.size() )
    restoreGeometry( cfg.mainWindowGeometry );

  if ( cfg.mainWindowState.size() )
    restoreState( cfg.mainWindowState, 1 );

  applyProxySettings();
  applyWebSettings();

  makeDictionaries();

  // After we have dictionaries and groups, we can populate history
  historyChanged();

  addNewTab();

  // Create tab list menu
  createTabList();

  // Show the initial welcome text

  {
    ArticleView & view =
      dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

    view.showDefinition( tr( "Welcome!" ), Instances::Group::HelpGroupId );
  }

  ui.translateLine->setFocus();

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

  // Only show window initially if it wasn't configured differently
  if ( !cfg.preferences.enableTrayIcon || !cfg.preferences.startToTray )
  {
    show();
    focusTranslateLine();
  }

  connect( &newReleaseCheckTimer, SIGNAL( timeout() ),
           this, SLOT( checkForNewRelease() ) );

  prepareNewReleaseChecks();

  // makeDictionaries() didn't do deferred init - we do it here, at the end.
  doDeferredInit( dictionaries );
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
  ui.translateLine->setText(str);

        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Enter,
           Qt::NoModifier);
        QApplication::sendEvent(ui.translateLine, &ev);
}

MainWindow::~MainWindow()
{
  commitData();

  // Close all tabs -- they should be destroyed before network managers
  // do.
  while( ui.tabWidget->count() )
  {
    QWidget * w = ui.tabWidget->widget( 0 );

    ui.tabWidget->removeTab( 0 );

    delete w;
  }
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

void MainWindow::applyQtStyleSheet( QString const & displayStyle )
{
  QFile builtInCssFile( ":/qt-style.css" );
  builtInCssFile.open( QFile::ReadOnly );
  QByteArray css = builtInCssFile.readAll();

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

  statusBar()->showMessage( tr( "%1 dictionaries, %2 articles, %3 words" ).
                              arg( dictionaries.size() ).arg( articleCount ).
                              arg( wordCount ) );
}

void MainWindow::updateGroupList()
{
  bool haveGroups = cfg.groups.size();

  groupList.setVisible( haveGroups );

  groupLabel.setText( haveGroups ? tr( "Look up in:" ) : tr( "Look up:" ) );

  // currentIndexChanged() signal is very trigger-happy. To avoid triggering
  // it, we disconnect it while we're clearing and filling back groups.
  disconnect( &groupList, SIGNAL( currentIndexChanged( QString const & ) ),
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

  for( unsigned x  = 0; x < cfg.groups.size(); ++x )
    groupInstances.push_back( Instances::Group( cfg.groups[ x ], dictionaries ) );

  // Update names for dictionaries that are present, so that they could be
  // found in case they got moved.
  Instances::updateNames( cfg, dictionaries );

  groupList.fill( groupInstances );
  groupList.setCurrentGroup( cfg.lastMainGroupId );
  updateCurrentGroupProperty();

  updateDictionaryBar();

  connect( &groupList, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );
}

void MainWindow::updateDictionaryBar()
{
  if ( !dictionaryBar.toggleViewAction()->isChecked() )
    return; // It's not enabled, therefore hidden -- don't waste time

  Instances::Group * grp =
      groupInstances.findGroup( groupList.getCurrentGroup() );

  if ( grp ) { // Should always be !0, but check as a safeguard
    dictionaryBar.setDictionaries( grp->dictionaries );

    if ( useSmallIconsInToolbarsAction.isChecked() ) {
      int extent = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
      dictionaryBar.setIconSize( QSize( extent, extent ) );
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

  connect( scanPopup.get(), SIGNAL(editGroupRequested( unsigned )),
           this,SLOT(editDictionaries( unsigned )), Qt::QueuedConnection );
}

vector< sptr< Dictionary::Class > > const & MainWindow::getActiveDicts()
{
  if ( groupInstances.empty() )
    return dictionaries;

  int current = groupList.currentIndex();

  if ( current < 0 || current >= (int) groupInstances.size() )
  {
    // This shouldn't ever happen
    return dictionaries;
  }

  if ( !dictionaryBar.toggleViewAction()->isChecked() )
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
      if ( !cfg.mutedDictionaries.contains(
              QString::fromStdString( activeDicts[ x ]->getId() ) ) )
        dictionariesUnmuted.push_back( activeDicts[ x ] );

    return dictionariesUnmuted;
  }
}


void MainWindow::createTabList()
{
  tabListMenu = new QMenu(tr("Opened tabs"), ui.tabWidget);
  tabListMenu->setIcon(QIcon(":/icons/windows-list.png"));
  connect(tabListMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowsMenu()));
  connect(tabListMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchToWindow(QAction*)));

  tabListButton = new QToolButton(ui.tabWidget);
  tabListButton->setAutoRaise(true);
  tabListButton->setIcon(tabListMenu->icon());
  tabListButton->setMenu(tabListMenu);
  tabListButton->setPopupMode(QToolButton::InstantPopup);
  ui.tabWidget->setCornerWidget(tabListButton);
}

void MainWindow::fillWindowsMenu()
{
  tabListMenu->clear();

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
                                        &cfg.mutedDictionaries,
                                        &groupList );

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

  int index = cfg.preferences.newTabsOpenAfterCurrentOne ?
              ui.tabWidget->currentIndex() + 1 : ui.tabWidget->count();

  QString escaped = name;
  escaped.replace( "&", "&&" );

  ui.tabWidget->insertTab( index, view, escaped );

  if ( switchToIt )
    ui.tabWidget->setCurrentIndex( index );

  view->setZoomFactor( cfg.preferences.zoomFactor );

  return view;
}


void MainWindow::tabCloseRequested( int x )
{
//  if ( ui.tabWidget->count() < 2 )
//    return; // We should always have at least one open tab

  QWidget * w = ui.tabWidget->widget( x );

  ui.tabWidget->removeTab( x );

  delete w;

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

void MainWindow::backClicked()
{
  printf( "Back\n" );

  ArticleView & view =
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  view.back();
}

void MainWindow::forwardClicked()
{
  printf( "Forward\n" );

  ArticleView & view =
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  view.forward();
}

void MainWindow::titleChanged( ArticleView * view, QString const & title )
{
  QString escaped = title;
  escaped.replace( "&", "&&" );

  ui.tabWidget->setTabText( ui.tabWidget->indexOf( view ), escaped );
}

void MainWindow::iconChanged( ArticleView * view, QIcon const & icon )
{
  ui.tabWidget->setTabIcon( ui.tabWidget->indexOf( view ), groupInstances.size() > 1 ? icon : QIcon() );
}

void MainWindow::pageLoaded( ArticleView * view )
{
  updateBackForwardButtons();

  updatePronounceAvailability();

  if ( cfg.preferences.pronounceOnLoadMain )
    pronounce( view );

  updateFoundInDictsList();
}

void MainWindow::tabSwitched( int )
{
  updateBackForwardButtons();
  updatePronounceAvailability();
  updateFoundInDictsList();
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
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).playSound();
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

  if ( QWidget * cw = ui.tabWidget->currentWidget() )
  {
    ArticleView & view = dynamic_cast< ArticleView & >( *( cw ) );

    QStringList ids = view.getArticlesList();

    for( QStringList::const_iterator i = ids.constBegin(); i != ids.constEnd(); ++i)
    {
      // Find this dictionary

      for( unsigned x = dictionaries.size(); x--; )
      {
        if ( dictionaries[ x ]->getId() == i->toUtf8().data() )
        {
          QString dictName = QString::fromUtf8( dictionaries[ x ]->getName().c_str() );
          QListWidgetItem * item =
              new QListWidgetItem(
                dictionaries[ x ]->getIcon(),
                dictName,
                ui.dictsList, QListWidgetItem::Type );
          item->setData(Qt::UserRole,
                QVariant( QString::fromUtf8(dictionaries[ x ]->getId().c_str() ) ) );
          item->setToolTip(dictName);

          ui.dictsList->addItem( item );
          break;
        }
      }
    }
  }
}

void MainWindow::updateBackForwardButtons()
{
  if ( QWidget * cw = ui.tabWidget->currentWidget() )
  {
    ArticleView & view = dynamic_cast< ArticleView & >( *( cw ) );
    navBack->setEnabled(view.canGoBack());
    navForward->setEnabled(view.canGoForward());
  }
}

void MainWindow::updatePronounceAvailability()
{
  bool pronounceEnabled = ui.tabWidget->count() > 0 &&
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).hasSound();

  navPronounce->setEnabled( pronounceEnabled );
}

void MainWindow::editDictionaries( unsigned editDictionaryGroup )
{
  hotkeyWrapper.reset(); // No hotkeys while we're editing dictionaries
  scanPopup.reset(); // No scan popup either. No one should use dictionaries.

  wordFinder.clear();
  dictionariesUnmuted.clear();

  EditDictionaries dicts( this, cfg, dictionaries, groupInstances, dictNetMgr );

  if ( editDictionaryGroup != Instances::Group::NoGroupId )
    dicts.editGroup( editDictionaryGroup );

  dicts.exec();

  if ( dicts.areDictionariesChanged() || dicts.areGroupsChanged() )
  {
    updateGroupList();

    Config::save( cfg );
  }

  makeScanPopup();
  installHotKeys();
}

void MainWindow::editCurrentGroup()
{
  editDictionaries( groupList.getCurrentGroup() );
}

void MainWindow::editPreferences()
{
  hotkeyWrapper.reset(); // So we could use the keys it hooks

  Preferences preferences( this, cfg.preferences );

  preferences.show();

  if ( preferences.exec() == QDialog::Accepted )
  {
    Config::Preferences p = preferences.getPreferences();

    // See if we need to reapply stylesheets
    if ( cfg.preferences.displayStyle != p.displayStyle )
    {
      applyQtStyleSheet( p.displayStyle );
      articleMaker.setDisplayStyle( p.displayStyle );

      for( int x = 0; x < ui.tabWidget->count(); ++x )
      {
        ArticleView & view =
          dynamic_cast< ArticleView & >( *( ui.tabWidget->widget( x ) ) );

        view.reload();
      }
    }

    cfg.preferences = p;

    scanPopupSeparator->setVisible( cfg.preferences.enableScanPopup );
    enableScanPopup->setVisible( cfg.preferences.enableScanPopup );

    if ( !cfg.preferences.enableScanPopup )
      enableScanPopup->setChecked( false );

    updateTrayIcon();
    applyProxySettings();
    applyWebSettings();
    makeScanPopup();

    setAutostart( cfg.preferences.autoStart );

    prepareNewReleaseChecks();

    Config::save( cfg );
  }

  installHotKeys();
}

void MainWindow::currentGroupChanged( QString const & )
{
  cfg.lastMainGroupId = groupList.getCurrentGroup();

  updateDictionaryBar();

  // Update word search results

  translateInputChanged( ui.translateLine->text() );

  updateCurrentGroupProperty();
}

void MainWindow::updateCurrentGroupProperty()
{
  // We maintain currentGroup property so styles could use that to change
  // fonts based on group names
  Instances::Group * grp =
      groupInstances.findGroup( groupList.getCurrentGroup() );

  if ( grp && ui.translateLine->property( "currentGroup" ).toString() !=
       grp->name )
  {
    ui.translateLine->setProperty( "currentGroup", grp->name );
    ui.wordList->setProperty( "currentGroup", grp->name );
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
  if ( !statusBar()->currentMessage().isEmpty() )
    statusBar()->clearMessage();

  // If some word is selected in the word list, unselect it. This prevents
  // triggering a set of spurious activation signals when the list changes.

  if ( ui.wordList->selectionModel()->hasSelection() )
    ui.wordList->setCurrentItem( 0, QItemSelectionModel::Clear );

  QString req = newValue.trimmed();

  if ( !req.size() )
  {
    // An empty request always results in an empty result
    wordFinder.cancel();
    ui.wordList->clear();
    ui.wordList->unsetCursor();

    // Reset the noResults mark if it's on right now
    if ( ui.translateLine->property( "noResults" ).toBool() )
    {
      ui.translateLine->setProperty( "noResults", false );
      setStyleSheet( styleSheet() );
    }
    return;
  }

  ui.wordList->setCursor( Qt::WaitCursor );

  wordFinder.prefixMatch( req, getActiveDicts() );
}

void MainWindow::translateInputFinished()
{
  QString word = ui.translateLine->text();

  if ( word.size() )
  {
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if ( mods & (Qt::ControlModifier | Qt::ShiftModifier) )
      addNewTab();

    showTranslationFor( word );

    if ( ui.searchPane->isFloating() )
      activateWindow();

    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).focus();
  }
}

void MainWindow::handleEsc()
{
  if ( dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).closeSearch() )
    return;

  if( cfg.preferences.escKeyHidesMainWindow )
  {
    if( cfg.preferences.enableTrayIcon )
      hide();
    else
      showMinimized();
  }
  else
    focusTranslateLine();
}

void MainWindow::focusTranslateLine()
{
  if ( ui.searchPane->isFloating() )
    ui.searchPane->activateWindow();

  ui.translateLine->setFocus();
  ui.translateLine->selectAll();
}

void MainWindow::prefixMatchUpdated()
{
  updateMatchResults( false );
}

void MainWindow::prefixMatchFinished()
{
  updateMatchResults( true );
}

void MainWindow::updateMatchResults( bool finished )
{
  WordFinder::SearchResults const & results = wordFinder.getResults();

  ui.wordList->setUpdatesEnabled( false );

  for( unsigned x = 0; x < results.size(); ++x )
  {
    QListWidgetItem * i = ui.wordList->item( x );

    if ( !i )
    {
      i = new QListWidgetItem( results[ x ].first, ui.wordList );

      if ( results[ x ].second )
      {
        QFont f = i->font();
        f.setItalic( true );
        i->setFont( f );
      }
      ui.wordList->addItem( i );
    }
    else
    {
      if ( i->text() != results[ x ].first )
        i->setText( results[ x ].first );

      QFont f = i->font();
      if ( f.italic() != results[ x ].second )
      {
        f.setItalic( results[ x ].second );
        i->setFont( f );
      }
    }
    if (i->text().at(0).direction() == QChar::DirR)
        i->setTextAlignment(Qt::AlignRight);
    if (i->text().at(0).direction() == QChar::DirL)
        i->setTextAlignment(Qt::AlignLeft);
  }

  while ( ui.wordList->count() > (int) results.size() )
  {
    // Chop off any extra items that were there
    QListWidgetItem * i = ui.wordList->takeItem( ui.wordList->count() - 1 );

    if ( i )
      delete i;
    else
      break;
  }

  if ( ui.wordList->count() )
  {
    ui.wordList->scrollToItem( ui.wordList->item( 0 ), QAbstractItemView::PositionAtTop );
    ui.wordList->setCurrentItem( 0, QItemSelectionModel::Clear );
  }

  ui.wordList->setUpdatesEnabled( true );

  if ( finished )
  {
    ui.wordList->unsetCursor();

    // Visually mark the input line to mark if there's no results

    bool setMark = results.empty() && !wordFinder.wasSearchUncertain();

    if ( ui.translateLine->property( "noResults" ).toBool() != setMark )
    {
      ui.translateLine->setProperty( "noResults", setMark );
      setStyleSheet( styleSheet() );
    }

    if ( !wordFinder.getErrorString().isEmpty() )
      statusBar()->showMessage( tr( "WARNING: %1" ).arg( wordFinder.getErrorString() ) );
  }
}

void MainWindow::applyMutedDictionariesState()
{
  // Redo the current search request
  translateInputChanged( ui.translateLine->text() );

  if ( QWidget * tabWidget = ui.tabWidget->currentWidget() )
  {
    // Update active article view
    ArticleView & view =
      dynamic_cast< ArticleView & >( *tabWidget );

    view.updateMutedContents();
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
  if ( ev->type() == QEvent::MouseButtonPress ) {
    QMouseEvent * event = static_cast< QMouseEvent * >( ev );
    return handleBackForwardMouseButtons( event );
  }

  if ( obj == ui.translateLine )
  {
    if ( ev->type() == QEvent::KeyPress )
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( keyEvent->matches( QKeySequence::MoveToNextLine ) && ui.wordList->count() )
      {
        ui.wordList->setFocus( Qt::ShortcutFocusReason );
        ui.wordList->setCurrentRow( 0, QItemSelectionModel::ClearAndSelect );
        return true;
      }
    }
    else
    if ( ev->type() == QEvent::FocusIn ) {
      QFocusEvent * focusEvent = static_cast< QFocusEvent * >( ev );

      // select all on mouse click
      if ( focusEvent->reason() == Qt::MouseFocusReason ) {
        QTimer::singleShot(0, this, SLOT(focusTranslateLine()));
      }
      return false;
    }
  }
  else
  if ( obj == ui.wordList )
  {
    if ( ev->type() == QEvent::KeyPress )
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( keyEvent->matches( QKeySequence::MoveToPreviousLine ) &&
           !ui.wordList->currentRow() )
      {
        ui.wordList->setCurrentRow( 0, QItemSelectionModel::Clear );
        ui.translateLine->setFocus( Qt::ShortcutFocusReason );
        return true;
      }

      if ( keyEvent->matches( QKeySequence::InsertParagraphSeparator ) &&
           ui.wordList->selectedItems().size() )
      {
        if ( ui.searchPane->isFloating() )
          activateWindow();

        dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).focus();

        return true;
      }

      // Handle typing events used to initiate new lookups
      // TODO: refactor to eliminate duplication (see below)

      if ( keyEvent->modifiers() &
           ( Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier ) )
        return false; // A non-typing modifier is pressed

      if ( keyEvent->key() == Qt::Key_Space ||
           keyEvent->key() == Qt::Key_Backspace ||
           keyEvent->key() == Qt::Key_Tab )
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
           keyEvent->key() == Qt::Key_Tab )
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
    return QMainWindow::eventFilter( obj, ev );

  return false;
}

void MainWindow::wordListItemActivated( QListWidgetItem * item )
{
  showTranslationFor( item->text() );
}

void MainWindow::wordListSelectionChanged()
{
  QList< QListWidgetItem * > selected = ui.wordList->selectedItems();

  if ( selected.size() )
    wordListItemActivated( selected.front() );
}

void MainWindow::dictsListItemActivated( QListWidgetItem * item )
{
  ArticleView & view = dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );
  QString id = item->data( Qt::UserRole ).toString();
  view.jumpToDictionary( id );
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

void MainWindow::typingEvent( QString const & t )
{
  if ( t == "\n" || t == "\r" )
    focusTranslateLine();
  else
  {
    if ( ui.searchPane->isFloating() || ui.dictsPane->isFloating() )
      ui.searchPane->activateWindow();

    ui.translateLine->setText( t );
    ui.translateLine->setFocus();
    ui.translateLine->setCursorPosition( t.size() );
  }
}

void MainWindow::mutedDictionariesChanged()
{
  if ( dictionaryBar.toggleViewAction()->isChecked() )
    applyMutedDictionariesState();
}

void MainWindow::showTranslationFor( QString const & inWord,
                                     unsigned inGroup )
{
  ArticleView & view =
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  navPronounce->setEnabled( false );

  unsigned group = inGroup ? inGroup :
                   ( groupInstances.empty() ? 0 :
                        groupInstances[ groupList.currentIndex() ].id );

  view.showDefinition( inWord, group );

  updatePronounceAvailability();
  updateFoundInDictsList();

  // Add to history

  history.addItem( History::Item( group, inWord.trimmed() ) );
  history.save();

  updateBackForwardButtons();

  #if 0
  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  req.addQueryItem( "word", inWord );
  req.addQueryItem( "group",
                    cfg.groups.empty() ? "" :
                      groupInstances[ groupList.currentIndex() ].name );

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
    printf( "Alt: %ls\n", alts[ x ].c_str() );
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

      printf( "From %s: %s\n", activeDicts[ x ]->getName().c_str(), body.c_str() );

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

void MainWindow::toggleMainWindow( bool onlyShow )
{
  bool shown = false;

  if ( !isVisible() )
  {
    show();
    activateWindow();
    raise();
    shown = true;
  }
  else
  if ( isMinimized() )
  {
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
    hide();

  if ( shown )
    focusTranslateLine();
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
      QMessageBox::critical( this, tr( "GoldenDict" ),
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
    on_actionCloseToTray_activated();
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

    printf( "Failed to check program version, retry in two hours\n" );
  }
  else
  {
    // Success -- reschedule for a normal check and save config
    cfg.timeForNewReleaseCheck = QDateTime();

    prepareNewReleaseChecks();

    Config::save( cfg );

    printf( "Program version's check successful, current version is %ls\n",
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
      toggleMainWindow();
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
      scanPopup->enableScanning();
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

  updateDictionaryBar();

  cfg.usingSmallIconsInToolbars = useSmallIcons;
}

void MainWindow::historyChanged()
{
  // Rebuild history menu

  ui.menuHistory->clear();

  QList< History::Item > const & items = history.getItems();

  for( int x = 0; x < items.size(); ++x )
  {
    History::Item const * i = &items[ x ];

    Instances::Group const * group = groupInstances.findGroup( i->groupId );

    QIcon icon = group ? group->makeIcon() : QIcon();

    QAction * action = ui.menuHistory->addAction( icon, i->word );
    action->setIconVisibleInMenu( true );

    action->setData( x );
  }

  ui.menuHistory->addSeparator();

  ui.menuHistory->addAction( ui.clearHistory );

  ui.clearHistory->setEnabled( items.size() );
}

void MainWindow::menuHistoryTriggered( QAction * action )
{
  if ( action->data().type() != QVariant::Int )
    return; // Not the item we added

  int index = action->data().toInt();

  QList< History::Item > const & items = history.getItems();

  if ( index < 0 || index >= items.size() )
    return; // Something's not right

  History::Item const & item = items[ index ];

  showTranslationFor( item.word, item.groupId );
}

void MainWindow::on_clearHistory_activated()
{
  history.clear();
  history.save();
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

void MainWindow::on_actionCloseToTray_activated()
{
  toggleMainWindow( !cfg.preferences.enableTrayIcon );
}

void MainWindow::on_pageSetup_activated()
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

void MainWindow::on_printPreview_activated()
{
  QPrintPreviewDialog dialog( &getPrinter(), this );

  connect( &dialog, SIGNAL( paintRequested( QPrinter * ) ),
           this, SLOT( printPreviewPaintRequested( QPrinter * ) ) );

  dialog.exec();
}

void MainWindow::on_print_activated()
{
  QPrintDialog dialog( &getPrinter(), this );

  dialog.setWindowTitle( tr( "Print Article") );

  if ( dialog.exec() != QDialog::Accepted )
   return;

  ArticleView & view = dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  view.print( &getPrinter() );
}

void MainWindow::printPreviewPaintRequested( QPrinter * printer )
{
  ArticleView & view = dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  view.print( printer );
}

void MainWindow::on_saveArticle_activated()
{
  ArticleView & view = dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  QFileDialog fileDialog( this, tr( "Save Article As" ), QString(), tr( "Html files (*.html *.htm)" ) );

  fileDialog.setAcceptMode( QFileDialog::AcceptSave );

  fileDialog.setDefaultSuffix( "html" );

  fileDialog.selectFile( view.getTitle() + ".html" );

  if ( fileDialog.exec() && fileDialog.selectedFiles().size() == 1 )
  {
    QString fileName = fileDialog.selectedFiles().front();

    QFile file( fileName );

    if ( !file.open( QIODevice::WriteOnly ) )
      QMessageBox::critical( this, tr( "Error" ),
                             tr( "Can't save article: %1" ).arg( file.errorString() ) );
    else
      file.write( view.toHtml().toUtf8() );
  }
}

void MainWindow::on_rescanFiles_activated()
{
  hotkeyWrapper.reset(); // No hotkeys while we're editing dictionaries
  scanPopup.reset(); // No scan popup either. No one should use dictionaries.

  groupInstances.clear(); // Release all the dictionaries they hold

  loadDictionaries( this, true, cfg, dictionaries, dictNetMgr );

  updateGroupList();

  makeScanPopup();
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
  if ( cfg.preferences.zoomFactor >= 3 )
    cfg.preferences.zoomFactor = 3;
  else if ( cfg.preferences.zoomFactor <= 0.7 )
    cfg.preferences.zoomFactor = 0.7;

  zoomIn->setEnabled( cfg.preferences.zoomFactor < 3 );
  zoomOut->setEnabled( cfg.preferences.zoomFactor > 0.7 );
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

  if ( ui.wordList->font().pointSize() != ps )
    ui.wordList->setFont( font );

  font = translateLineDefaultFont;

  ps = font.pointSize();

  if ( cfg.preferences.wordsZoomLevel != 0 )
  {
    ps += cfg.preferences.wordsZoomLevel;

    if ( ps < 1 )
      ps = 1;

    font.setPointSize( ps );
  }

  if ( ui.translateLine->font().pointSize() != ps )
    ui.translateLine->setFont( font );

  wordsZoomBase->setEnabled( cfg.preferences.wordsZoomLevel != 0 );
}

void MainWindow::messageFromAnotherInstanceReceived( QString const & message )
{
  if ( message == "bringToFront" )
    toggleMainWindow( true );
  else
    qWarning() << "Unknown message received from another instance: " << message;
}
