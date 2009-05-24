/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articleview.hh"
#include "externalviewer.hh"
#include <map>
#include <QMessageBox>
#include <QWebHitTestResult>
#include <QMenu>
#include <QDesktopServices>
#include <QWebHistory>
#include <QClipboard>
#include <QKeyEvent>
#include "folding.hh"
#include "wstring_qt.hh"

#ifdef Q_OS_WIN32
#include <windows.h>
#include <mmsystem.h> // For PlaySound
#endif

using std::map;
using std::list;

ArticleView::ArticleView( QWidget * parent, ArticleNetworkAccessManager & nm,
                          std::vector< sptr< Dictionary::Class > > const & allDictionaries_,
                          Instances::Groups const & groups_, bool popupView_,
                          Config::Class const & cfg_,
                          GroupComboBox const * groupComboBox_ ):
  QFrame( parent ),
  articleNetMgr( nm ),
  allDictionaries( allDictionaries_ ),
  groups( groups_ ),
  popupView( popupView_ ),
  cfg( cfg_ ),
  pasteAction( this ),
  articleUpAction( this ),
  articleDownAction( this ),
  openSearchAction( this ),
  searchIsOpened( false ),
  groupComboBox( groupComboBox_ )
{
  ui.setupUi( this );

  ui.definition->pageAction( QWebPage::Back )->setShortcut( QKeySequence( "Alt+Left" ) );
  ui.definition->addAction( ui.definition->pageAction( QWebPage::Back ) );

  ui.definition->pageAction( QWebPage::Forward )->setShortcut( QKeySequence( "Alt+Right" ) );
  ui.definition->addAction( ui.definition->pageAction( QWebPage::Forward ) );

  ui.definition->pageAction( QWebPage::Copy )->setShortcut( QKeySequence::Copy );
  ui.definition->addAction( ui.definition->pageAction( QWebPage::Copy ) );

  ui.definition->setContextMenuPolicy( Qt::CustomContextMenu );

  ui.definition->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

  ui.definition->page()->setNetworkAccessManager( &articleNetMgr );

  connect( ui.definition, SIGNAL( loadFinished( bool ) ),
           this, SLOT( loadFinished( bool ) ) );
  
  connect( ui.definition, SIGNAL( titleChanged( QString const & ) ),
           this, SLOT( handleTitleChanged( QString const & ) ) );

  connect( ui.definition, SIGNAL( urlChanged( QUrl const & ) ),
           this, SLOT( handleUrlChanged( QUrl const & ) ) );

  connect( ui.definition, SIGNAL( customContextMenuRequested( QPoint const & ) ),
           this, SLOT( contextMenuRequested( QPoint const & ) ) );

  connect( ui.definition, SIGNAL( linkClicked( QUrl const & ) ),
           this, SLOT( linkClicked( QUrl const & ) ) );

  pasteAction.setShortcut( QKeySequence::Paste  );
  ui.definition->addAction( &pasteAction );
  connect( &pasteAction, SIGNAL( triggered() ), this, SLOT( pasteTriggered() ) );

  articleUpAction.setShortcut( QKeySequence( "Alt+Up" ) );
  ui.definition->addAction( &articleUpAction );
  connect( &articleUpAction, SIGNAL( triggered() ), this, SLOT( moveOneArticleUp() ) );

  articleDownAction.setShortcut( QKeySequence( "Alt+Down" ) );
  ui.definition->addAction( &articleDownAction );
  connect( &articleDownAction, SIGNAL( triggered() ), this, SLOT( moveOneArticleDown() ) );

  openSearchAction.setShortcut( QKeySequence( "Ctrl+F" ) );
  ui.definition->addAction( &openSearchAction );
  connect( &openSearchAction, SIGNAL( triggered() ), this, SLOT( openSearch() ) );

  ui.definition->installEventFilter( this );

  // Load the default blank page instantly, so there would be no flicker.

  QString contentType;
  QUrl blankPage( "gdlookup://localhost?blank=1" );

  sptr< Dictionary::DataRequest > r = articleNetMgr.getResource( blankPage,
                                                                 contentType );

  ui.definition->setHtml( QByteArray( &( r->getFullData().front() ),
                                      r->getFullData().size() ), blankPage );
}

void ArticleView::setGroupComboBox( GroupComboBox const * g )
{
  groupComboBox = g;
}

ArticleView::~ArticleView()
{
  cleanupTemp();
  
  #ifdef Q_OS_WIN32
  if ( winWavData.size() )
  {
    // If we were playing some sound some time ago, make sure it stopped
    // playing before freeing the waveform memory.
    PlaySoundA( 0, 0, 0 );
  }
  #endif
}

void ArticleView::showDefinition( QString const & word, unsigned group,
                                  QString const & scrollTo )
{
  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  req.addQueryItem( "word", word );
  req.addQueryItem( "group", QString::number( group ) );

  if ( scrollTo.size() )
    req.setFragment( scrollTo );

  // Save current article, if any

  QString currentArticle = getCurrentArticle();

  if ( currentArticle.size() )
    ui.definition->history()->currentItem().setUserData( currentArticle );

  ui.definition->load( req );

  //QApplication::setOverrideCursor( Qt::WaitCursor );
  ui.definition->setCursor( Qt::WaitCursor );
}

void ArticleView::showAnticipation()
{
  ui.definition->setHtml( "" );
  ui.definition->setCursor( Qt::WaitCursor );
  //QApplication::setOverrideCursor( Qt::WaitCursor );
}

void ArticleView::loadFinished( bool )
{
  QUrl url = ui.definition->url();

  QVariant userData = ui.definition->history()->currentItem().userData();

  if ( userData.type() == QVariant::String && userData.toString().startsWith( "gdfrom-" ) )
  {
    printf( "has user data\n" );
    // There's an active article saved, so set it to be active.
    setCurrentArticle( userData.toString() );
  }
  else
  if ( url.hasFragment() && url.fragment().startsWith( "gdfrom-" ) )
  {
    // There is no active article saved in history, but we have it in fragment.
    // setCurrentArticle will save it.
    setCurrentArticle( url.fragment() );
  }

  // See if we have any iframes in need of expansion

  QList< QWebFrame * > frames = ui.definition->page()->mainFrame()->childFrames();

  bool wereFrames = false;

  for( QList< QWebFrame * >::iterator i = frames.begin(); i != frames.end(); ++i )
  {
    if ( (*i)->frameName().startsWith( "gdexpandframe-" ) )
    {
      //printf( "Name: %s\n", (*i)->frameName().toUtf8().data() );
      //printf( "Size: %d\n", (*i)->contentsSize().height() );
      //printf( ">>>>>>>>Height = %s\n", (*i)->evaluateJavaScript( "document.body.offsetHeight;" ).toString().toUtf8().data() );
  
      // Set the height
      ui.definition->page()->mainFrame()->evaluateJavaScript( QString( "document.getElementById('%1').height = %2;" ).
        arg( (*i)->frameName() ).
        arg( (*i)->contentsSize().height() ) );

      // Show it
      ui.definition->page()->mainFrame()->evaluateJavaScript( QString( "document.getElementById('%1').style.display = 'block';" ).
        arg( (*i)->frameName() ) );

      wereFrames = true;
    }
  }

  if ( wereFrames )
  {
    // There's some sort of glitch -- sometimes you need to move a mouse

    QMouseEvent ev( QEvent::MouseMove, QPoint(), Qt::MouseButton(), 0, 0 );

    qApp->sendEvent( ui.definition, &ev );
  }

  ui.definition->unsetCursor();
  //QApplication::restoreOverrideCursor();
  emit pageLoaded( this );
}

void ArticleView::handleTitleChanged( QString const & title )
{
  emit titleChanged( this, title );
}

void ArticleView::handleUrlChanged( QUrl const & url )
{
  QIcon icon;

  unsigned group = getGroup( url );

  if ( group )
  {
    // Find the group's instance corresponding to the fragment value
    for( unsigned x = 0; x < groups.size(); ++x )
      if ( groups[ x ].id == group )
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

unsigned ArticleView::getGroup( QUrl const & url )
{
  if ( url.scheme() == "gdlookup" && url.hasQueryItem( "group" ) )
    return url.queryItemValue( "group" ).toUInt();

  return 0;
}

QStringList ArticleView::getArticlesList()
{
  return ui.definition->page()->mainFrame()->
           evaluateJavaScript( "gdArticleContents;" ).toString().
             trimmed().split( ' ', QString::SkipEmptyParts );
}

QString ArticleView::getCurrentArticle()
{
  QVariant v = ui.definition->page()->mainFrame()->evaluateJavaScript(
    QString( "gdCurrentArticle;" ) );

  if ( v.type() == QVariant::String )
    return v.toString();
  else
    return QString();
}

void ArticleView::setCurrentArticle( QString const & id, bool moveToIt )
{
  if ( !id.startsWith( "gdfrom-" ) )
    return; // Incorrect id

  if ( getArticlesList().contains( id.mid( 7 ) ) )
  {
    if ( moveToIt )
    {
      QUrl url( ui.definition->url() );
      url.setFragment( id );
      openLink( url, ui.definition->url() );
    }

    ui.definition->history()->currentItem().setUserData( id );
    ui.definition->page()->mainFrame()->evaluateJavaScript(
      QString( "gdMakeArticleActive( '%1' );" ).arg( id.mid( 7 ) ) );
  }
}

void ArticleView::cleanupTemp()
{
  if ( desktopOpenedTempFile.size() )
  {
    QFile( desktopOpenedTempFile ).remove();
    desktopOpenedTempFile.clear();
  }  
}

bool ArticleView::eventFilter( QObject * obj, QEvent * ev )
{
  if ( obj == ui.definition )
  {
    if ( ev->type() == QEvent::KeyPress )
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( keyEvent->modifiers() &
           ( Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier ) )
        return false; // A non-typing modifier is pressed

      if ( keyEvent->key() == Qt::Key_Space ||
           keyEvent->key() == Qt::Key_Backspace ||
           keyEvent->key() == Qt::Key_Tab )
        return false; // Those key have other uses than to start typing

      QString text = keyEvent->text();

      if ( text.size() )
      {
        emit typingEvent( text );
        return true;
      }
    }
  }
  else
    return QFrame::eventFilter( obj, ev );

  return false;
}


void ArticleView::linkClicked( QUrl const & url )
{
  openLink( url, ui.definition->url(), getCurrentArticle() );
}

void ArticleView::openLink( QUrl const & url, QUrl const & ref,
                            QString const & scrollTo )
{
  printf( "clicked %s\n", url.toString().toLocal8Bit().data() );

  if ( url.scheme() == "bword" )
    showDefinition( ( url.host().startsWith( "xn--" ) ?
                      QUrl::fromPunycode( url.host().toLatin1() ) :
                      url.host() ) + url.path(),
                    getGroup( ref ), scrollTo );
  else
  if ( url.scheme() == "gdlookup" ) // Plain html links inherit gdlookup scheme
  {
    if ( url.hasFragment() )
    {
      ui.definition->page()->mainFrame()->evaluateJavaScript(
        QString( "window.location = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );
    }
    else
    showDefinition( url.path().mid( 1 ),
                    getGroup( ref ), scrollTo );
  }
  else
  if ( url.scheme() == "bres" || url.scheme() == "gdau" )
  {
    // Download it

    // Clear any pending ones

    resourceDownloadRequests.clear();

    resourceDownloadUrl = url;

    if ( url.scheme() == "gdau" && url.host() == "search" )
    {
      // Since searches should be limited to current group, we just do them
      // here ourselves since otherwise we'd need to pass group id to netmgr
      // and it should've been having knowledge of the current groups, too.

      unsigned currentGroup = getGroup( ref );

      std::vector< sptr< Dictionary::Class > > const * activeDicts = 0;

      if ( groups.size() )
      {
        for( unsigned x = 0; x < groups.size(); ++x )
          if ( groups[ x ].id == currentGroup )
          {
            activeDicts = &( groups[ x ].dictionaries );
            break;
          }
      }
      else
        activeDicts = &allDictionaries;

      if ( activeDicts )
        for( unsigned x = 0; x < activeDicts->size(); ++x )
        {
          sptr< Dictionary::DataRequest > req = 
            (*activeDicts)[ x ]->getResource(
              url.path().mid( 1 ).toUtf8().data() );
  
          if ( req->isFinished() && req->dataSize() >= 0 )
          {
            // A request was instantly finished with success.
            // If we've managed to spawn some lingering requests already,
            // erase them.
            resourceDownloadRequests.clear();
  
            // Handle the result
            resourceDownloadRequests.push_back( req );
            resourceDownloadFinished();
  
            return;
          }
          else
          if ( !req->isFinished() )
          {
            resourceDownloadRequests.push_back( req );
  
            connect( req.get(), SIGNAL( finished() ),
                     this, SLOT( resourceDownloadFinished() ) );
          }
        }
    }
    else
    {
      // Normal resource download
      QString contentType;

      sptr< Dictionary::DataRequest > req = 
        articleNetMgr.getResource( url, contentType );

      if ( !req.get() )
      {
        // Request failed, fail
      }
      else
      if ( req->isFinished() && req->dataSize() >= 0 )
      {
        // Have data ready, handle it
        resourceDownloadRequests.push_back( req );
        resourceDownloadFinished();

        return;
      }
      else
      if ( !req->isFinished() )
      {
        // Queue to be handled when done

        resourceDownloadRequests.push_back( req );

        connect( req.get(), SIGNAL( finished() ),
                 this, SLOT( resourceDownloadFinished() ) );
      }
    }

    QString contentType;

    if ( resourceDownloadRequests.empty() ) // No requests were queued
    {
      QMessageBox::critical( this, tr( "GoldenDict" ), tr( "The referenced resource doesn't exist." ) );
      return;
    }
    else
      resourceDownloadFinished(); // Check any requests finished already
  }
  else
  if ( url.scheme() == "http" || url.scheme() == "https" ||
      url.scheme() == "ftp" || url.scheme() == "mailto" )
  {
    // Use the system handler for the conventional internet links
    QDesktopServices::openUrl( url );
  }
}

bool ArticleView::hasSound()
{
  return ui.definition->page()->mainFrame()->
    evaluateJavaScript( "gdAudioLink;" ).type() == QVariant::String;
}

void ArticleView::playSound()
{
  QVariant v = ui.definition->page()->mainFrame()->evaluateJavaScript(
    QString( "gdAudioLink;" ) );

  if ( v.type() == QVariant::String )
    openLink( QUrl::fromEncoded( v.toString().toUtf8() ), ui.definition->url() );
}

QString ArticleView::toHtml()
{
  return ui.definition->page()->mainFrame()->toHtml();
}

QString ArticleView::getTitle()
{
  return ui.definition->page()->mainFrame()->title();
}

void ArticleView::print( QPrinter * printer ) const
{
  ui.definition->print( printer );
}

void ArticleView::contextMenuRequested( QPoint const & pos )
{
  // Is that a link? Is there a selection?

  QWebHitTestResult r = ui.definition->page()->mainFrame()->
                          hitTestContent( pos );

  QMenu menu( this );


  QAction * followLink = 0;
  QAction * followLinkNewTab = 0;
  QAction * lookupSelection = 0;
  QAction * lookupSelectionGr = 0;
  QAction * lookupSelectionNewTab = 0;
  QAction * lookupSelectionNewTabGr = 0;

  if ( !r.linkUrl().isEmpty() )
  {
    followLink = new QAction( tr( "&Open Link" ), &menu );
    menu.addAction( followLink );
    
    if ( !popupView )
    {
      followLinkNewTab = new QAction( tr( "Open Link in New &Tab" ), &menu );
      menu.addAction( followLinkNewTab );
    }

    QString scheme = r.linkUrl().scheme();

    if ( scheme == "http" || scheme == "https" || scheme == "ftp" || scheme == "mailto" )
      menu.addAction( ui.definition->pageAction( QWebPage::CopyLinkToClipboard ) );
  }

  QString selectedText = ui.definition->selectedText();
  
  if ( selectedText.size() )
  {
    lookupSelection = new QAction( tr( "&Look up \"%1\"" ).
                                   arg( ui.definition->selectedText() ),
                                   &menu );
    menu.addAction( lookupSelection );
    
    if ( !popupView )
    {
      lookupSelectionNewTab = new QAction( QIcon( ":/icons/addtab.png" ),
                                           tr( "Look up \"%1\" in &New Tab" ).
                                           arg( ui.definition->selectedText() ),
                                           &menu );
      menu.addAction( lookupSelectionNewTab );
    }

    Instances::Group const * altGroup =
      ( groupComboBox && groupComboBox->getCurrentGroup() !=  getGroup( ui.definition->url() )  ) ?
        groups.findGroup( groupComboBox->getCurrentGroup() ) : 0;

    if ( altGroup )
    {
      QIcon icon = altGroup->icon.size() ? QIcon( ":/flags/" + altGroup->icon ) :
                   QIcon();

      lookupSelectionGr = new QAction( icon, tr( "Look up \"%1\" in %2" ).
                                       arg( ui.definition->selectedText() ).
                                       arg( altGroup->name ), &menu );
      menu.addAction( lookupSelectionGr );

      if ( !popupView )
      {
        lookupSelectionNewTabGr = new QAction( QIcon( ":/icons/addtab.png" ),
                                               tr( "Look up \"%1\" in %2 in &New Tab" ).
                                               arg( ui.definition->selectedText() ).
                                               arg( altGroup->name ), &menu );
        menu.addAction( lookupSelectionNewTabGr );
      }
    }

    menu.addAction( ui.definition->pageAction( QWebPage::Copy ) );
  }

  map< QAction *, QString > tableOfContents;
    
  // Add table of contents
  QStringList ids = getArticlesList();

  if ( !menu.isEmpty() && ids.size() )
    menu.addSeparator();

  unsigned refsAdded = 0;

  for( QStringList::const_iterator i = ids.constBegin(); i != ids.constEnd();
       ++i, ++refsAdded )
  {
    if ( refsAdded == 20 )
    {
      // Enough! Or the menu would become too large.
      menu.addAction( new QAction( ".........", &menu ) );
      break;
    }

    // Find this dictionary

    for( unsigned x = allDictionaries.size(); x--; )
    {
      if ( allDictionaries[ x ]->getId() == i->toUtf8().data() )
      {
        QAction * action =
          new QAction(
                allDictionaries[ x ]->getIcon(),
                QString::fromUtf8( allDictionaries[ x ]->getName().c_str() ),
                &menu );

        menu.addAction( action );

        tableOfContents[ action ] = *i;

        break;
      }
    }
  }
  
  if ( !menu.isEmpty() )
  {
    QAction * result = menu.exec( ui.definition->mapToGlobal( pos ) );

    if ( result == followLink )
      linkClicked( r.linkUrl() );
    else
    if ( result == lookupSelection )
      showDefinition( selectedText, getGroup( ui.definition->url() ), getCurrentArticle() );
    else
    if ( result == lookupSelectionGr && groupComboBox )
      showDefinition( selectedText, groupComboBox->getCurrentGroup(), QString() );
    else
    if ( !popupView && result == followLinkNewTab )
      emit openLinkInNewTab( r.linkUrl(), ui.definition->url(), getCurrentArticle() );
    else
    if ( !popupView && result == lookupSelectionNewTab )
      emit showDefinitionInNewTab( selectedText, getGroup( ui.definition->url() ),
                                   getCurrentArticle() );
    else
    if ( !popupView && result == lookupSelectionNewTabGr && groupComboBox )
      emit showDefinitionInNewTab( selectedText, groupComboBox->getCurrentGroup(),
                                   QString() );
    else
    {
      // Match against table of contents
      QString id = tableOfContents[ result ];

      if ( id.size() )
        setCurrentArticle( "gdfrom-" + id, true );
    }
  }
#if 0
  printf( "%s\n", r.linkUrl().isEmpty() ? "null" : "not null" );

  printf( "url = %s\n", r.linkUrl().toString().toLocal8Bit().data() );
  printf( "title = %s\n", r.title().toLocal8Bit().data() );
#endif
}

void ArticleView::resourceDownloadFinished()
{
  if ( resourceDownloadRequests.empty() )
    return; // Stray signal

  // Find any finished resources
  for( list< sptr< Dictionary::DataRequest > >::iterator i =
       resourceDownloadRequests.begin(); i != resourceDownloadRequests.end(); )
  {
    if ( (*i)->isFinished() )
    {
      if ( (*i)->dataSize() >= 0 )
      {
        // Ok, got one finished, all others are irrelevant now

        vector< char > const & data = (*i)->getFullData();

        if ( resourceDownloadUrl.scheme() == "gdau" )
        {
          #ifdef Q_OS_WIN32
          // Windows-only: use system PlaySound function

          if ( winWavData.size() )
            PlaySoundA( 0, 0, 0 ); // Stop any currently playing sound to make sure
                                   // previous data isn't used anymore
                                   // 
          winWavData = data;

          PlaySoundA( &winWavData.front(), 0,
                      SND_ASYNC | SND_MEMORY | SND_NODEFAULT | SND_NOWAIT );
          #else

          // Use external viewer to play the file
          try
          {
            ExternalViewer * viewer = new ExternalViewer( this, data, "wav", cfg.preferences.audioPlaybackProgram );

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
            QMessageBox::critical( this, tr( "GoldenDict" ), tr( "Failed to run a player to play sound file: %1" ).arg( e.what() ) );
          }

          #endif
        }
        else
        {
          // Create a temporary file
  
  
          // Remove the one previously used, if any
          cleanupTemp();
  
          {
            QTemporaryFile tmp(
              QDir::temp().filePath( "XXXXXX-" + resourceDownloadUrl.path().section( '/', -1 ) ), this );

            if ( !tmp.open() || tmp.write( &data.front(), data.size() ) != data.size() )
            {
              QMessageBox::critical( this, tr( "GoldenDict" ), tr( "Failed to create temporary file." ) );
              return;
            }
  
            tmp.setAutoRemove( false );
  
            desktopOpenedTempFile = tmp.fileName();
          }
  
          if ( !QDesktopServices::openUrl( QUrl::fromLocalFile( desktopOpenedTempFile ) ) )
            QMessageBox::critical( this, tr( "GoldenDict" ), tr( "Failed to auto-open resource file, try opening manually: %1." ).arg( desktopOpenedTempFile ) );
        }

        // Ok, whatever it was, it's finished. Remove this and any other
        // requests and finish.

        resourceDownloadRequests.clear();

        return;
      }
      else
      {
        // This one had no data. Erase  it.
        resourceDownloadRequests.erase( i++ );
      }
    }
    else // Unfinished, try the next one.
      i++;
  }

  if ( resourceDownloadRequests.empty() )
  {
    // No requests suceeded.
    QMessageBox::critical( this, tr( "GoldenDict" ), tr( "The referenced resource failed to download." ) );
  }
}

void ArticleView::pasteTriggered()
{
  QString text =
      gd::toQString(
          Folding::trimWhitespaceOrPunct(
              gd::toWString(
                  QApplication::clipboard()->text() ) ) );

  if ( text.size() )
    showDefinition( text, getGroup( ui.definition->url() ), getCurrentArticle() );
}

void ArticleView::moveOneArticleUp()
{
  QString current = getCurrentArticle();

  if ( current.size() )
  {
    QStringList lst = getArticlesList();

    int idx = lst.indexOf( current.mid( 7 ) );

    if ( idx != -1 )
    {
      --idx;

      if ( idx < 0 )
        idx = lst.size() - 1;

      setCurrentArticle( "gdfrom-" + lst[ idx ], true );
    }
  }
}

void ArticleView::moveOneArticleDown()
{
  QString current = getCurrentArticle();

  if ( current.size() )
  {
    QStringList lst = getArticlesList();

    int idx = lst.indexOf( current.mid( 7 ) );

    if ( idx != -1 )
    {
      idx = ( idx + 1 ) % lst.size();

      setCurrentArticle( "gdfrom-" + lst[ idx ], true );
    }
  }
}

void ArticleView::openSearch()
{
  if ( !searchIsOpened )
  {
    ui.searchFrame->show();
    ui.searchText->setText( getTitle() );
    searchIsOpened = true;
  }

  ui.searchText->setFocus();
  ui.searchText->selectAll();

  // Clear any current selection
  if ( ui.definition->selectedText().size() )
  {
    ui.definition->page()->currentFrame()->
           evaluateJavaScript( "window.getSelection().removeAllRanges();" );
  }

  if ( ui.searchText->property( "noResults" ).toBool() )
  {
    ui.searchText->setProperty( "noResults", false );

    // Reload stylesheet
    for( QWidget * w = parentWidget(); w; w = w->parentWidget() )
    {
      if ( w->styleSheet().size() )
      {
        w->setStyleSheet( w->styleSheet() );
        break;
      }
    }
  }
}

void ArticleView::on_searchPrevious_clicked()
{
  performFindOperation( false, true );
}

void ArticleView::on_searchNext_clicked()
{
  performFindOperation( false, false );
}

void ArticleView::on_searchText_textEdited()
{
  performFindOperation( true, false );
}

void ArticleView::on_searchText_returnPressed()
{
  on_searchNext_clicked();
}

void ArticleView::on_searchCloseButton_clicked()
{
  closeSearch();
}

void ArticleView::performFindOperation( bool restart, bool backwards )
{
  QString text = ui.searchText->text();

  if ( restart )
  {
    // Anyone knows how we reset the search position?
    // For now we resort to this hack:
    if ( ui.definition->selectedText().size() )
    {
      ui.definition->page()->currentFrame()->
             evaluateJavaScript( "window.getSelection().removeAllRanges();" );
    }
  }

  QWebPage::FindFlags f( 0 );

  if ( ui.searchCaseSensitive->isChecked() )
    f |= QWebPage::FindCaseSensitively;

  if ( backwards )
    f |= QWebPage::FindBackward;

  bool setMark = text.size() && !ui.definition->findText( text, f );

  if ( ui.searchText->property( "noResults" ).toBool() != setMark )
  {
    ui.searchText->setProperty( "noResults", setMark );

    // Reload stylesheet
    for( QWidget * w = parentWidget(); w; w = w->parentWidget() )
    {
      if ( w->styleSheet().size() )
      {
        w->setStyleSheet( w->styleSheet() );
        break;
      }
    }
  }
}

bool ArticleView::closeSearch()
{
  if ( searchIsOpened )
  {
    ui.searchFrame->hide();
    ui.definition->setFocus();
    searchIsOpened = false;

    return true;
  }
  else
    return false;
}

void ArticleView::showEvent( QShowEvent * ev )
{
  QFrame::showEvent( ev );

  if ( !searchIsOpened )
    ui.searchFrame->hide();
}
