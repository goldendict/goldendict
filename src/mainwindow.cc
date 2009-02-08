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
#include "dictlock.hh"
#include "ui_about.h"
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

MainWindow::MainWindow(): 
  trayIcon( 0 ),
  trayIconMenu( this ),
  addTab( this ),
  cfg( Config::load() ),
  articleMaker( dictionaries, groupInstances ),
  articleNetMgr( this, dictionaries, articleMaker ),
  wordFinder( this ),
  initializing( 0 )
{
  ui.setupUi( this );

  // Make the toolbar
  navToolbar = addToolBar( tr( "Navigation" ) );
  navBack = navToolbar->addAction( QIcon( ":/icons/previous.png" ), tr( "Back" ) );
  navForward = navToolbar->addAction( QIcon( ":/icons/next.png" ), tr( "Forward" ) );
  
  enableScanPopup = navToolbar->addAction( QIcon( ":/icons/wizard.png" ), tr( "Scan Popup" ) );
  enableScanPopup->setCheckable( true );
  enableScanPopup->setVisible( cfg.preferences.enableScanPopup );
  if ( cfg.preferences.enableScanPopup && cfg.preferences.startWithScanPopupOn )
    enableScanPopup->setChecked( true );

  connect( enableScanPopup, SIGNAL( toggled( bool ) ),
           this, SLOT( scanEnableToggled( bool ) ) );

  connect( trayIconMenu.addAction( tr( "Show &Main Window" ) ), SIGNAL( activated() ),
           this, SLOT( showMainWindow() ) );
  trayIconMenu.addAction( enableScanPopup );
  trayIconMenu.addSeparator();
  connect( trayIconMenu.addAction( tr( "&Quit" ) ), SIGNAL( activated() ),
           qApp, SLOT( quit() ) );
  
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
  ui.tabWidget->setDocumentMode( true );

  connect( &addTab, SIGNAL( clicked() ),
           this, SLOT( addNewTab() ) );

  connect( ui.tabWidget, SIGNAL( tabCloseRequested( int ) ),
           this, SLOT( tabCloseRequested( int ) ) );

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

  connect( ui.wordList, SIGNAL( itemActivated( QListWidgetItem * ) ),
           this, SLOT( wordListItemActivated( QListWidgetItem * ) ) );
  
  connect( ui.wordList, SIGNAL( itemSelectionChanged() ),
           this, SLOT( wordListSelectionChanged() ) );
  
  connect( wordFinder.qobject(), SIGNAL( prefixMatchComplete( WordFinderResults ) ),
           this, SLOT( prefixMatchComplete( WordFinderResults ) ) );

  makeDictionaries();

  addNewTab();

  // Show the initial welcome text

  {
    ArticleView & view =
      dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

    view.showDefinition( "Welcome!", "internal:about" );
  }
  
  ui.translateLine->setFocus();

  updateTrayIcon();

  // Only show window initially if it wasn't configured differently
  if ( !cfg.preferences.enableTrayIcon || !cfg.preferences.startToTray )
    show();
}

MainWindow::~MainWindow()
{
  // Save any changes in last chosen groups etc
  Config::save( cfg );
}

LoadDictionaries::LoadDictionaries( vector< string > const & allFiles_ ):
  allFiles( allFiles_ )
{
}

void LoadDictionaries::run()
{
  {
    Bgl::Format bglFormat;
  
    dictionaries = bglFormat.makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );
  }

  {
    Stardict::Format stardictFormat;
  
    vector< sptr< Dictionary::Class > > stardictDictionaries =
      stardictFormat.makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );
  
    dictionaries.insert( dictionaries.end(), stardictDictionaries.begin(),
                         stardictDictionaries.end() );
  }

  {
    Lsa::Format lsaFormat;

    vector< sptr< Dictionary::Class > > lsaDictionaries =
      lsaFormat.makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );

    dictionaries.insert( dictionaries.end(), lsaDictionaries.begin(),
                         lsaDictionaries.end() );
  }

  {
    Dsl::Format dslFormat;

    vector< sptr< Dictionary::Class > > dslDictionaries =
      dslFormat.makeDictionaries( allFiles, Config::getIndexDir().toLocal8Bit().data(), *this );

    dictionaries.insert( dictionaries.end(), dslDictionaries.begin(),
                         dslDictionaries.end() );
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

void MainWindow::makeDictionaries()
{
  {
    DictLock _;
    dictionaries.clear();
  }

  ::Initializing init( this );

  try
  {
    initializing = &init;

    // Traverse through known directories in search for the files

    vector< string > allFiles;

    for( Config::Paths::const_iterator i = cfg.paths.begin();
         i != cfg.paths.end(); ++i )
    {
      QDir dir( *i );

      QStringList entries = dir.entryList();

      for( QStringList::const_iterator i = entries.constBegin();
           i != entries.constEnd(); ++i )
        allFiles.push_back( QDir::toNativeSeparators( dir.filePath( *i ) ).toLocal8Bit().data() );
    }

    // Now start a thread to load all the dictionaries

    LoadDictionaries loadDicts( allFiles );

    connect( &loadDicts, SIGNAL( indexingDictionarySignal( QString ) ),
             this, SLOT( indexingDictionary( QString ) ) );

    QEventLoop localLoop;

    connect( &loadDicts, SIGNAL( finished() ),
             &localLoop, SLOT( quit() ) );

    loadDicts.start();

    localLoop.exec();

    loadDicts.wait();

    {
      DictLock _;

      dictionaries = loadDicts.getDictionaries();
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

  {
    DictLock _;
  
    groupInstances.clear();
  
    for( unsigned x  = 0; x < cfg.groups.size(); ++x )
      groupInstances.push_back( Instances::Group( cfg.groups[ x ], dictionaries ) );
  }

  ui.groupList->fill( groupInstances );
  ui.groupList->setCurrentGroup( cfg.lastMainGroup );

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
  ArticleView * view = new ArticleView( this, articleNetMgr, groupInstances,
                                        false );

  connect( view, SIGNAL( titleChanged(  ArticleView *, QString const & ) ),
           this, SLOT( titleChanged(  ArticleView *, QString const & ) ) );

  connect( view, SIGNAL( iconChanged( ArticleView *, QIcon const & ) ),
           this, SLOT( iconChanged( ArticleView *, QIcon const & ) ) );

  connect( view, SIGNAL( openLinkInNewTab( QUrl const &, QUrl const & ) ),
           this, SLOT( openLinkInNewTab( QUrl const &, QUrl const & ) ) );
  
  connect( view, SIGNAL( showDefinitionInNewTab( QString const &, QString const & ) ),
           this, SLOT( showDefinitionInNewTab( QString const &, QString const & ) ) );
  
  ui.tabWidget->addTab( view, tr( "(untitled)" ) );

  ui.tabWidget->setCurrentIndex( ui.tabWidget->count() - 1 );
}

void MainWindow::tabCloseRequested( int x )
{
  if ( ui.tabWidget->count() < 2 )
    return; // We should always have at least one open tab

  QWidget * w = ui.tabWidget->widget( x );

  ui.tabWidget->removeTab( x );

  delete w;
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
  ui.tabWidget->setTabText( ui.tabWidget->indexOf( view ), title );
}

void MainWindow::iconChanged( ArticleView * view, QIcon const & icon )
{
  ui.tabWidget->setTabIcon( ui.tabWidget->indexOf( view ), icon );
}

void MainWindow::editSources()
{
  Sources src( this, cfg.paths );

  src.show();

  if ( src.exec() == QDialog::Accepted )
  {
    cfg.paths = src.getPaths();

    makeDictionaries();

    Config::save( cfg );
  }
}

void MainWindow::editGroups()
{
  {
    // We lock all dictionaries during the entire group editing process, since
    // the dictionaries might get queried for various infos there
    DictLock _;
  
    Groups groups( this, dictionaries, cfg.groups );
  
    groups.show();

    if ( groups.exec() == QDialog::Accepted )
    {
      cfg.groups = groups.getGroups();

      Config::save( cfg );
    }
    else
      return;
  }

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
    makeScanPopup();
    Config::save( cfg );
  }
}

void MainWindow::currentGroupChanged( QString const & gr )
{
  cfg.lastMainGroup = gr;

  // Update word search results

  translateInputChanged( ui.translateLine->text() );
}

void MainWindow::translateInputChanged( QString const & newValue )
{
  QString req = newValue.trimmed();

  if ( !req.size() )
  {
    // An empty request always results in an empty result
    prefixMatchComplete( WordFinderResults( req, &getActiveDicts() ) );

    return;
  }

  ui.wordList->setCursor( Qt::WaitCursor );

  wordFinder.prefixMatch( req, &getActiveDicts() );
}

void MainWindow::translateInputFinished()
{
  if ( ui.wordList->count() )
    wordListItemActivated( ui.wordList->item( 0 ) );
}

void MainWindow::prefixMatchComplete( WordFinderResults r )
{
  if ( r.requestStr != ui.translateLine->text().trimmed() ||
      r.requestDicts != &getActiveDicts() )
  {
    // Those results are already irrelevant, ignore the result
    return;
  }

  ui.wordList->setUpdatesEnabled( false );

  for( unsigned x = 0; x < r.results.size(); ++x )
  {
    QListWidgetItem * i = ui.wordList->item( x );

    if ( !i )
      ui.wordList->addItem( r.results[ x ] );
    else
    if ( i->text() != r.results[ x ] )
      i->setText( r.results[ x ] );
  }

  while ( ui.wordList->count() > (int) r.results.size() )
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
  ui.wordList->unsetCursor();
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
  addNewTab();

  ArticleView & view =
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  view.openLink( url, referrer );
}

void MainWindow::showDefinitionInNewTab( QString const & word,
                                         QString const & group )
{
  addNewTab();

  ArticleView & view =
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  view.showDefinition( word, group );
}

void MainWindow::showTranslationFor( QString const & inWord )
{
  ArticleView & view =
    dynamic_cast< ArticleView & >( *( ui.tabWidget->currentWidget() ) );

  view.showDefinition( inWord, cfg.groups.empty() ? "" :
                        groupInstances[ ui.groupList->currentIndex() ].name );

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

void MainWindow::trayIconActivated( QSystemTrayIcon::ActivationReason r )
{
  if ( r == QSystemTrayIcon::DoubleClick )
  {
    // Double-click toggles the visibility of main window
    if ( !isVisible() )
      show();
    else
    if ( isMinimized() )
    {
      showNormal();
      activateWindow();
    }
    else
      hide();
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
  if ( !isVisible() )
    show();
  else
  if ( isMinimized() )
  {
    showNormal();
    activateWindow();
  }
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

  about.show();
  about.exec();
}

