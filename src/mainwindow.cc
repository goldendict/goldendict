/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mainwindow.hh"
#include "sources.hh"
#include "groups.hh"
#include "bgl.hh"
#include "stardict.hh"
#include "lsa.hh"
#include "dsl.hh"
#include <QDir>
#include <QMessageBox>
#include <QIcon>
#include <QToolBar>
#include <set>
#include <map>

using std::set;
using std::wstring;
using std::map;
using std::pair;

MainWindow::MainWindow(): 
  addTab( this ),
  cfg( Config::load() ),
  articleMaker( dictionaries, groupInstances ),
  articleNetMgr( this, dictionaries, articleMaker ),
  scanPopup( 0, articleNetMgr ),
  startLookupTimeout( this ),
  initializing( 0 )
{
  ui.setupUi( this );

  // Make the toolbar
  navToolbar = addToolBar( tr( "Navigation" ) );
  navBack = navToolbar->addAction( QIcon( ":/icons/previous.png" ), tr( "Back" ) );
  navForward = navToolbar->addAction( QIcon( ":/icons/next.png" ), tr( "Forward" ) );

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

  startLookupTimeout.setSingleShot( true );

  ui.tabWidget->setTabsClosable( true );

  connect( ui.sources, SIGNAL( activated() ),
           this, SLOT( editSources() ) );

  connect( ui.groups, SIGNAL( activated() ),
           this, SLOT( editGroups() ) );

  connect( ui.translateLine, SIGNAL( textChanged( QString const & ) ),
           this, SLOT( translateInputChanged( QString const & ) ) );

  connect( &startLookupTimeout, SIGNAL( timeout() ),
           this, SLOT( startLookup() ) );

  connect( ui.wordList, SIGNAL( itemActivated( QListWidgetItem * ) ),
           this, SLOT( wordListItemActivated( QListWidgetItem * ) ) );

  makeDictionaries();

  addNewTab();
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

void MainWindow::makeDictionaries()
{
  dictionaries.clear();

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
        allFiles.push_back( dir.filePath( *i ).toLocal8Bit().data() );
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

    dictionaries = loadDicts.getDictionaries();

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

  ui.groupList->clear();

  groupInstances.clear();

  for( unsigned x  = 0; x < cfg.groups.size(); ++x )
  {
    groupInstances.push_back( Instances::Group( cfg.groups[ x ], dictionaries ) );

    QIcon icon = cfg.groups[ x ].icon.size() ?
                   QIcon( ":/flags/" + cfg.groups[ x ].icon ) : QIcon();

    ui.groupList->addItem( icon, cfg.groups[ x ].name );
  }

  if ( haveGroups )
    ui.groupList->setCurrentIndex( 0 );
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
  Groups groups( this, dictionaries, cfg.groups );

  groups.show();

  if ( groups.exec() == QDialog::Accepted )
  {
    cfg.groups = groups.getGroups();

    Config::save( cfg );

    updateGroupList();
  }
}

void MainWindow::translateInputChanged( QString const & )
{
  startLookup();
  //startLookupTimeout.start( 0 );
}

void MainWindow::startLookup()
{
  QString word = ui.translateLine->text();

  ui.wordList->clear();

  wstring wordW = word.trimmed().toStdWString();

  if ( wordW.empty() )
    return;

  // Maps lowercased string to the original one. This catches all duplicates
  // without case sensitivity
  map< wstring, wstring > exactResults, prefixResults;

  // Where to look?

  vector< sptr< Dictionary::Class > > const & activeDicts = getActiveDicts();

  for( unsigned x = 0; x < activeDicts.size(); ++x )
  {
    vector< wstring > exactMatches, prefixMatches;

    activeDicts[ x ]->findExact( wordW, exactMatches, prefixMatches, 200 );

    for( unsigned y = 0; y < exactMatches.size(); ++y )
    {
      wstring lowerCased = Folding::applySimpleCaseOnly( exactMatches[ y ] );

      pair< map< wstring, wstring >::iterator, bool > insertResult =
        exactResults.insert( pair< wstring, wstring >( lowerCased, exactMatches[ y ] ) );

      if ( !insertResult.second )
      {
        // Wasn't inserted since there was already an item -- check the case
        if ( insertResult.first->second != exactMatches[ y ] )
        {
          // The case is different -- agree on a lowercase version
          insertResult.first->second = lowerCased;
        }
      }
    }

    for( unsigned y = 0; y < prefixMatches.size(); ++y )
    {
      wstring lowerCased = Folding::applySimpleCaseOnly( prefixMatches[ y ] );

      pair< map< wstring, wstring >::iterator, bool > insertResult =
        prefixResults.insert( pair< wstring, wstring >( lowerCased, prefixMatches[ y ] ) );

      if ( !insertResult.second )
      {
        // Wasn't inserted since there was already an item -- check the case
        if ( insertResult.first->second != prefixMatches[ y ] )
        {
          // The case is different -- agree on a lowercase version
          insertResult.first->second = lowerCased;
        }
      }
    }
  }

  // Do any sort of collation here in the future. For now we just put the
  // strings sorted by the map.

  for( map< wstring, wstring >::const_iterator i = exactResults.begin();
       i != exactResults.end(); ++i )
  {
    ui.wordList->addItem( QString::fromStdWString( i->second ) );

    QListWidgetItem * item = ui.wordList->item( ui.wordList->count() - 1 );

    QFont font = item->font();
    //font.setWeight( QFont::Bold );

    item->setFont( font );
  }

  if ( prefixResults.size() )
  {
    for( map< wstring, wstring >::const_iterator i = prefixResults.begin();
         i != prefixResults.end(); ++i )
    {
        ui.wordList->addItem( QString::fromStdWString( i->second ) );

        QListWidgetItem * item = ui.wordList->item( ui.wordList->count() - 1 );

        QFont font = item->font();
        //font.setStyle( QFont::StyleOblique );

        item->setFont( font );
    }
  }
}

void MainWindow::wordListItemActivated( QListWidgetItem * item )
{
  printf( "act: %s\n", item->text().toLocal8Bit().data() );

  showTranslationFor( item->text() );
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
