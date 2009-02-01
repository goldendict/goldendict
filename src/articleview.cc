/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articleview.hh"
#include "externalviewer.hh"
#include <QMessageBox>
#include <QWebHitTestResult>
#include <QMenu>
#include <QDesktopServices>


ArticleView::ArticleView( QWidget * parent, ArticleNetworkAccessManager & nm,
                          Instances::Groups const & groups_, bool popupView_ ):
  QFrame( parent ),
  articleNetMgr( nm ),
  groups( groups_ ),
  popupView( popupView_ )
{
  ui.setupUi( this );

  ui.definition->setContextMenuPolicy( Qt::CustomContextMenu );

  ui.definition->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

  ui.definition->page()->setNetworkAccessManager( &articleNetMgr );

  connect( ui.definition, SIGNAL( titleChanged( QString const & ) ),
           this, SLOT( handleTitleChanged( QString const & ) ) );

  connect( ui.definition, SIGNAL( urlChanged( QUrl const & ) ),
           this, SLOT( handleUrlChanged( QUrl const & ) ) );

  connect( ui.definition, SIGNAL( customContextMenuRequested( QPoint const & ) ),
           this, SLOT( contextMenuRequested( QPoint const & ) ) );

  connect( ui.definition, SIGNAL( linkClicked( QUrl const & ) ),
           this, SLOT( linkClicked( QUrl const & ) ) );
}

void ArticleView::showDefinition( QString const & word, QString const & group )
{
  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  req.addQueryItem( "word", word );
  req.addQueryItem( "group", group );

  ui.definition->load( req );
}

void ArticleView::showNotFound( QString const & word, QString const & group )
{
  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  req.addQueryItem( "word", word );
  req.addQueryItem( "group", group );
  req.addQueryItem( "notfound", "1" );

  ui.definition->load( req );
}

void ArticleView::handleTitleChanged( QString const & title )
{
  emit titleChanged( this, title );
}

void ArticleView::handleUrlChanged( QUrl const & url )
{
  QIcon icon;

  QString group = getGroup( url );

  if ( group.size() )
  {
    // Find the group's instance corresponding to the fragment value
    for( unsigned x = 0; x < groups.size(); ++x )
      if ( groups[ x ].name == group )
      {
        // Found it

        if ( groups[ x ].icon.size() )
          icon = QIcon( ":/flags/" + groups[ x ].icon );
        else
          break;
      }
  }

  emit iconChanged( this, icon );
}

QString ArticleView::getGroup( QUrl const & url )
{
  if ( url.scheme() == "gdlookup" && url.hasQueryItem( "group" ) )
    return url.queryItemValue( "group" );

  return QString();
}


void ArticleView::linkClicked( QUrl const & url )
{
  printf( "clicked %s\n", url.toString().toLocal8Bit().data() );

  if ( url.scheme() == "bword" )
    showDefinition( url.host().startsWith( "xn--" ) ?
                      QUrl::fromPunycode( url.host().toLatin1() ) :
                      url.host(),
                    getGroup( ui.definition->url() ) );
  else
  if ( url.scheme() == "gdlookup" ) // Plain html links inherit gdlookup scheme
    showDefinition( url.path().mid( 1 ),
                    getGroup( ui.definition->url() ) );
  else
  if ( url.scheme() == "bres" || url.scheme() == "gdau" )
  {
    vector< char > data;

    // Download it

    QString contentType;

    if ( !articleNetMgr.getResource( url, data, contentType ) )
    {
      QMessageBox::critical( this, tr( "GoldenDict" ), tr( "The referenced resource doesn't exist." ) );
      return;
    }

    // Decide the viewer

    QString program, extension;

    if ( url.scheme() == "gdau" )
    {
      program = "mplayer";
      extension = "wav";
    }
    else
    if ( url.path().endsWith( ".pdf", Qt::CaseInsensitive ) )
    {
      program = "evince";
      extension = "pdf";
    }
    else
    if ( url.path().endsWith( ".rtf", Qt::CaseInsensitive ) )
    {
      program = "oowriter";
      extension = "rtf";
    }
    else
    {
      QMessageBox::critical( this, tr( "GoldenDict" ), tr( "Don't know how to handle the specified resource." ) );
      return;
    }

    try
    {
      ExternalViewer * viewer = new ExternalViewer( this, data, extension, program );

      try
      {
        viewer->start();

        // Once started, it will erase itself
      }
      catch( ... )
      {
        delete viewer;
        throw;
      }
    }
    catch( ExternalViewer::Ex & e )
    {
      printf( "%s\n", e.what() );
    }
  }
  else
  if ( url.scheme() == "http" || url.scheme() == "https" ||
      url.scheme() == "ftp" || url.scheme() == "mailto" )
  {
    // Use the system handler for the conventional internet links
    QDesktopServices::openUrl( url );
  }
}

void ArticleView::contextMenuRequested( QPoint const & pos )
{
  // Is that a link? Is there a selection?

  QWebHitTestResult r = ui.definition->page()->currentFrame()->
                          hitTestContent( pos );

  QMenu menu( this );


  QAction * followLink = 0;
  QAction * lookupSelection = 0;

  if ( !r.linkUrl().isEmpty() )
  {
    followLink = new QAction( tr( "Open the link" ), &menu );
    menu.addAction( followLink );
  }

  QString selectedText = ui.definition->selectedText();
  if ( selectedText.size() )
  {
    lookupSelection = new QAction( tr( "Look up \"%1\"" ).arg( ui.definition->selectedText() ), &menu );
    menu.addAction( lookupSelection );
  }

  if ( !menu.isEmpty() )
  {
    QAction * result = menu.exec( ui.definition->mapToGlobal( pos ) );

    if ( result == followLink )
      linkClicked( r.linkUrl() );
    else
    if ( result == lookupSelection )
      showDefinition( selectedText, getGroup( ui.definition->url() ) );
  }
#if 0
  printf( "%s\n", r.linkUrl().isEmpty() ? "null" : "not null" );

  printf( "url = %s\n", r.linkUrl().toString().toLocal8Bit().data() );
  printf( "title = %s\n", r.title().toLocal8Bit().data() );
#endif
}

void ArticleView::showEvent( QShowEvent * ev )
{
  QFrame::showEvent( ev );

  ui.searchFrame->hide();
}
