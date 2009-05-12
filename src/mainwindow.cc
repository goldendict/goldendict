/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mainwindow.hh"
#include "editdictionaries.hh"
#include "loaddictionaries.hh"
#include "preferences.hh"
#include "ui_about.h"
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
  trayIcon( 0 ),
  groupLabel( &searchPaneTitleBar ),
  groupList( &searchPaneTitleBar ),
  focusTranslateLineAction( this ),
  addTabAction( this ),
  closeCurrentTabAction( this ),
  switchToNextTabAction( this ),
  switchToPrevTabAction( this ),
  trayIconMenu( this ),
  addTab( this ),
  cfg( cfg_ ),
  articleMaker( dictionaries, groupInstances, cfg.preferences.displayStyle ),
  articleNetMgr( this, dictionaries, articleMaker ),
  dictNetMgr( this ),
  wordFinder( this ),
  newReleaseCheckTimer( this )
{
  ui.setupUi( this );

  // Make the search pane's titlebar

  groupLabel.setText( tr( "Look up in:" ) );

  searchPaneTitleBarLayout.setContentsMargins( 8, 5, 8, 4 );
  searchPaneTitleBarLayout.addWidget( &groupLabel );
  searchPaneTitleBarLayout.addWidget( &groupList );
  searchPaneTitleBarLayout.addStretch();

  searchPaneTitleBar.setLayout( &searchPaneTitleBarLayout );

  ui.searchPane->setTitleBarWidget( &searchPaneTitleBar );

  // Make the toolbar
  navToolbar = addToolBar( tr( "Navigation" ) );
  navToolbar->setObjectName( "navToolbar" );

  navBack = navToolbar->addAction( QIcon( ":/icons/previous.png" ), tr( "Back" ) );
  navForward = navToolbar->addAction( QIcon( ":/icons/next.png" ), tr( "Forward" ) );

  navToolbar->addSeparator();
  navToolbar->addAction( ui.print );
  navToolbar->addAction( ui.saveArticle );

  navToolbar->addSeparator();
  enableScanPopup = navToolbar->addAction( QIcon( ":/icons/wizard.png" ), tr( "Scan Popup" ) );
  enableScanPopup->setCheckable( true );
  enableScanPopup->setVisible( cfg.preferences.enableScanPopup );
  if ( cfg.preferences.enableScanPopup && cfg.preferences.startWithScanPopupOn )
    enableScanPopup->setChecked( true );

  connect( enableScanPopup, SIGNAL( toggled( bool ) ),
           this, SLOT( scanEnableToggled( bool ) ) );

  navToolbar->addSeparator();
  navPronounce = navToolbar->addAction( QIcon( ":/icons/playsound.png" ), tr( "Pronounce word" ) );
  navPronounce->setEnabled( false );

  connect( navPronounce, SIGNAL( triggered() ),
           this, SLOT( pronounce() ) );

  // zooming
  navToolbar->addSeparator();
  zoomIn = navToolbar->addAction( QIcon( ":/icons/icon32_zoomin.png" ), tr( "Zoom In" ) );
  zoomOut = navToolbar->addAction( QIcon( ":/icons/icon32_zoomout.png" ), tr( "Zoom Out" ) );
  zoomBase = navToolbar->addAction( QIcon( ":/icons/icon32_zoombase.png" ), tr( "Normal Size" ) );

  connect( zoomIn, SIGNAL( triggered() ),
           this, SLOT( zoomin() ) );
  connect( zoomOut, SIGNAL( triggered() ),
           this, SLOT( zoomout() ) );
  connect( zoomBase, SIGNAL( triggered() ),
           this, SLOT( unzoom() ) );

  // tray icon
  connect( trayIconMenu.addAction( tr( "Show &Main Window" ) ), SIGNAL( activated() ),
           this, SLOT( showMainWindow() ) );
  trayIconMenu.addAction( enableScanPopup );
  trayIconMenu.addSeparator();
  connect( trayIconMenu.addAction( tr( "&Quit" ) ), SIGNAL( activated() ),
           qApp, SLOT( quit() ) );

  focusTranslateLineAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  focusTranslateLineAction.setShortcut( QKeySequence( "Esc" ) );

  connect( &focusTranslateLineAction, SIGNAL( triggered() ),
           this, SLOT( focusTranslateLine() ) );

  ui.centralWidget->addAction( &focusTranslateLineAction );
  ui.searchPaneWidget->addAction( &focusTranslateLineAction );
  groupList.addAction( &focusTranslateLineAction );

  addTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  addTabAction.setShortcut( QKeySequence( "Ctrl+T" ) );

  connect( &addTabAction, SIGNAL( triggered() ),
           this, SLOT( addNewTab() ) );

  addAction( &addTabAction );

  closeCurrentTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  closeCurrentTabAction.setShortcut( QKeySequence( "Ctrl+W" ) );

  connect( &closeCurrentTabAction, SIGNAL( triggered() ),
           this, SLOT( closeCurrentTab() ) );

  addAction( &closeCurrentTabAction );

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

  // Show tray icon early so the user would be happy
  updateTrayIcon();

  if ( trayIcon )
    trayIcon->setToolTip( tr( "Loading..." ) );

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

  connect( &addTab, SIGNAL( clicked() ),
           this, SLOT( addNewTab() ) );

  connect( ui.tabWidget, SIGNAL( tabCloseRequested( int ) ),
           this, SLOT( tabCloseRequested( int ) ) );

  connect( ui.tabWidget, SIGNAL( currentChanged( int ) ),
           this, SLOT( tabSwitched( int ) ) );

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

  connect( &wordFinder, SIGNAL( updated() ),
           this, SLOT( prefixMatchUpdated() ) );
  connect( &wordFinder, SIGNAL( finished() ),
           this, SLOT( prefixMatchFinished() ) );

  ui.translateLine->installEventFilter( this );
  ui.wordList->installEventFilter( this );

  if ( cfg.mainWindowGeometry.size() )
    restoreGeometry( cfg.mainWindowGeometry );

  if ( cfg.mainWindowState.size() )
    restoreState( cfg.mainWindowState, 1 );

  applyProxySettings();

  makeDictionaries();

  addNewTab();

  // Show the initial welcome text

  {
    ArticleView & view =
      dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

    view.showDefinition( tr( "Welcome!" ), UINT_MAX );
  }

  ui.translateLine->setFocus();

  updateTrayIcon();

  // Update zoomers
  applyZoomFactor();

  // Update autostart info
  setAutostart(cfg.preferences.autoStart);

  // Initialize global hotkeys
  installHotKeys();

  // Only show window initially if it wasn't configured differently
  if ( !cfg.preferences.enableTrayIcon || !cfg.preferences.startToTray )
    show();

  connect( &newReleaseCheckTimer, SIGNAL( timeout() ),
           this, SLOT( checkForNewRelease() ) );

  prepareNewReleaseChecks();
}

MainWindow::~MainWindow()
{
  // Save MainWindow state and geometry
  cfg.mainWindowState = saveState( 1 );
  cfg.mainWindowGeometry = saveGeometry();

  // Save any changes in last chosen groups etc
  Config::save( cfg );
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

  qApp->setStyleSheet( css );
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
    ev->accept();
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

void MainWindow::makeDictionaries()
{
  scanPopup.reset();

  wordFinder.clear();

  loadDictionaries( this, isVisible(), cfg, dictionaries, dictNetMgr );

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

  for( unsigned x  = 0; x < cfg.groups.size(); ++x )
  {
    groupInstances.push_back( Instances::Group( cfg.groups[ x ], dictionaries ) );

    // Update names for dictionaries that are present, so that they could be
    // found in case they got moved.
    Instances::updateNames( cfg.groups[ x ], dictionaries );
  }

  groupList.fill( groupInstances );
  groupList.setCurrentGroup( cfg.lastMainGroupId );

  connect( &groupList, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );
}

void MainWindow::makeScanPopup()
{
  scanPopup.reset();

  if ( !cfg.preferences.enableScanPopup &&
       !cfg.preferences.enableClipboardHotkey )
    return;

  scanPopup = new ScanPopup( 0, cfg, articleNetMgr, dictionaries, groupInstances );

  if ( cfg.preferences.enableScanPopup && enableScanPopup->isChecked() )
    scanPopup->enableScanning();
}

vector< sptr< Dictionary::Class > > const & MainWindow::getActiveDicts()
{
  if ( cfg.groups.empty() )
    return dictionaries;

  int current = groupList.currentIndex();

  if ( current < 0 || current >= (int) groupInstances.size() )
  {
    // This shouldn't ever happen
    return dictionaries;
  }

  return groupInstances[ current ].dictionaries;
}

void MainWindow::addNewTab()
{
  createNewTab( true, tr( "(untitled)" ) );
}

ArticleView * MainWindow::createNewTab( bool switchToIt,
                                        QString const & name )
{
  ArticleView * view = new ArticleView( this, articleNetMgr, dictionaries,
                                        groupInstances, false, cfg );

  connect( view, SIGNAL( titleChanged(  ArticleView *, QString const & ) ),
           this, SLOT( titleChanged(  ArticleView *, QString const & ) ) );

  connect( view, SIGNAL( iconChanged( ArticleView *, QIcon const & ) ),
           this, SLOT( iconChanged( ArticleView *, QIcon const & ) ) );

  connect( view, SIGNAL( pageLoaded() ), this, SLOT( pageLoaded() ) );

  connect( view, SIGNAL( openLinkInNewTab( QUrl const &, QUrl const &, QString const & ) ),
           this, SLOT( openLinkInNewTab( QUrl const &, QUrl const &, QString const & ) ) );

  connect( view, SIGNAL( showDefinitionInNewTab( QString const &, unsigned, QString const & ) ),
           this, SLOT( showDefinitionInNewTab( QString const &, unsigned, QString const & ) ) );

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
  if ( ui.tabWidget->count() < 2 )
    return; // We should always have at least one open tab

  QWidget * w = ui.tabWidget->widget( x );

  ui.tabWidget->removeTab( x );

  delete w;
}

void MainWindow::closeCurrentTab()
{
  tabCloseRequested( ui.tabWidget->currentIndex() );
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
  ui.tabWidget->setTabIcon( ui.tabWidget->indexOf( view ), icon );
}

void MainWindow::pageLoaded()
{
  updatePronounceAvailability();

  if ( cfg.preferences.pronounceOnLoadMain )
    pronounce();
}

void MainWindow::tabSwitched( int )
{
  updatePronounceAvailability();
}

void MainWindow::pronounce()
{
  dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).playSound();
}

void MainWindow::updatePronounceAvailability()
{
  bool pronounceEnabled = ui.tabWidget->count() > 0 &&
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).hasSound();

  navPronounce->setEnabled( pronounceEnabled );
}

void MainWindow::editDictionaries()
{
  hotkeyWrapper.reset(); // No hotkeys while we're editing dictionaries
  scanPopup.reset(); // No scan popup either. No one should use dictionaries.

  EditDictionaries dicts( this, cfg, dictionaries, dictNetMgr );

  dicts.exec();

  if ( dicts.areDictionariesChanged() || dicts.areGroupsChanged() )
  {
    updateGroupList();

    Config::save( cfg );
  }

  makeScanPopup();
  installHotKeys();
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

    enableScanPopup->setVisible( cfg.preferences.enableScanPopup );

    if ( !cfg.preferences.enableScanPopup )
      enableScanPopup->setChecked( false );

    updateTrayIcon();
    applyProxySettings();
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

  // Update word search results

  translateInputChanged( ui.translateLine->text() );
}

void MainWindow::translateInputChanged( QString const & newValue )
{
  // If there's some status bar message present, clear it since it may be
  // about the previous search that has failed.
  if ( !statusBar()->currentMessage().isEmpty() )
    statusBar()->clearMessage();

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
      qApp->setStyleSheet( qApp->styleSheet() );
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
    if ( mods & Qt::ControlModifier )
      addNewTab();

    showTranslationFor( word );

    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).focus();
  }
}

void MainWindow::focusTranslateLine()
{
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

    bool setMark = results.empty();

    if ( ui.translateLine->property( "noResults" ).toBool() != setMark )
    {
      ui.translateLine->setProperty( "noResults", setMark );
      qApp->setStyleSheet( qApp->styleSheet() );
    }

    if ( !wordFinder.getErrorString().isEmpty() )
      statusBar()->showMessage( tr( "WARNING: %1" ).arg( wordFinder.getErrorString() ) );
  }
}

bool MainWindow::eventFilter( QObject * obj, QEvent * ev )
{
  if ( obj == ui.translateLine )
  {
    if ( ev->type() == /*QEvent::KeyPress*/ 6 )
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( keyEvent->matches( QKeySequence::MoveToNextLine ) && ui.wordList->count() )
      {
        ui.wordList->setFocus( Qt::ShortcutFocusReason );
        ui.wordList->setCurrentRow( 0, QItemSelectionModel::ClearAndSelect );
        return true;
      }
    }
  }
  else
  if ( obj == ui.wordList )
  {
    if ( ev->type() == /*QEvent::KeyPress*/ 6 )
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( keyEvent->matches( QKeySequence::MoveToPreviousLine ) &&
           !ui.wordList->currentRow() )
      {
        ui.wordList->setCurrentRow( 0, QItemSelectionModel::Clear );
        ui.translateLine->setFocus( Qt::ShortcutFocusReason );
        return true;
      }

#if QT_VERSION >= 0x040500
      if ( keyEvent->matches( QKeySequence::InsertParagraphSeparator ) &&
           ui.wordList->selectedItems().size() )
      {
        dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).focus();

        return true;
      }
#endif

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

void MainWindow::openLinkInNewTab( QUrl const & url,
                                   QUrl const & referrer,
                                   QString const & fromArticle )
{
  createNewTab( !cfg.preferences.newTabsOpenInBackground, "" )->
      openLink( url, referrer, fromArticle );
}

void MainWindow::showDefinitionInNewTab( QString const & word,
                                         unsigned group,
                                         QString const & fromArticle )
{
  createNewTab( !cfg.preferences.newTabsOpenInBackground, word )->
      showDefinition( word, group, fromArticle );
}

void MainWindow::typingEvent( QString const & t )
{
  if ( t == "\n" || t == "\r" )
    focusTranslateLine();
  else
  {
    ui.translateLine->setText( t );
    ui.translateLine->setFocus();
    ui.translateLine->setCursorPosition( t.size() );
  }
}

void MainWindow::showTranslationFor( QString const & inWord )
{
  ArticleView & view =
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  navPronounce->setEnabled( false );

  view.showDefinition( inWord, cfg.groups.empty() ? 0 :
                        groupInstances[ groupList.currentIndex() ].id );

  updatePronounceAvailability();

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
  if ( !isVisible() )
  {
    show();
    activateWindow();
    raise();
  }
  else
  if ( isMinimized() )
  {
    showNormal();
    activateWindow();
    raise();
  }
  else
  if ( !isActiveWindow() )
  {
    activateWindow();
    raise();
  }
  else
  if ( !onlyShow )
    hide();
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
    QUrl( "http://goldendict.berlios.de/latest_release.php?current="
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

  if ( success && latestVersion > PROGRAM_VERSION )
  {
    QMessageBox msg( QMessageBox::Information,
                     tr( "New Release Available" ),
                     tr( "Version <b>%1</b> of GoldenDict is now available for download.<br>"
                         "Click <b>Download</b> to get to the download page." ).arg( latestVersion ),
                     QMessageBox::NoButton,
                     this );

    QPushButton * dload = msg.addButton( tr( "Download" ), QMessageBox::AcceptRole );
    msg.addButton( QMessageBox::Cancel );

    msg.exec();

    if ( msg.clickedButton() == dload )
      QDesktopServices::openUrl( QUrl( downloadUrl ) );
  }
}

void MainWindow::trayIconActivated( QSystemTrayIcon::ActivationReason r )
{
  if ( r == QSystemTrayIcon::Trigger )
  {
    // Left click toggles the visibility of main window
    toggleMainWindow();
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
  QDesktopServices::openUrl( QUrl( "http://goldendict.berlios.de/" ) );
}

void MainWindow::visitForum()
{
  QDesktopServices::openUrl( QUrl( "http://goldendict.berlios.de/forum/" ) );
}

void MainWindow::showAbout()
{
  QDialog about( this );

  Ui::About ui;

  ui.setupUi( &about );

  ui.version->setText( PROGRAM_VERSION );

  about.show();
  about.exec();
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
  QPageSetupDialog dialog( &printer, this );

  dialog.exec();
}

void MainWindow::on_printPreview_activated()
{
  QPrintPreviewDialog dialog( &printer, this );

  connect( &dialog, SIGNAL( paintRequested( QPrinter * ) ),
           this, SLOT( printPreviewPaintRequested( QPrinter * ) ) );

  dialog.exec();
}

void MainWindow::on_print_activated()
{
  QPrintDialog dialog( &printer, this );

  dialog.setWindowTitle( tr( "Print Article") );

  if ( dialog.exec() != QDialog::Accepted )
   return;

  ArticleView & view = dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  view.print( &printer );
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
