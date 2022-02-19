/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QDir>
#include <QFileInfo>
#include <QSplitter>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QLayout>
#include <QDesktopServices>

#include "helpwindow.hh"
#include "gddebug.hh"
#include <QHelpLink>

namespace Help {

HelpBrowser::HelpBrowser( QHelpEngineCore * engine, QWidget *parent ) :
    QTextBrowser( parent ),
    helpEngine( engine )
{
  connect( this, SIGNAL( anchorClicked( QUrl ) ), this, SLOT( linkClicked( QUrl ) ) );
}

void HelpBrowser::showHelpForKeyword( QString const & id )
{
  if ( helpEngine )
  {
    QList<QHelpLink> links = helpEngine->documentsForIdentifier(id);
    if (!links.isEmpty()) {
      QHelpLink link=links.constFirst();
      setSource(link.url);
    }
  }
}

QVariant HelpBrowser::loadResource( int type, QUrl const & name )
{
  QByteArray ba;
  if( type < 4 && helpEngine )
  {
    QUrl url(name);
    if( name.isRelative() )
      url = source().resolved( url );

     ba = helpEngine->fileData(url );
   }
   return ba;
}

void HelpBrowser::linkClicked( QUrl const & url )
{
  if( url.scheme() == "http" || url.scheme() == "https" )
  {
    QDesktopServices::openUrl( url );
  }
  else
    setSource( url );
}

HelpWindow::HelpWindow( QWidget * parent, Config::Class & cfg_ ) :
  QDialog( parent ),
  cfg( cfg_ ),
  helpEngine( 0 )
{
  resize( QSize( 600, 450 ) );
  setWindowTitle( tr( "GoldenDict help" ) );
  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

  QVBoxLayout * mainLayout = new QVBoxLayout( this );
  setLayout( mainLayout );

  navToolBar = new QToolBar( this );
  navHome = navToolBar->addAction( QIcon( ":/icons/home.svg" ), tr( "Home" ) );
  navToolBar->widgetForAction( navHome )->setObjectName( "helpHomeButton" );
  navBack = navToolBar->addAction( QIcon( ":/icons/previous.svg" ), tr( "Back" ) );
  navToolBar->widgetForAction( navBack )->setObjectName( "helpBackButton" );
  navForward = navToolBar->addAction( QIcon( ":/icons/next.svg" ), tr( "Forward" ) );
  navToolBar->widgetForAction( navForward )->setObjectName( "helpForwardButton" );

  navToolBar->addSeparator();

  zoomInAction = navToolBar->addAction( QIcon( ":/icons/icon32_zoomin.svg" ), tr( "Zoom In" ) );
  navToolBar->widgetForAction( zoomInAction )->setObjectName( "zoomInButton" );
  zoomOutAction = navToolBar->addAction( QIcon( ":/icons/icon32_zoomout.svg" ), tr( "Zoom Out" ) );
  navToolBar->widgetForAction( zoomInAction )->setObjectName( "zoomOutButton" );
  zoomBaseAction = navToolBar->addAction( QIcon( ":/icons/icon32_zoombase.svg" ), tr( "Normal Size" ) );
  navToolBar->widgetForAction( zoomBaseAction )->setObjectName( "zoomBaseButton" );

  navForward->setEnabled( false );
  navBack->setEnabled( false );

  mainLayout->addWidget( navToolBar );

  QString localeName = cfg.preferences.helpLanguage;

  if( localeName.isEmpty() )
    localeName = cfg.preferences.interfaceLanguage;

  if( localeName.isEmpty() )
    localeName = QLocale::system().name();

  QString helpDir = Config::getHelpDir();
  helpFile = QDir::toNativeSeparators( helpDir + "/gdhelp_" + localeName + ".qch" );

  if( !QFileInfo( helpFile ).isFile() )
    helpFile = QDir::toNativeSeparators( helpDir + "/gdhelp_" + localeName.left( 2 ) + ".qch" );

  if( !QFileInfo( helpFile ).isFile() )
    helpFile = QDir::toNativeSeparators( helpDir + "/gdhelp_en.qch" );

  helpCollectionFile = QDir::toNativeSeparators( Config::getConfigDir() + "gdhelp.qhc" );

  helpEngine = new QHelpEngine( helpCollectionFile );

  if( !helpEngine->setupData() )
  {
    gdWarning( "Help engine initialization error: %s", helpEngine->error().toUtf8().data() );
    delete helpEngine;
    helpEngine = 0;
  }
  else
  {
    if( !helpEngine->registerDocumentation( helpFile ) )
    {
      gdWarning( "Help engine set file error: %s", helpEngine->error().toUtf8().data() );
    }

    tabWidget = new QTabWidget( this );
    tabWidget->addTab( helpEngine->contentWidget(), tr( "Content" ) );
    tabWidget->addTab( helpEngine->indexWidget(), tr( "Index" ) );

    helpBrowser = new HelpBrowser( helpEngine, this );

    helpBrowser->setOpenLinks( false );

    connect( helpEngine->contentWidget(), SIGNAL( linkActivated( QUrl ) ),
             helpBrowser, SLOT( setSource( QUrl ) ) );
    connect( helpEngine->indexWidget(), SIGNAL( linkActivated( QUrl, QString ) ),
             helpBrowser, SLOT( setSource( QUrl ) ) );

    connect( navHome, SIGNAL( triggered() ), helpBrowser, SLOT( home() ) );
    connect( navForward, SIGNAL( triggered() ), helpBrowser, SLOT( forward() ) );
    connect( navBack, SIGNAL( triggered() ), helpBrowser, SLOT( backward() ) );

    connect( helpBrowser, SIGNAL( forwardAvailable( bool ) ),
             this, SLOT( forwardEnabled( bool ) ) );

    connect( helpBrowser, SIGNAL( backwardAvailable( bool ) ),
             this, SLOT( backwardEnabled( bool ) ) );

    connect( helpEngine->contentWidget(), SIGNAL( clicked( QModelIndex ) ),
             this, SLOT( contentsItemClicked( QModelIndex ) ) );

    connect( zoomInAction, SIGNAL( triggered( ) ), this, SLOT( zoomIn() ) );
    connect( zoomOutAction, SIGNAL( triggered( ) ), this, SLOT( zoomOut() ) );
    connect( zoomBaseAction, SIGNAL( triggered( ) ), this, SLOT( zoomBase() ) );

    splitter = new QSplitter( this );
    splitter->addWidget( tabWidget );
    splitter->addWidget( helpBrowser );

    splitter->setStretchFactor( 0, 1 );
    splitter->setStretchFactor( 1, 4 );
    mainLayout->addWidget( splitter );
  }

  if( !cfg.helpWindowGeometry.isEmpty() )
    restoreGeometry( cfg.helpWindowGeometry );
  if( !cfg.helpSplitterState.isEmpty() )
    splitter->restoreState( cfg.helpSplitterState );

  QFont f = helpBrowser->font();
  fontSize = f.pointSize();
  if( fontSize < 10 )
  {
    fontSize = 10;
    f.setPointSize( fontSize );
    helpBrowser->setFont( f );
  }

  applyZoomFactor();
}

HelpWindow::~HelpWindow()
{
  if( helpEngine )
    delete helpEngine;

  QFile f( helpCollectionFile );
  f.remove();
}

void HelpWindow::reject()
{
  cfg.helpWindowGeometry = saveGeometry();
  cfg.helpSplitterState = splitter->saveState();
  emit needClose();
}

void HelpWindow::accept()
{
  cfg.helpWindowGeometry = saveGeometry();
  cfg.helpSplitterState = splitter->saveState();
  emit needClose();
}

void HelpWindow::showHelpFor( QString const & keyword )
{
  helpBrowser->showHelpForKeyword( keyword );
}

void HelpWindow::forwardEnabled( bool enabled )
{
  navForward->setEnabled( enabled );
}

void HelpWindow::backwardEnabled( bool enabled )
{
  navBack->setEnabled( enabled );
}

void HelpWindow::contentsItemClicked( QModelIndex const & index )
{
  QHelpContentItem * item = helpEngine->contentModel()->contentItemAt( index );
  if( !item->url().isEmpty() )
    helpBrowser->setSource( item->url() );
}

void HelpWindow::zoomIn()
{
  cfg.preferences.helpZoomFactor += 0.2;
  applyZoomFactor();
}

void HelpWindow::zoomOut()
{
  cfg.preferences.helpZoomFactor -= 0.2;
  applyZoomFactor();
}

void HelpWindow::zoomBase()
{
  cfg.preferences.helpZoomFactor = 1;
  applyZoomFactor();
}

void HelpWindow::applyZoomFactor()
{
  if ( cfg.preferences.helpZoomFactor >= 5 )
    cfg.preferences.helpZoomFactor = 5;
  else if ( cfg.preferences.helpZoomFactor <= 0.2 )
    cfg.preferences.helpZoomFactor = 0.2;

  zoomInAction->setEnabled( cfg.preferences.helpZoomFactor < 5 );
  zoomOutAction->setEnabled( cfg.preferences.helpZoomFactor > 0.2 );
  zoomBaseAction->setEnabled( !qFuzzyCompare(cfg.preferences.helpZoomFactor, 1.0) );

  if( fontSize > 0 )
  {
    QFont f = helpBrowser->font();
    f.setPointSize( fontSize * cfg.preferences.helpZoomFactor );
    helpBrowser->setFont( f );
  }
}

} // namespace Help
