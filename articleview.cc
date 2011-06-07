/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
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
#include "webmultimediadownload.hh"
#include "programs.hh"

#ifdef Q_OS_WIN32
#include <windows.h>
#include <mmsystem.h> // For PlaySound
#endif
#include <QBuffer>

// Phonon headers are a mess. How to include them properly? Send patches if you
// know.
#ifdef __WIN32
#include <Phonon/AudioOutput>
#include <Phonon/MediaObject>
#else
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#endif

using std::map;
using std::list;

/// A phonon-based audio player, created on demand
struct AudioPlayer
{
  Phonon::AudioOutput output;
  Phonon::MediaObject object;

  static AudioPlayer & instance();

private:

  AudioPlayer();
};

AudioPlayer::AudioPlayer():
    output( Phonon::AccessibilityCategory )
{
  Phonon::createPath( &object, &output );
}

AudioPlayer & AudioPlayer::instance()
{
  static AudioPlayer a;

  return a;
}


ArticleView::ArticleView( QWidget * parent, ArticleNetworkAccessManager & nm,
                          std::vector< sptr< Dictionary::Class > > const & allDictionaries_,
                          Instances::Groups const & groups_, bool popupView_,
                          Config::Class const & cfg_,
                          QAction * dictionaryBarToggled_,
                          Config::MutedDictionaries * mutedDictionaries_,
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
  goBackAction( this ),
  goForwardAction( this ),
  openSearchAction( this ),
  searchIsOpened( false ),
  dictionaryBarToggled( dictionaryBarToggled_ ),
  mutedDictionaries( mutedDictionaries_ ),
  groupComboBox( groupComboBox_ )
{
  ui.setupUi( this );

  goBackAction.setShortcut( QKeySequence( "Alt+Left" ) );
  ui.definition->addAction( &goBackAction );
  connect( &goBackAction, SIGNAL( triggered() ),
           this, SLOT( back() ) );

  goForwardAction.setShortcut( QKeySequence( "Alt+Right" ) );
  ui.definition->addAction( &goForwardAction );
  connect( &goForwardAction, SIGNAL( triggered() ),
           this, SLOT( forward() ) );

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

  connect( ui.definition, SIGNAL(doubleClicked()),this,SLOT(doubleClicked()) );

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
                                  QString const & scrollTo,
                                  Contexts const & contexts )
{
  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  req.addQueryItem( "word", word );
  req.addQueryItem( "group", QString::number( group ) );

  if ( scrollTo.size() )
    req.addQueryItem( "scrollto", scrollTo );

  if ( contexts.size() )
  {
    QBuffer buf;

    buf.open( QIODevice::WriteOnly );

    QDataStream stream( &buf );

    stream << contexts;

    buf.close();

    req.addQueryItem( "contexts", QString::fromAscii( buf.buffer().toBase64() ) );
  }

  QString mutedDicts = getMutedForGroup( group );

  if ( mutedDicts.size() )
    req.addQueryItem( "muted", mutedDicts );

  // Update history
  saveHistoryUserData();

  // Any search opened is probably irrelevant now
  closeSearch();

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

      (*i)->evaluateJavaScript( "var gdLastUrlText;" );
      (*i)->evaluateJavaScript( "document.addEventListener( 'click', function() { gdLastUrlText = window.event.srcElement.textContent; }, true );" );
      (*i)->evaluateJavaScript( "document.addEventListener( 'contextmenu', function() { gdLastUrlText = window.event.srcElement.textContent; }, true );" );

      wereFrames = true;
    }
  }

  if ( wereFrames )
  {
    // There's some sort of glitch -- sometimes you need to move a mouse

    QMouseEvent ev( QEvent::MouseMove, QPoint(), Qt::MouseButton(), 0, 0 );

    qApp->sendEvent( ui.definition, &ev );
  }

  QVariant userDataVariant = ui.definition->history()->currentItem().userData();

  if ( userDataVariant.type() == QVariant::Map )
  {
    QMap< QString, QVariant > userData = userDataVariant.toMap();

    QString currentArticle = userData.value( "currentArticle" ).toString();

    if ( currentArticle.size() )
    {
      // There's an active article saved, so set it to be active.
      setCurrentArticle( currentArticle );
    }

    double sx = 0, sy = 0;

    if ( userData.value( "sx" ).type() == QVariant::Double )
      sx = userData.value( "sx" ).toDouble();

    if ( userData.value( "sy" ).type() == QVariant::Double )
      sy = userData.value( "sy" ).toDouble();

    if ( sx != 0 || sy != 0 )
    {
      // Restore scroll position
      ui.definition->page()->mainFrame()->evaluateJavaScript(
          QString( "window.scroll( %1, %2 );" ).arg( sx ).arg( sy ) );
    }
  }
  else
  if ( url.queryItemValue( "scrollto" ).startsWith( "gdfrom-" ) )
  {
    // There is no active article saved in history, but we have it as a parameter.
    // setCurrentArticle will save it and scroll there.
    setCurrentArticle( url.queryItemValue( "scrollto" ), true );
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

        icon = groups[ x ].makeIcon();
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

void ArticleView::jumpToDictionary(QString const & id)
{
  setCurrentArticle( "gdfrom-" + id, true );
}

void ArticleView::setCurrentArticle( QString const & id, bool moveToIt )
{
  if ( !id.startsWith( "gdfrom-" ) )
    return; // Incorrect id

  if ( getArticlesList().contains( id.mid( 7 ) ) )
  {
    if ( moveToIt )
      ui.definition->page()->mainFrame()->evaluateJavaScript( QString( "document.getElementById('%1').scrollIntoView(true);" ).arg( id ) );

    QMap< QString, QVariant > userData = ui.definition->history()->
                                         currentItem().userData().toMap();
    userData[ "currentArticle" ] = id;
    ui.definition->history()->currentItem().setUserData( userData );

    ui.definition->page()->mainFrame()->evaluateJavaScript(
      QString( "gdMakeArticleActive( '%1' );" ).arg( id.mid( 7 ) ) );
  }
}

bool ArticleView::isFramedArticle( QString const & ca )
{
  if ( ca.isEmpty() )
    return false;

  return ui.definition->page()->mainFrame()->
               evaluateJavaScript( QString( "!!document.getElementById('gdexpandframe-%1');" ).arg( ca.mid( 7 ) ) ).toBool();
}

bool ArticleView::isExternalLink( QUrl const & url )
{
  return url.scheme() == "http" || url.scheme() == "https" ||
         url.scheme() == "ftp" || url.scheme() == "mailto" ||
         url.scheme() == "file";
}

void ArticleView::tryMangleWebsiteClickedUrl( QUrl & url, Contexts & contexts )
{
  // Don't try mangling audio urls, even if they are from the framed websites

  if( ( url.scheme() == "http" || url.scheme() == "https" )
      && ! Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
  {
    // Maybe a link inside a website was clicked?

    QString ca = getCurrentArticle();

    if ( isFramedArticle( ca ) )
    {
      QVariant result = ui.definition->page()->currentFrame()->evaluateJavaScript( "gdLastUrlText;" );

      if ( result.type() == QVariant::String )
      {
        // Looks this way

        contexts[ ca.mid( 7 ) ] = QString::fromAscii( url.toEncoded() );

        QUrl target;

        QString queryWord = result.toString();

        // Empty requests are treated as no request, so we work this around by
        // adding a space.
        if ( queryWord.isEmpty() )
          queryWord = " ";

        target.setScheme( "gdlookup" );
        target.setHost( "localhost" );
        target.setPath( "/" + queryWord );

        url = target;
      }
    }
  }
}

void ArticleView::updateCurrentArticleFromCurrentFrame( QWebFrame * frame )
{
  if ( !frame )
    frame = ui.definition->page()->currentFrame();

  for( ; frame; frame = frame->parentFrame() )
  {
    QString frameName = frame->frameName();

    if ( frameName.startsWith( "gdexpandframe-" ) )
    {
      QString newCurrent = "gdfrom-" + frameName.mid( 14 );

      if ( getCurrentArticle() != newCurrent )
        setCurrentArticle( newCurrent, false );

      break;
    }
  }
}

void ArticleView::saveHistoryUserData()
{
  QMap< QString, QVariant > userData = ui.definition->history()->
                                       currentItem().userData().toMap();

  // Save current article, which can be empty

  userData[ "currentArticle" ] = getCurrentArticle();

  // We also save window position. We restore it when the page is fully loaded,
  // when any hidden frames are expanded.

  userData[ "sx" ] = ui.definition->page()->mainFrame()->evaluateJavaScript( "window.scrollX;" ).toDouble();
  userData[ "sy" ] = ui.definition->page()->mainFrame()->evaluateJavaScript( "window.scrollY;" ).toDouble();

  ui.definition->history()->currentItem().setUserData( userData );
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
    if ( ev->type() == QEvent::MouseButtonPress ) {
      QMouseEvent * event = static_cast< QMouseEvent * >( ev );
      if ( event->button() == Qt::XButton1 ) {
        back();
        return true;
      }
      if ( event->button() == Qt::XButton2 ) {
        forward();
        return true;
      }
    }
    else
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

QString ArticleView::getMutedForGroup( unsigned group )
{
  if ( dictionaryBarToggled && mutedDictionaries && dictionaryBarToggled->isChecked() )
  {
    // Dictionary bar is active -- mute the muted dictionaries
    Instances::Group const * groupInstance = groups.findGroup( group );

    QStringList mutedDicts;

    if ( groupInstance )
    {
      for( unsigned x = 0; x < groupInstance->dictionaries.size(); ++x )
      {
        QString id = QString::fromStdString(
                       groupInstance->dictionaries[ x ]->getId() );

        if ( mutedDictionaries->contains( id ) )
          mutedDicts.append( id );
      }
    }

    if ( mutedDicts.size() )
      return mutedDicts.join( "," );
  }

  return QString();
}

void ArticleView::linkClicked( QUrl const & url_ )
{
  updateCurrentArticleFromCurrentFrame();

  QUrl url( url_ );
  Contexts contexts;

  tryMangleWebsiteClickedUrl( url, contexts );

  if ( !popupView && ui.definition->isMidButtonPressed() )
  {
    // Mid button is currently pressed - open the link in new tab
    emit openLinkInNewTab( url, ui.definition->url(), getCurrentArticle(), contexts );
  }
  else
    openLink( url, ui.definition->url(), getCurrentArticle(), contexts );
}

void ArticleView::openLink( QUrl const & url, QUrl const & ref,
                            QString const & scrollTo,
                            Contexts const & contexts )
{
  printf( "clicked %s\n", url.toString().toLocal8Bit().data() );

  if ( url.scheme().compare( "bword" ) == 0 )
  {
    showDefinition( url.path(),
                    getGroup( ref ), scrollTo, contexts );
  }
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
                    getGroup( ref ), scrollTo, contexts );
  }
  else
  if ( url.scheme() == "bres" || url.scheme() == "gdau" ||
       Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
  {
    // Download it

    // Clear any pending ones

    resourceDownloadRequests.clear();

    resourceDownloadUrl = url;

    if ( Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
    {
      sptr< Dictionary::DataRequest > req =
        new Dictionary::WebMultimediaDownload( url, articleNetMgr );

      resourceDownloadRequests.push_back( req );

      connect( req.get(), SIGNAL( finished() ),
               this, SLOT( resourceDownloadFinished() ) );
    }
    else
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
  if ( url.scheme() == "gdprg" )
  {
    // Program. Run it.
    QString id( url.host() );

    for( Config::Programs::const_iterator i = cfg.programs.begin();
         i != cfg.programs.end(); ++i )
    {
      if ( i->id == id )
      {
        // Found the corresponding program.
        Programs::RunInstance * req = new Programs::RunInstance;

        connect( req, SIGNAL(finished(QByteArray,QString)),
                 req, SLOT( deleteLater() ) );

        QString error;

        // Delete the request if it fails to start
        if ( !req->start( *i, url.path().mid( 1 ), error ) )
        {
          delete req;

          QMessageBox::critical( this, tr( "GoldenDict" ),
                                 error );
        }

        return;
      }
    }

    // Still here? No such program exists.
    QMessageBox::critical( this, tr( "GoldenDict" ),
                           tr( "The referenced audio program doesn't exist." ) );
  }
  else
  if ( isExternalLink( url ) )
  {
    // Use the system handler for the conventional external links
    QDesktopServices::openUrl( url );
  }
}

void ArticleView::updateMutedContents()
{
  QUrl currentUrl = ui.definition->url();

  if ( currentUrl.scheme() != "gdlookup" )
    return; // Weird url -- do nothing

  unsigned group = getGroup( currentUrl );

  if ( !group )
    return; // No group in url -- do nothing

  QString mutedDicts = getMutedForGroup( group );

  if ( currentUrl.queryItemValue( "muted" ) != mutedDicts )
  {
    // The list has changed -- update the url

    currentUrl.removeQueryItem( "muted" );

    if ( mutedDicts.size() )
    currentUrl.addQueryItem( "muted", mutedDicts );

    saveHistoryUserData();

    ui.definition->load( currentUrl );

    //QApplication::setOverrideCursor( Qt::WaitCursor );
    ui.definition->setCursor( Qt::WaitCursor );
  }
}

void ArticleView::back()
{
  // Don't allow navigating back to page 0, which is usually the initial
  // empty page
  if ( ui.definition->history()->currentItemIndex() > 1 )
  {
    saveHistoryUserData();
    ui.definition->back();
  }
}

void ArticleView::forward()
{
  saveHistoryUserData();
  ui.definition->forward();
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

  updateCurrentArticleFromCurrentFrame( r.frame() );

  QMenu menu( this );


  QAction * followLink = 0;
  QAction * followLinkExternal = 0;
  QAction * followLinkNewTab = 0;
  QAction * lookupSelection = 0;
  QAction * lookupSelectionGr = 0;
  QAction * lookupSelectionNewTab = 0;
  QAction * lookupSelectionNewTabGr = 0;

  QUrl targetUrl( r.linkUrl() );
  Contexts contexts;

  tryMangleWebsiteClickedUrl( targetUrl, contexts );

  if ( !r.linkUrl().isEmpty() )
  {
    if ( !isExternalLink( targetUrl ) )
    {
      followLink = new QAction( tr( "&Open Link" ), &menu );
      menu.addAction( followLink );

      if ( !popupView )
      {
        followLinkNewTab = new QAction( QIcon( ":/icons/addtab.png" ),
                                        tr( "Open Link in New &Tab" ), &menu );
        menu.addAction( followLinkNewTab );
      }
    }

    if ( isExternalLink( r.linkUrl() ) )
    {
      followLinkExternal = new QAction( tr( "Open Link in &External Browser" ), &menu );
      menu.addAction( followLinkExternal );
      menu.addAction( ui.definition->pageAction( QWebPage::CopyLinkToClipboard ) );
    }
  }

  QString selectedText = ui.definition->selectedText();

  if ( selectedText.size() && selectedText.size() < 60 )
  {
    // We don't prompt for selections larger or equal to 60 chars, since
    // it ruins the menu and it's hardly a single word anyway.

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
  }

  if ( selectedText.size() )
    menu.addAction( ui.definition->pageAction( QWebPage::Copy ) );

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

    if ( !result )
      return;

    if ( result == followLink )
      openLink( targetUrl, ui.definition->url(), getCurrentArticle(), contexts );
    else
    if ( result == followLinkExternal )
      QDesktopServices::openUrl( r.linkUrl() );
    else
    if ( result == lookupSelection )
      showDefinition( selectedText, getGroup( ui.definition->url() ), getCurrentArticle() );
    else
    if ( result == lookupSelectionGr && groupComboBox )
      showDefinition( selectedText, groupComboBox->getCurrentGroup(), QString() );
    else
    if ( !popupView && result == followLinkNewTab )
      emit openLinkInNewTab( targetUrl, ui.definition->url(), getCurrentArticle(), contexts );
    else
    if ( !popupView && result == lookupSelectionNewTab )
      emit showDefinitionInNewTab( selectedText, getGroup( ui.definition->url() ),
                                   getCurrentArticle(), Contexts() );
    else
    if ( !popupView && result == lookupSelectionNewTabGr && groupComboBox )
      emit showDefinitionInNewTab( selectedText, groupComboBox->getCurrentGroup(),
                                   QString(), Contexts() );
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

        if ( resourceDownloadUrl.scheme() == "gdau" ||
             Dictionary::WebMultimediaDownload::isAudioUrl( resourceDownloadUrl ) )
        {
          // Audio data

#ifdef Q_OS_WIN32
          // If we use Windows PlaySound, use that, not Phonon.
          if ( !cfg.preferences.useExternalPlayer &&
               cfg.preferences.useWindowsPlaySound )
          {
            // Stop any currently playing sound to make sure the previous data
            // isn't used anymore
            if ( winWavData.size() )
            {
              PlaySoundA( 0, 0, 0 );
              winWavData.clear();
            }

            if ( data.size() < 4 || memcmp( data.data(), "RIFF", 4 ) != 0 )
            {
              QMessageBox::information( this, tr( "Playing a non-WAV file" ),
                tr( "To enable playback of files different than WAV, please go "
                    "to Edit|Preferences, choose the Audio tab and select "
                    "\"Play via DirectShow\" there." ) );
            }
            else
            {
              winWavData = data;
              PlaySoundA( &winWavData.front(), 0,
                          SND_ASYNC | SND_MEMORY | SND_NODEFAULT | SND_NOWAIT );
            }
          }
          else
#endif
          if ( !cfg.preferences.useExternalPlayer )
          {
            // Play via Phonon

            QBuffer * buf = new QBuffer;

            buf->buffer().append( &data.front(), data.size() );

            Phonon::MediaSource source( buf );
            source.setAutoDelete( true ); // Dispose of our buf when done

            AudioPlayer::instance().object.stop();
            AudioPlayer::instance().object.clear();
            AudioPlayer::instance().object.enqueue( source );
            AudioPlayer::instance().object.play();
          }
          else
          {

            // Use external viewer to play the file
            try
            {
              ExternalViewer * viewer = new ExternalViewer( this, data, "wav", cfg.preferences.audioPlaybackProgram.trimmed() );

              // Once started, it will erase itself
              viewer->start();
            }
            catch( ExternalViewer::Ex & e )
            {
              QMessageBox::critical( this, tr( "GoldenDict" ), tr( "Failed to run a player to play sound file: %1" ).arg( e.what() ) );
            }
          }
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
        // This one had no data. Erase it.
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
    reloadStyleSheet();
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

void ArticleView::doubleClicked()
{
  // We might want to initiate translation of the selected word

  if ( cfg.preferences.doubleClickTranslates )
  {
    QString selectedText = ui.definition->selectedText();

    // Do some checks to make sure there's a sensible selection indeed
    if ( Folding::applyWhitespaceOnly( gd::toWString( selectedText ) ).size() &&
         selectedText.size() < 40 )
    {
      // Initiate translation
      Qt::KeyboardModifiers kmod = QApplication::keyboardModifiers();
      if (kmod & (Qt::ControlModifier | Qt::ShiftModifier))
      { // open in new tab
        emit showDefinitionInNewTab( selectedText, getGroup( ui.definition->url() ),
                                     getCurrentArticle(), Contexts() );
      }
      else
        showDefinition( selectedText, getGroup( ui.definition->url() ), getCurrentArticle() );
    }
  }
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
    reloadStyleSheet();
  }
}

void ArticleView::reloadStyleSheet()
{
  for( QWidget * w = parentWidget(); w; w = w->parentWidget() )
  {
    if ( w->styleSheet().size() )
    {
      w->setStyleSheet( w->styleSheet() );
      break;
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
