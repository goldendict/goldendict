/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mainwindow.hh"
#include "sources.hh"
#include "groups.hh"
#include "preferences.hh"
#include "bgl.hh"
#include "stardict.hh"
#include "lsa.hh"
#include "dsl.hh"
#include "mediawiki.hh"
#include "sounddir.hh"
#include "hunspell.hh"
#include "dictdfiles.hh"
#include "hotkeywrapper.hh"
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
  focusTranslateLineAction( this ),
  addTabAction( this ),
  closeCurrentTabAction( this ),
  switchToNextTabAction( this ),
  switchToPrevTabAction( this ),
  trayIconMenu( this ),
  addTab( this ),
  cfg( cfg_ ),
  articleMaker( dictionaries, groupInstances ),
  articleNetMgr( this, dictionaries, articleMaker ),
  dictNetMgr( this ),
  wordFinder( this ),
  initializing( 0 )
{
  ui.setupUi( this );

  // Make the toolbar
  navToolbar = addToolBar( tr( "Navigation" ) );
  navToolbar->setObjectName( "navToolbar" );

  navBack = navToolbar->addAction( QIcon( ":/icons/previous.png" ), tr( "Back" ) );
  navForward = navToolbar->addAction( QIcon( ":/icons/next.png" ), tr( "Forward" ) );

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

  addTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  addTabAction.setShortcut( QKeySequence( "Ctrl+T" ) );

  connect( &addTabAction, SIGNAL( triggered() ),
           this, SLOT( addNewTab() ) );

  ui.centralWidget->addAction( &addTabAction );

  closeCurrentTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  closeCurrentTabAction.setShortcut( QKeySequence( "Ctrl+W" ) );

  connect( &closeCurrentTabAction, SIGNAL( triggered() ),
           this, SLOT( closeCurrentTab() ) );

  ui.centralWidget->addAction( &closeCurrentTabAction );

  switchToNextTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  switchToNextTabAction.setShortcut( QKeySequence( "Ctrl+PgDown" ) );

  connect( &switchToNextTabAction, SIGNAL( triggered() ),
           this, SLOT( switchToNextTab() ) );

  ui.centralWidget->addAction( &switchToNextTabAction );

  switchToPrevTabAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  switchToPrevTabAction.setShortcut( QKeySequence( "Ctrl+PgUp" ) );

  connect( &switchToPrevTabAction, SIGNAL( triggered() ),
           this, SLOT( switchToPrevTab() ) );

  ui.centralWidget->addAction( &switchToPrevTabAction );

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

  ui.tabWidget->setMovable( true );
  #ifndef Q_OS_WIN32
  ui.tabWidget->setDocumentMode( true );
  #endif

  connect( &addTab, SIGNAL( clicked() ),
           this, SLOT( addNewTab() ) );

  connect( ui.tabWidget, SIGNAL( tabCloseRequested( int ) ),
           this, SLOT( tabCloseRequested( int ) ) );

  connect( ui.tabWidget, SIGNAL( currentChanged( int ) ),
           this, SLOT( tabSwitched( int ) ) );

  ui.tabWidget->setTabsClosable( true );

  connect( ui.quit, SIGNAL( activated() ),
           qApp, SLOT( quit() ) );

  connect( ui.sources, SIGNAL( activated() ),
           this, SLOT( editSources() ) );

  connect( ui.groups, SIGNAL( activated() ),
           this, SLOT( editGroups() ) );

  connect( ui.preferences, SIGNAL( activated() ),
           this, SLOT( editPreferences() ) );

  connect( ui.visitHomepage, SIGNAL( activated() ),
           this, SLOT( visitHomepage() ) );
  connect( ui.visitForum, SIGNAL( activated() ),
           this, SLOT( visitForum() ) );
  connect( ui.about, SIGNAL( activated() ),
           this, SLOT( showAbout() ) );

  connect( ui.groupList, SIGNAL( currentIndexChanged( QString const & ) ),
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

  // Update autostart info
  setAutostart(cfg.preferences.autoStart);

  // Initialize global hotkeys
  hotkeyWrapper = new HotkeyWrapper(this);
  hotkeyWrapper->setGlobalKey(Qt::Key_F11, Qt::Key_F11, Qt::ControlModifier, this,
                              SLOT(on_actionCloseToTray_activated()));

  // Shortcut for close to tray
  ui.actionCloseToTray->setShortcut(QKeySequence("Ctrl+F11"));

  // Only show window initially if it wasn't configured differently
  if ( !cfg.preferences.enableTrayIcon || !cfg.preferences.startToTray )
    show();
}

MainWindow::~MainWindow()
{
  // Save MainWindow state and geometry
  cfg.mainWindowState = saveState( 1 );
  cfg.mainWindowGeometry = saveGeometry();

  // Save any changes in last chosen groups etc
  Config::save( cfg );
}

LoadDictionaries::LoadDictionaries( Config::Class const & cfg ):
  paths( cfg.paths ), soundDirs( cfg.soundDirs ), hunspell( cfg.hunspell )
{
}

void LoadDictionaries::run()
{
  try
  {
    for( Config::Paths::const_iterator i = paths.begin(); i != paths.end(); ++i )
      handlePath( *i );

    // Make soundDirs
    {
      vector< sptr< Dictionary::Class > > soundDirDictionaries =
        SoundDir::makeDictionaries( soundDirs, Config::getIndexDir().toLocal8Bit().data(), *this );

      dictionaries.insert( dictionaries.end(), soundDirDictionaries.begin(),
                           soundDirDictionaries.end() );
    }

    // Make hunspells
    {
      vector< sptr< Dictionary::Class > > hunspellDictionaries =
        HunspellMorpho::makeDictionaries( hunspell );

      dictionaries.insert( dictionaries.end(), hunspellDictionaries.begin(),
                           hunspellDictionaries.end() );
    }

  }
  catch( std::exception & e )
  {
    exceptionText = e.what();
  }
}

void LoadDictionaries::handlePath( Config::Path const & path )
{
  vector< string > allFiles;

  QDir dir( path.path );

  QFileInfoList entries = dir.entryInfoList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot );

  for( QFileInfoList::const_iterator i = entries.constBegin();
       i != entries.constEnd(); ++i )
  {
    QString fullName = i->canonicalFilePath();

    if ( path.recursive && i->isDir() )
      handlePath( Config::Path( fullName, true ) );

    allFiles.push_back( QDir::toNativeSeparators( fullName ).toLocal8Bit().data() );
  }

  {
    vector< sptr< Dictionary::Class > > bglDictionaries =
      Bgl::makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );

    dictionaries.insert( dictionaries.end(), bglDictionaries.begin(),
                         bglDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > stardictDictionaries =
      Stardict::makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );

    dictionaries.insert( dictionaries.end(), stardictDictionaries.begin(),
                         stardictDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > lsaDictionaries =
      Lsa::makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );

    dictionaries.insert( dictionaries.end(), lsaDictionaries.begin(),
                         lsaDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > dslDictionaries =
      Dsl::makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );

    dictionaries.insert( dictionaries.end(), dslDictionaries.begin(),
                         dslDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > dictdDictionaries =
      DictdFiles::makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );

    dictionaries.insert( dictionaries.end(), dictdDictionaries.begin(),
                         dictdDictionaries.end() );
  }
}

void LoadDictionaries::indexingDictionary( string const & dictionaryName ) throw()
{
  emit indexingDictionarySignal( QString::fromUtf8( dictionaryName.c_str() ) );
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

  dictionaries.clear();

  ::Initializing init( this, isVisible() );

  try
  {
    initializing = &init;

    // Start a thread to load all the dictionaries

    LoadDictionaries loadDicts( cfg );

    connect( &loadDicts, SIGNAL( indexingDictionarySignal( QString ) ),
             this, SLOT( indexingDictionary( QString ) ) );

    QEventLoop localLoop;

    connect( &loadDicts, SIGNAL( finished() ),
             &localLoop, SLOT( quit() ) );

    loadDicts.start();

    localLoop.exec();

    loadDicts.wait();

    if ( loadDicts.getExceptionText().size() )
    {
      initializing = 0;

      QMessageBox::critical( this, tr( "Error loading dictionaries" ),
                             QString::fromUtf8( loadDicts.getExceptionText().c_str() ) );

      return;
    }

    dictionaries = loadDicts.getDictionaries();

    ///// We create MediaWiki dicts syncronously, since they use netmgr

    {
      vector< sptr< Dictionary::Class > > dicts =
        MediaWiki::makeDictionaries( loadDicts, cfg.mediawikis, dictNetMgr );

      dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
    }

    initializing = 0;

    // Remove any stale index files

    set< string > ids;

    for( unsigned x = dictionaries.size(); x--; )
      ids.insert( dictionaries[ x ]->getId() );

    QDir indexDir( Config::getIndexDir() );

    QStringList allIdxFiles = indexDir.entryList( QDir::Files );

    for( QStringList::const_iterator i = allIdxFiles.constBegin();
         i != allIdxFiles.constEnd(); ++i )
    {
      if ( ids.find( i->toLocal8Bit().data() ) == ids.end() &&
           i->size() == 32 )
        indexDir.remove( *i );
    }
  }
  catch( ... )
  {
    initializing = 0;
    throw;
  }

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

  ui.groupList->setVisible( haveGroups );

  ui.groupLabel->setText( haveGroups ? tr( "Look up in:" ) : tr( "Look up:" ) );

  // currentIndexChanged() signal is very trigger-happy. To avoid triggering
  // it, we disconnect it while we're clearing and filling back groups.
  disconnect( ui.groupList, SIGNAL( currentIndexChanged( QString const & ) ),
              this, SLOT( currentGroupChanged( QString const & ) ) );

  groupInstances.clear();

  for( unsigned x  = 0; x < cfg.groups.size(); ++x )
  {
    groupInstances.push_back( Instances::Group( cfg.groups[ x ], dictionaries ) );

    // Update names for dictionaries that are present, so that they could be
    // found in case they got moved.
    Instances::updateNames( cfg.groups[ x ], dictionaries );
  }

  ui.groupList->fill( groupInstances );
  ui.groupList->setCurrentGroup( cfg.lastMainGroupId );

  connect( ui.groupList, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );
}

void MainWindow::makeScanPopup()
{
  scanPopup.reset();

  if ( !cfg.preferences.enableScanPopup )
    return;

  scanPopup = new ScanPopup( 0, cfg, articleNetMgr, dictionaries, groupInstances );

  if ( enableScanPopup->isChecked() )
    scanPopup->enableScanning();
}

vector< sptr< Dictionary::Class > > const & MainWindow::getActiveDicts()
{
  if ( cfg.groups.empty() )
    return dictionaries;

  int current = ui.groupList->currentIndex();

  if ( current < 0 || current >= (int) groupInstances.size() )
  {
    // This shouldn't ever happen
    return dictionaries;
  }

  return groupInstances[ current ].dictionaries;
}

void MainWindow::indexingDictionary( QString dictionaryName )
{
  if ( initializing )
    initializing->indexing( dictionaryName );
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

  connect( view, SIGNAL( openLinkInNewTab( QUrl const &, QUrl const & ) ),
           this, SLOT( openLinkInNewTab( QUrl const &, QUrl const & ) ) );

  connect( view, SIGNAL( showDefinitionInNewTab( QString const &, unsigned ) ),
           this, SLOT( showDefinitionInNewTab( QString const &, unsigned ) ) );

  int index = cfg.preferences.newTabsOpenAfterCurrentOne ?
              ui.tabWidget->currentIndex() + 1 : ui.tabWidget->count();

  QString escaped = name;
  escaped.replace( "&", "&&" );

  ui.tabWidget->insertTab( index, view, escaped );

  if ( switchToIt )
    ui.tabWidget->setCurrentIndex( index );

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

void MainWindow::editSources()
{
  Sources src( this, cfg.paths, cfg.soundDirs, cfg.hunspell, cfg.mediawikis );

  src.show();

  if ( src.exec() == QDialog::Accepted )
  {
    cfg.paths = src.getPaths();
    cfg.soundDirs = src.getSoundDirs();
    cfg.hunspell = src.getHunspell();
    cfg.mediawikis = src.getMediaWikis();

    makeDictionaries();

    Config::save( cfg );
  }
}

void MainWindow::editGroups()
{
  Groups groups( this, dictionaries, cfg.groups );

  groups.show();

  if ( groups.exec() == QDialog::Accepted )
  {
    cfg.groups = groups.getGroups();

    Config::save( cfg );
  }
  else
    return;

  scanPopup.reset(); // It was holding group instances
  updateGroupList();
  makeScanPopup();
}

void MainWindow::editPreferences()
{
  Preferences preferences( this, cfg.preferences );

  preferences.show();

  if ( preferences.exec() == QDialog::Accepted )
  {
    cfg.preferences = preferences.getPreferences();

    enableScanPopup->setVisible( cfg.preferences.enableScanPopup );

    if ( !cfg.preferences.enableScanPopup )
      enableScanPopup->setChecked( false );

    updateTrayIcon();
    applyProxySettings();
    makeScanPopup();

    setAutostart(cfg.preferences.autoStart);

    Config::save( cfg );
  }
}

void MainWindow::currentGroupChanged( QString const & )
{
  cfg.lastMainGroupId = ui.groupList->getCurrentGroup();

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
    showTranslationFor( word );
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

      if ( keyEvent->matches( QKeySequence::InsertParagraphSeparator ) &&
           ui.wordList->selectedItems().size() )
      {
        dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) ).focus();

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

void MainWindow::openLinkInNewTab( QUrl const & url,
                                   QUrl const & referrer )
{
  createNewTab( !cfg.preferences.newTabsOpenInBackground, "" )->openLink( url, referrer );
}

void MainWindow::showDefinitionInNewTab( QString const & word,
                                         unsigned group )
{
  createNewTab( !cfg.preferences.newTabsOpenInBackground, word )->showDefinition( word, group );
}

void MainWindow::showTranslationFor( QString const & inWord )
{
  ArticleView & view =
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  navPronounce->setEnabled( false );

  view.showDefinition( inWord, cfg.groups.empty() ? 0 :
                        groupInstances[ ui.groupList->currentIndex() ].id );

  updatePronounceAvailability();

  #if 0
  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  req.addQueryItem( "word", inWord );
  req.addQueryItem( "group",
                    cfg.groups.empty() ? "" :
                      groupInstances[ ui.groupList->currentIndex() ].name );

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
    show();
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
