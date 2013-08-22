/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
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
#include <QFileDialog>
#include "folding.hh"
#include "wstring_qt.hh"
#include "webmultimediadownload.hh"
#include "programs.hh"
#include "dprintf.hh"
#include "ffmpegaudio.hh"
#include <QDebug>
#include <QCryptographicHash>

#if QT_VERSION >= 0x040600
#include <QWebElement>
#endif

#include <assert.h>

#ifdef Q_OS_WIN32
#include <windows.h>

#include <QPainter>
#endif

#include <QBuffer>

#if defined( Q_OS_WIN32 ) || defined( Q_OS_MACX )
#include "speechclient.hh"
#endif

using std::map;
using std::list;


static QVariant evaluateJavaScriptVariableSafe( QWebFrame * frame, const QString & variable )
{
  return frame->evaluateJavaScript(
        QString( "( typeof( %1 ) !== 'undefined' && %1 !== undefined ) ? %1 : null;" )
        .arg( variable ) );
}

ArticleView::ArticleView( QWidget * parent, ArticleNetworkAccessManager & nm,
                          std::vector< sptr< Dictionary::Class > > const & allDictionaries_,
                          Instances::Groups const & groups_, bool popupView_,
                          Config::Class const & cfg_,
                          QAction * dictionaryBarToggled_,
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
  selectCurrentArticleAction( this ),
  copyAsTextAction( this ),
  inspectAction( this ),
  searchIsOpened( false ),
  dictionaryBarToggled( dictionaryBarToggled_ ),
  groupComboBox( groupComboBox_ )
{
  ui.setupUi( this );

  ui.definition->setUp( const_cast< Config::Class * >( &cfg ) );

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

  QAction * selectAll = ui.definition->pageAction( QWebPage::SelectAll );
  selectAll->setShortcut( QKeySequence::SelectAll );
  selectAll->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  ui.definition->addAction( selectAll );

  ui.definition->setContextMenuPolicy( Qt::CustomContextMenu );

  ui.definition->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

  ui.definition->page()->setNetworkAccessManager( &articleNetMgr );

  connect( ui.definition, SIGNAL( loadFinished( bool ) ),
           this, SLOT( loadFinished( bool ) ) );

  attachToJavaScript();
  connect( ui.definition->page()->mainFrame(), SIGNAL( javaScriptWindowObjectCleared() ),
           this, SLOT( attachToJavaScript() ) );

  connect( ui.definition, SIGNAL( titleChanged( QString const & ) ),
           this, SLOT( handleTitleChanged( QString const & ) ) );

  connect( ui.definition, SIGNAL( urlChanged( QUrl const & ) ),
           this, SLOT( handleUrlChanged( QUrl const & ) ) );

  connect( ui.definition, SIGNAL( customContextMenuRequested( QPoint const & ) ),
           this, SLOT( contextMenuRequested( QPoint const & ) ) );

  connect( ui.definition, SIGNAL( linkClicked( QUrl const & ) ),
           this, SLOT( linkClicked( QUrl const & ) ) );

  connect( ui.definition->page(), SIGNAL( linkHovered ( const QString &, const QString &, const QString & ) ),
           this, SLOT( linkHovered ( const QString &, const QString &, const QString & ) ) );

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

  selectCurrentArticleAction.setShortcut( QKeySequence( "Ctrl+Shift+A" ));
  selectCurrentArticleAction.setText( tr( "Select Current Article" ) );
  ui.definition->addAction( &selectCurrentArticleAction );
  connect( &selectCurrentArticleAction, SIGNAL( triggered() ),
           this, SLOT( selectCurrentArticle() ) );

  copyAsTextAction.setShortcut( QKeySequence( "Ctrl+Shift+C" ) );
  copyAsTextAction.setText( tr( "Copy as text" ) );
  ui.definition->addAction( &copyAsTextAction );
  connect( &copyAsTextAction, SIGNAL( triggered() ),
           this, SLOT( copyAsText() ) );

  inspectAction.setShortcut( QKeySequence( Qt::Key_F12 ) );
  inspectAction.setText( tr( "Inspect" ) );
  ui.definition->addAction( &inspectAction );
  connect( &inspectAction, SIGNAL( triggered() ), this, SLOT( inspect() ) );

  ui.definition->installEventFilter( this );
  ui.searchFrame->installEventFilter( this );

  // Load the default blank page instantly, so there would be no flicker.

  QString contentType;
  QUrl blankPage( "gdlookup://localhost?blank=1" );

  sptr< Dictionary::DataRequest > r = articleNetMgr.getResource( blankPage,
                                                                 contentType );

  ui.definition->setHtml( QString::fromUtf8( &( r->getFullData().front() ),
                                             r->getFullData().size() ),
                          blankPage );

  expandOptionalParts = cfg.preferences.alwaysExpandOptionalParts;
}

// explicitly report the minimum size, to avoid
// sidebar widgets' improper resize during restore
QSize ArticleView::minimumSizeHint() const
{
  return ui.searchFrame->minimumSizeHint();
}

void ArticleView::setGroupComboBox( GroupComboBox const * g )
{
  groupComboBox = g;
}

ArticleView::~ArticleView()
{
  cleanupTemp();

#ifndef DISABLE_INTERNAL_PLAYER
  if ( cfg.preferences.useInternalPlayer )
    Ffmpeg::AudioPlayer::instance().stop();
#endif
}

void ArticleView::showDefinition( QString const & word, unsigned group,
                                  QString const & scrollTo,
                                  Contexts const & contexts )
{

#ifndef DISABLE_INTERNAL_PLAYER
  // first, let's stop the player
  if ( cfg.preferences.useInternalPlayer )
    Ffmpeg::AudioPlayer::instance().stop();
#endif

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

    req.addQueryItem( "contexts", QString::fromLatin1( buf.buffer().toBase64() ) );
  }

  QString mutedDicts = getMutedForGroup( group );

  if ( mutedDicts.size() )
    req.addQueryItem( "muted", mutedDicts );

  // Update both histories (pages history and headwords history)
  saveHistoryUserData();
  emit sendWordToHistory( word );

  // Any search opened is probably irrelevant now
  closeSearch();

  // Clear highlight all button selection
  ui.highlightAllButton->setChecked(false);

  emit setExpandMode( expandOptionalParts );

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
      //DPRINTF( "Name: %s\n", (*i)->frameName().toUtf8().data() );
      //DPRINTF( "Size: %d\n", (*i)->contentsSize().height() );
      //DPRINTF( ">>>>>>>>Height = %s\n", (*i)->evaluateJavaScript( "document.body.offsetHeight;" ).toString().toUtf8().data() );

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

  // Expand collapsed article if only one loaded
  ui.definition->page()->mainFrame()->evaluateJavaScript( QString( "gdCheckArticlesNumber();" ) );

  // Jump to current article after page reloading
  if( !articleToJump.isEmpty() )
  {
    setCurrentArticle( articleToJump, true );
    articleToJump.clear();
  }

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
  return evaluateJavaScriptVariableSafe( ui.definition->page()->mainFrame(), "gdArticleContents" )
      .toString().trimmed().split( ' ', QString::SkipEmptyParts );
}

QString ArticleView::getActiveArticleId()
{
  QString currentArticle = getCurrentArticle();
  if ( !currentArticle.startsWith( "gdfrom-" ) )
    return QString(); // Incorrect id

  return currentArticle.mid( 7 );
}

QString ArticleView::getCurrentArticle()
{
  QVariant v = evaluateJavaScriptVariableSafe( ui.definition->page()->mainFrame(), "gdCurrentArticle" );

  if ( v.type() == QVariant::String )
    return v.toString();
  else
    return QString();
}

void ArticleView::jumpToDictionary( QString const & id, bool force )
{
  QString targetArticle = "gdfrom-" + id;

  // jump only if neceessary, or when forced
  if ( force || targetArticle != getCurrentArticle() )
  {
    setCurrentArticle( targetArticle, true );
  }
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

void ArticleView::selectCurrentArticle()
{
  ui.definition->page()->mainFrame()->evaluateJavaScript(
        QString( "gdSelectArticle( '%1' );" ).arg( getActiveArticleId() ) );
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
      QVariant result = evaluateJavaScriptVariableSafe( ui.definition->page()->currentFrame(), "gdLastUrlText" );

      if ( result.type() == QVariant::String )
      {
        // Looks this way

        contexts[ ca.mid( 7 ) ] = QString::fromLatin1( url.toEncoded() );

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

bool ArticleView::handleF3( QObject * /*obj*/, QEvent * ev )
{
  if ( ev->type() == QEvent::ShortcutOverride ) {
    QKeyEvent * ke = static_cast<QKeyEvent *>( ev );
    if ( ke->key() == Qt::Key_F3 && isSearchOpened() ) {
      if ( !ke->modifiers() )
      {
        on_searchNext_clicked();
        ev->accept();
        return true;
      }

      if ( ke->modifiers() == Qt::ShiftModifier )
      {
        on_searchPrevious_clicked();
        ev->accept();
        return true;
      }
    }
  }

  return false;
}

bool ArticleView::eventFilter( QObject * obj, QEvent * ev )
{

  if ( handleF3( obj, ev ) )
  {
    return true;
  }

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
           keyEvent->key() == Qt::Key_Tab ||
           keyEvent->key() == Qt::Key_Backtab )
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
  if ( dictionaryBarToggled && dictionaryBarToggled->isChecked() )
  {
    // Dictionary bar is active -- mute the muted dictionaries
    Instances::Group const * groupInstance = groups.findGroup( group );

    // Find muted dictionaries for current group
    Config::Group const * grp = cfg.getGroup( group );
    Config::MutedDictionaries const * mutedDictionaries;
    if( group == Instances::Group::AllGroupId )
      mutedDictionaries = popupView ? &cfg.popupMutedDictionaries : &cfg.mutedDictionaries;
    else
        mutedDictionaries = grp ? ( popupView ? &grp->popupMutedDictionaries : &grp->mutedDictionaries ) : 0;
    if( !mutedDictionaries )
      return QString();

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

void ArticleView::linkHovered ( const QString & link, const QString & , const QString & )
{
  QString msg;
  QUrl url(link);

  if ( url.scheme() == "bres" )
  {
    msg = tr( "Resource" );
  }
  else
  if ( url.scheme() == "gdau" || Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
  {
    msg = tr( "Audio" );
  }
  else
  if ( url.scheme() == "gdtts" )
  {
    msg = tr( "TTS Voice" );
  }
  else
  if ( url.scheme() == "gdpicture" )
  {
    msg = tr( "Picture" );
  }
  else
  if ( url.scheme() == "gdvideo" )
  {
    if ( url.path().isEmpty() )
    {
      msg = tr( "Video" );
    }
    else
    {
      QString path = url.path();
      if ( path.startsWith( '/' ) )
      {
        path = path.mid( 1 );
      }
      msg = tr( "Video: %1" ).arg( path );
    }
  }
  else
  if (url.scheme() == "gdlookup" || url.scheme().compare( "bword" ) == 0)
  {
    QString def = url.path();
    if (def.startsWith("/"))
    {
      def = def.mid( 1 );
    }

    if( url.hasQueryItem( "dict" ) )
    {
      // Link to other dictionary
      QString dictName( url.queryItemValue( "dict" ) );
      if( !dictName.isEmpty() )
        msg = tr( "Definition from dictionary \"%1\": %2" ).arg( dictName ).arg( def );
    }

    if( msg.isEmpty() )
      msg = tr( "Definition: %1").arg( def );
  }
  else
  {
    msg = link;
  }

  emit statusBarMessage( msg );
}

void ArticleView::attachToJavaScript()
{
  ui.definition->page()->mainFrame()->addToJavaScriptWindowObject( QString( "articleview" ), this );
}

void ArticleView::linkClicked( QUrl const & url_ )
{
  updateCurrentArticleFromCurrentFrame();

  QUrl url( url_ );
  Contexts contexts;

  tryMangleWebsiteClickedUrl( url, contexts );

  Qt::KeyboardModifiers kmod = QApplication::keyboardModifiers();
  if ( !popupView &&
       ( ui.definition->isMidButtonPressed() ||
         ( kmod & ( Qt::ControlModifier | Qt::ShiftModifier ) ) ) )
  {
    // Mid button or Control/Shift is currently pressed - open the link in new tab
    emit openLinkInNewTab( url, ui.definition->url(), getCurrentArticle(), contexts );
  }
  else
    openLink( url, ui.definition->url(), getCurrentArticle(), contexts );
}

void ArticleView::openLink( QUrl const & url, QUrl const & ref,
                            QString const & scrollTo,
                            Contexts const & contexts )
{
  qDebug() << "clicked" << url;

  if( url.scheme().compare( "gdpicture" ) == 0 )
    ui.definition->load( url );
  else
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
    {
      QString newScrollTo( scrollTo );
      if( url.hasQueryItem( "dict" ) )
      {
        // Link to other dictionary
        QString dictName( url.queryItemValue( "dict" ) );
        for( unsigned i = 0; i < allDictionaries.size(); i++ )
        {
          if( dictName.compare( QString::fromUtf8( allDictionaries[ i ]->getName().c_str() ) ) == 0 )
          {
            newScrollTo = QString( "gdfrom-" ) + QString::fromUtf8( allDictionaries[ i ]->getId().c_str() );
            break;
          }
        }
      }
      showDefinition( url.path().mid( 1 ),
                      getGroup( ref ), newScrollTo, contexts );
    }
  }
  else
  if ( url.scheme() == "bres" || url.scheme() == "gdau" || url.scheme() == "gdvideo" ||
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

    if ( resourceDownloadRequests.empty() ) // No requests were queued
    {
      QMessageBox::critical( this, "GoldenDict", tr( "The referenced resource doesn't exist." ) );
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

          QMessageBox::critical( this, "GoldenDict",
                                 error );
        }

        return;
      }
    }

    // Still here? No such program exists.
    QMessageBox::critical( this, "GoldenDict",
                           tr( "The referenced audio program doesn't exist." ) );
  }
  else
  if ( url.scheme() == "gdtts" )
  {
// TODO: Port TTS
#if defined( Q_OS_WIN32 ) || defined( Q_OS_MACX )
    // Text to speech
    QString md5Id = url.queryItemValue( "engine" );
    QString text( url.path().mid( 1 ) );

    for ( Config::VoiceEngines::const_iterator i = cfg.voiceEngines.begin();
          i != cfg.voiceEngines.end(); ++i )
    {
      QString itemMd5Id = QString( QCryptographicHash::hash(
                                     i->id.toUtf8(),
                                     QCryptographicHash::Md5 ).toHex() );

      if ( itemMd5Id == md5Id )
      {
        SpeechClient * speechClient = new SpeechClient( *i, this );
        connect( speechClient, SIGNAL( finished() ), speechClient, SLOT( deleteLater() ) );
        speechClient->tell( text );
        break;
      }
    }
#endif
  }
  else
  if ( isExternalLink( url ) )
  {
    // Use the system handler for the conventional external links
    QDesktopServices::openUrl( url );
  }
}

vector< ResourceToSaveHandler * > ArticleView::saveResource( const QUrl & url, const QString & fileName )
{
  return saveResource( url, ui.definition->url(), fileName );
}

vector< ResourceToSaveHandler * > ArticleView::saveResource( const QUrl & url, const QUrl & ref, const QString & fileName )
{
  vector< ResourceToSaveHandler * > handlers;
  sptr< Dictionary::DataRequest > req;

  if( url.scheme() == "bres" || url.scheme() == "gico" || url.scheme() == "gdau")
  {
    if ( url.host() == "search" )
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
      {
        for( unsigned x = 0; x < activeDicts->size(); ++x )
        {
          req = (*activeDicts)[ x ]->getResource(
                  url.path().mid( 1 ).toUtf8().data() );

          ResourceToSaveHandler * handler = new ResourceToSaveHandler( this, req, fileName );
          handlers.push_back( handler );
        }
      }
    }
    else
    {
      // Normal resource download
      QString contentType;
      req = articleNetMgr.getResource( url, contentType );

      ResourceToSaveHandler * handler = new ResourceToSaveHandler( this, req, fileName );
      handlers.push_back( handler );
    }
  }
  else
  {
    req = new Dictionary::WebMultimediaDownload( url, articleNetMgr );

    ResourceToSaveHandler * handler = new ResourceToSaveHandler( this, req, fileName );
    handlers.push_back( handler );
  }

  if ( handlers.empty() ) // No requests were queued
  {
    emit statusBarMessage(
          tr( "ERROR: %1" ).arg( tr( "The referenced resource doesn't exist." ) ),
          10000, QPixmap( ":/icons/error.png" ) );
  }

  return handlers;
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

bool ArticleView::canGoBack()
{
  // First entry in a history is always an empty page,
  // so we skip it.
  return ui.definition->history()->currentItemIndex() > 1;
}

bool ArticleView::canGoForward()
{
  return ui.definition->history()->canGoForward();
}

void ArticleView::setSelectionBySingleClick( bool set )
{
  ui.definition->setSelectionBySingleClick( set );
}

void ArticleView::back()
{
  // Don't allow navigating back to page 0, which is usually the initial
  // empty page
  if ( canGoBack() )
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
  QVariant v = ui.definition->page()->mainFrame()->evaluateJavaScript( "gdAudioLinks.first" );
  if ( v.type() == QVariant::String )
    return !v.toString().isEmpty();
  return false;
}

void ArticleView::playSound()
{
  QVariant v;
  QString soundScript;

  v = ui.definition->page()->mainFrame()->evaluateJavaScript( "gdAudioLinks[gdAudioLinks.current]" );

  if ( v.type() == QVariant::String )
    soundScript = v.toString();

  // fallback to the first one
  if ( soundScript.isEmpty() )
  {
    v = ui.definition->page()->mainFrame()->evaluateJavaScript( "gdAudioLinks.first" );
    if ( v.type() == QVariant::String )
      soundScript = v.toString();
  }

  if ( !soundScript.isEmpty() )
    openLink( QUrl::fromEncoded( soundScript.toUtf8() ), ui.definition->url() );
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
  QAction * maxDictionaryRefsAction = 0;
  QAction * addWordToHistoryAction = 0;
  QAction * addHeaderToHistoryAction = 0;
  QAction * sendWordToInputLineAction = 0;
  QAction * saveImageAction = 0;
  QAction * saveSoundAction = 0;

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

#if QT_VERSION >= 0x040600
  QWebElement el = r.element();
  QUrl imageUrl;
  if( !popupView && el.tagName().compare( "img", Qt::CaseInsensitive ) == 0 )
  {
    imageUrl = QUrl::fromPercentEncoding( el.attribute( "src" ).toLatin1() );
    if( !imageUrl.isEmpty() )
    {
      menu.addAction( ui.definition->pageAction( QWebPage::CopyImageToClipboard ) );
      saveImageAction = new QAction( tr( "Save &image..." ), &menu );
      menu.addAction( saveImageAction );
    }
  }

  if( !popupView && ( targetUrl.scheme() == "gdau"
                      || Dictionary::WebMultimediaDownload::isAudioUrl( targetUrl ) ) )
  {
    saveSoundAction = new QAction( tr( "Save s&ound..." ), &menu );
    menu.addAction( saveSoundAction );
  }
#endif

  QString selectedText = ui.definition->selectedText();

  if ( selectedText.size() && selectedText.size() < 60 )
  {
    // We don't prompt for selections larger or equal to 60 chars, since
    // it ruins the menu and it's hardly a single word anyway.

    QString text = ui.definition->selectedText();
    if( text.isRightToLeft() )
    {
      text.insert( 0, (ushort)0x202E ); // RLE, Right-to-Left Embedding
      text.append( (ushort)0x202C ); // PDF, POP DIRECTIONAL FORMATTING
    }

    lookupSelection = new QAction( tr( "&Look up \"%1\"" ).
                                   arg( text ),
                                   &menu );
    menu.addAction( lookupSelection );

    if ( !popupView )
    {
      lookupSelectionNewTab = new QAction( QIcon( ":/icons/addtab.png" ),
                                           tr( "Look up \"%1\" in &New Tab" ).
                                           arg( text ),
                                           &menu );
      menu.addAction( lookupSelectionNewTab );

      sendWordToInputLineAction = new QAction( tr( "Send \"%1\" to input line" ).
                                               arg( text ),
                                               &menu );
      menu.addAction( sendWordToInputLineAction );
    }

    addWordToHistoryAction = new QAction( tr( "&Add \"%1\" to history" ).
                                          arg( text ),
                                          &menu );
    menu.addAction( addWordToHistoryAction );

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

  if( selectedText.isEmpty() && !cfg.preferences.storeHistory)
  {
      addHeaderToHistoryAction = new QAction( tr( "&Add \"%1\" to history" ).
                                            arg( ui.definition->title() ),
                                            &menu );
      menu.addAction( addHeaderToHistoryAction );
  }

  if ( selectedText.size() )
  {
    menu.addAction( ui.definition->pageAction( QWebPage::Copy ) );
    menu.addAction( &copyAsTextAction );
  }
  else
  {
    menu.addAction( &selectCurrentArticleAction );
    menu.addAction( ui.definition->pageAction( QWebPage::SelectAll ) );
  }

  map< QAction *, QString > tableOfContents;

  // Add table of contents
  QStringList ids = getArticlesList();

  if ( !menu.isEmpty() && ids.size() )
    menu.addSeparator();

  unsigned refsAdded = 0;
  bool maxDictionaryRefsReached = false;

  for( QStringList::const_iterator i = ids.constBegin(); i != ids.constEnd();
       ++i, ++refsAdded )
  {
    // Find this dictionary

    for( unsigned x = allDictionaries.size(); x--; )
    {
      if ( allDictionaries[ x ]->getId() == i->toUtf8().data() )
      {
        QAction * action = 0;
        if ( refsAdded == cfg.preferences.maxDictionaryRefsInContextMenu )
        {
          // Enough! Or the menu would become too large.
          maxDictionaryRefsAction = new QAction( ".........", &menu );
          action = maxDictionaryRefsAction;
          maxDictionaryRefsReached = true;
        }
        else
        {
          action = new QAction(
                  allDictionaries[ x ]->getIcon(),
                  QString::fromUtf8( allDictionaries[ x ]->getName().c_str() ),
                  &menu );
          // Force icons in menu on all platfroms,
          // since without them it will be much harder
          // to find things.
          action->setIconVisibleInMenu( true );
        }
        menu.addAction( action );

        tableOfContents[ action ] = *i;

        break;
      }
    }
    if( maxDictionaryRefsReached )
      break;
  }

  menu.addSeparator();
  menu.addAction( &inspectAction );

  if ( !menu.isEmpty() )
  {
    connect( this, SIGNAL( closePopupMenu() ), &menu, SLOT( close() ) );
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
    if ( result == addWordToHistoryAction )
      emit forceAddWordToHistory( selectedText );
    if ( result == addHeaderToHistoryAction )
      emit forceAddWordToHistory( ui.definition->title() );
    else
    if( result == sendWordToInputLineAction )
      emit sendWordToInputLine( selectedText );
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
    if( result == saveImageAction || result == saveSoundAction )
    {
#if QT_VERSION >= 0x040600
      QUrl url = ( result == saveImageAction ) ? imageUrl : targetUrl;
      QString savePath;
      QString fileName;

      if ( cfg.resourceSavePath.isEmpty() )
        savePath = QDir::homePath();
      else
      {
        savePath = QDir::fromNativeSeparators( cfg.resourceSavePath );
        if ( !QDir( savePath ).exists() )
          savePath = QDir::homePath();
      }

      QString name = url.path().section( '/', -1 );

      if ( result == saveSoundAction )
      {
        // Audio data
        if ( name.indexOf( '.' ) < 0 )
          name += ".wav";

        fileName = savePath + "/" + name;
        fileName = QFileDialog::getSaveFileName( parentWidget(), tr( "Save sound" ),
                                                 fileName,
                                                 tr( "Sound files (*.wav *.ogg *.mp3 *.mp4 *.aac *.flac *.mid *.wv *.ape);;All files (*.*)" ) );
      }
      else
      {
        // Image data

        // Check for babylon image name
        if ( name[ 0 ] == '\x1E' )
          name.remove( 0, 1 );
        if ( name.length() && name[ name.length() - 1 ] == '\x1F' )
          name.chop( 1 );

        fileName = savePath + "/" + name;
        fileName = QFileDialog::getSaveFileName( parentWidget(), tr( "Save image" ),
                                                 fileName,
                                                 tr( "Image files (*.bmp *.jpg *.png *.tif);;All files (*.*)" ) );
      }

      if ( !fileName.isEmpty() )
      {
        QFileInfo fileInfo( fileName );
        emit storeResourceSavePath( QDir::toNativeSeparators( fileInfo.absoluteDir().absolutePath() ) );
        saveResource( url, ui.definition->url(), fileName );
      }
#endif
    }
    else
    {
      if ( !popupView && result == maxDictionaryRefsAction )
        emit showDictsPane();

      // Match against table of contents
      QString id = tableOfContents[ result ];

      if ( id.size() )
        setCurrentArticle( "gdfrom-" + id, true );
    }
  }
#if 0
  DPRINTF( "%s\n", r.linkUrl().isEmpty() ? "null" : "not null" );

  DPRINTF( "url = %s\n", r.linkUrl().toString().toLocal8Bit().data() );
  DPRINTF( "title = %s\n", r.title().toLocal8Bit().data() );
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
#ifndef DISABLE_INTERNAL_PLAYER
          if ( cfg.preferences.useInternalPlayer )
          {
            Ffmpeg::AudioPlayer & player = Ffmpeg::AudioPlayer::instance();
            connect( &player, SIGNAL( error( QString ) ), this, SLOT( audioPlayerError( QString ) ), Qt::UniqueConnection );
            player.playMemory( data.data(), data.size() );
          }
          else
#endif
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
              QMessageBox::critical( this, "GoldenDict", tr( "Failed to run a player to play sound file: %1" ).arg( e.what() ) );
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

            if ( !tmp.open() || (size_t) tmp.write( &data.front(), data.size() ) != data.size() )
            {
              QMessageBox::critical( this, "GoldenDict", tr( "Failed to create temporary file." ) );
              return;
            }

            tmp.setAutoRemove( false );

            desktopOpenedTempFile = tmp.fileName();
          }

          if ( !QDesktopServices::openUrl( QUrl::fromLocalFile( desktopOpenedTempFile ) ) )
            QMessageBox::critical( this, "GoldenDict",
                                   tr( "Failed to auto-open resource file, try opening manually: %1." ).arg( desktopOpenedTempFile ) );
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
    emit statusBarMessage(
          tr( "WARNING: %1" ).arg( tr( "The referenced resource failed to download." ) ),
          10000, QPixmap( ":/icons/error.png" ) );
  }
}

void ArticleView::audioPlayerError( QString const & message )
{
  emit statusBarMessage( tr( "WARNING: FFmpeg Audio Player: %1" ).arg( message ),
                         10000, QPixmap( ":/icons/error.png" ) );
}

void ArticleView::pasteTriggered()
{
  QString text =
      gd::toQString(
          Folding::trimWhitespaceOrPunct(
              gd::toWString(
                  QApplication::clipboard()->text() ) ) );

  if ( text.size() )
  {
    unsigned groupId = getGroup( ui.definition->url() );
    if ( groupId == 0 )
    {
      // We couldn't figure out the group out of the URL,
      // so let's try the currently selected group.
      groupId = groupComboBox->getCurrentGroup();
    }
    showDefinition( text, groupId, getCurrentArticle() );
  }
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
  if ( searchIsOpened )
    performFindOperation( false, true );
}

void ArticleView::on_searchNext_clicked()
{
  if ( searchIsOpened )
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

void ArticleView::on_searchCaseSensitive_clicked()
{
  performFindOperation( false, false, true );
}

void ArticleView::on_highlightAllButton_clicked()
{
  performFindOperation( false, false, true );
}

void ArticleView::onJsActiveArticleChanged(QString const & id)
{
  if ( !id.startsWith( "gdfrom-" ) )
    return; // Incorrect id

  emit activeArticleChanged( id.mid( 7 ) );
}

void ArticleView::doubleClicked()
{
  // We might want to initiate translation of the selected word

  if ( cfg.preferences.doubleClickTranslates )
  {
    QString selectedText = ui.definition->selectedText();

    // Do some checks to make sure there's a sensible selection indeed
    if ( Folding::applyWhitespaceOnly( gd::toWString( selectedText ) ).size() &&
         selectedText.size() < 60 )
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


void ArticleView::performFindOperation( bool restart, bool backwards, bool checkHighlight )
{
  QString text = ui.searchText->text();

  if ( restart || checkHighlight )
  {
    if( restart ) {
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
#if QT_VERSION >= 0x040600
    f |= QWebPage::HighlightAllOccurrences;
#endif

    ui.definition->findText( "", f );

    if( ui.highlightAllButton->isChecked() )
      ui.definition->findText( text, f );

    if( checkHighlight )
      return;
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

bool ArticleView::isSearchOpened()
{
  return searchIsOpened;
}

void ArticleView::showEvent( QShowEvent * ev )
{
  QFrame::showEvent( ev );

  if ( !searchIsOpened )
    ui.searchFrame->hide();
}

void ArticleView::receiveExpandOptionalParts( bool expand )
{
  if( expandOptionalParts != expand )
  {
    int n = getArticlesList().indexOf( getActiveArticleId() );
    if( n > 0 )
       articleToJump = getCurrentArticle();

    emit setExpandMode( expand );
    expandOptionalParts = expand;
    reload();
  }
}

void ArticleView::switchExpandOptionalParts()
{
  expandOptionalParts = !expandOptionalParts;

  int n = getArticlesList().indexOf( getActiveArticleId() );
  if( n > 0 )
    articleToJump = getCurrentArticle();

  emit setExpandMode( expandOptionalParts );
  reload();
}

void ArticleView::copyAsText()
{
  QString text = ui.definition->selectedText();
  if( !text.isEmpty() )
    QApplication::clipboard()->setText( text );
}

void ArticleView::inspect()
{
  ui.definition->triggerPageAction( QWebPage::InspectElement );
}

#ifdef Q_OS_WIN32

void ArticleView::readTag( const QString & from, QString & to, int & count )
{
    QChar ch, prev_ch;
    bool inQuote = false, inDoublequote = false;

    to.append( ch = prev_ch = from[ count++ ] );
    while( count < from.size() )
    {
        ch = from[ count ];
        if( ch == '>' && !( inQuote || inDoublequote ) )
        {
            to.append( ch );
            break;
        }
        if( ch == '\'' )
            inQuote = !inQuote;
        if( ch == '\"' )
            inDoublequote = !inDoublequote;
        to.append( prev_ch = ch );
        count++;
    }
}

QString ArticleView::insertSpans( QString const & html )
{
    QChar ch;
    QString newContent;
    bool inSpan = false, escaped = false;

    /// Enclose every word in string (exclude tags) with <span></span>

    for( int i = 0; i < html.size(); i++ )
    {
        ch = html[ i ];
        if( ch == '&' )
        {
            escaped = true;
            if( inSpan )
            {
                newContent.append( "</span>" );
                inSpan = false;
            }
            newContent.append( ch );
            continue;
        }

        if( ch == '<' ) // Skip tag
        {
            escaped = false;
            if( inSpan )
            {
                newContent.append( "</span>" );
                inSpan = false;
            }
            readTag( html, newContent, i );
            continue;
        }

        if( escaped )
        {
            if( ch == ';' )
                escaped = false;
            newContent.append( ch );
            continue;
        }

        if( !inSpan && ( ch.isLetterOrNumber() || ch.isLowSurrogate() ) )
        {
            newContent.append( "<span>");
            inSpan = true;
        }

        if( inSpan && !( ch.isLetterOrNumber() || ch.isLowSurrogate() ) )
        {
            newContent.append( "</span>");
            inSpan = false;
        }

        if( ch.isLowSurrogate() )
        {
            newContent.append( ch );
            ch = html[ ++i ];
        }

        newContent.append( ch );
        if( ch == '-' && !( html[ i + 1 ] == ' ' || ( i > 0 && html[ i - 1 ] == ' ' ) ) )
            newContent.append( "<span style=\"font-size:0pt\"> </span>" );
    }
    if( inSpan )
        newContent.append( "</span>" );
    return newContent;
}

QString ArticleView::checkElement( QWebElement & elem, QPoint const & pt )
{
    /// Search for lower-level matching element

    QWebElement parentElem = elem;
    QWebElement childElem = elem.firstChild();
    while( !childElem.isNull() )
    {
      if( childElem.geometry().contains( pt ) )
      {
        parentElem = childElem;
        childElem = parentElem.firstChild();
        continue;
      }
      childElem = childElem.nextSibling();
    }

    return parentElem.toPlainText();
}

QString ArticleView::wordAtPoint( int x, int y )
{
  QString word;

  if( popupView )
    return word;

  QPoint pos = mapFromGlobal( QPoint( x, y ) );
  QWebFrame *frame = ui.definition->page()->frameAt( pos );
  if( !frame )
    return word;

  QPoint posWithScroll = pos + frame->scrollPosition();

  /// Find target HTML element

  QWebHitTestResult result = frame->hitTestContent( pos );
  QWebElement baseElem = result.enclosingBlockElement();

  if( baseElem.tagName().compare( "BODY" ) == 0 ||      /// Assume empty field position
      baseElem.tagName().compare( "HTML" ) == 0 ||
      baseElem.tagName().compare( "HEAD" ) == 0 )
    return word;

  /// Save selection position

  baseElem.evaluateJavaScript( "var __gd_sel=window.getSelection();"
                               "if(__gd_sel && __gd_sel.rangeCount>0) {"
                                 "__gd_SelRange=__gd_sel.getRangeAt(0);"
                                 "if(__gd_SelRange.collapsed) __gd_sel.removeAllRanges();"
                                 "else {"
                                   "__gd_StartTree=[]; __gd_EndTree=[];"
                                   "var __gd_baseRange=document.createRange();"
                                   "__gd_baseRange.selectNode(this);"
                                   "if(__gd_baseRange.comparePoint(__gd_SelRange.startContainer,0)==0) {"
                                     "__gd_StartOffset=__gd_SelRange.startOffset;"
                                     "var __gd_child=__gd_SelRange.startContainer;"
                                     "var __gd_parent='';"
                                     "if(__gd_child==this) __gd_StartTree.push(-1);"
                                     "else while(__gd_parent!=this) {"
                                       "var n=0; __gd_parent=__gd_child.parentNode;"
                                       "var __gd_el=__gd_parent.firstChild;"
                                       "while(__gd_el!=__gd_child) { n++; __gd_el=__gd_el.nextSibling; }"
                                       "__gd_StartTree.push(n);"
                                       "__gd_child=__gd_parent;"
                                     "}"
                                   "}"
                                   "if(__gd_baseRange.comparePoint(__gd_SelRange.endContainer,0)==0) {"
                                     "__gd_EndOffset=__gd_SelRange.endOffset;"
                                     "var __gd_child=__gd_SelRange.endContainer;"
                                     "var __gd_parent='';"
                                     "if(__gd_child==this) __gd_EndTree.push(-1);"
                                     "else while(__gd_parent!=this) {"
                                       "var n=0; __gd_parent=__gd_child.parentNode;"
                                       "var __gd_el=__gd_parent.firstChild;"
                                       "while(__gd_el!=__gd_child) { n++; __gd_el=__gd_el.nextSibling; }"
                                       "__gd_EndTree.push(n);"
                                       "__gd_child=__gd_parent;"
                                     "}"
                                   "}"
                                 "}"
                               "}"
                               );

  /// Enclose every word be <span> </span>

  QString content = baseElem.toInnerXml();
  QString newContent = insertSpans( content );

  /// Set new code and re-render it to fill geometry

  QImage img( baseElem.geometry().width(), baseElem.geometry().height(), QImage::Format_Mono );
  img.fill( 0 );
  QPainter painter( & img );

  baseElem.setInnerXml( newContent );
  baseElem.render( &painter );

  /// Search in all child elements and check it

  QWebElementCollection elemCollection = baseElem.findAll( "*" );
  foreach ( QWebElement elem, elemCollection )
  {
      if( elem.geometry().contains( posWithScroll ) )
          word = checkElement( elem, posWithScroll );
      if( !word.isEmpty() )
          break;
  }

  /// Restore old content
  baseElem.setInnerXml( content );

  /// Restore selection

  baseElem.evaluateJavaScript( "var flag=0;"
                               "if(__gd_StartTree && __gd_StartTree.length) {"
                                 "var __gd_el=this;"
                                 "while(__gd_StartTree.length) {"
                                   "__gd_el=__gd_el.firstChild;"
                                   "var n=__gd_StartTree.pop();"
                                   "if(n<0) __gd_el=this;"
                                   "else for(var i=0;i<n;i++) __gd_el=__gd_el.nextSibling;"
                                 "}"
                                 "__gd_SelRange.setStart(__gd_el, __gd_StartOffset);"
                                 "__gd_StartTree.splice(0,__gd_StartTree.length);"
                                 "flag+=1;"
                               "}"
                               "if(__gd_EndTree && __gd_EndTree.length) {"
                                 "var __gd_el=this;"
                                 "while(__gd_EndTree.length) {"
                                   "__gd_el=__gd_el.firstChild;"
                                   "var n=__gd_EndTree.pop();"
                                   "if(n<0) __gd_el=this;"
                                   "else for(var i=0;i<n;i++) __gd_el=__gd_el.nextSibling;"
                                 "}"
                                 "__gd_SelRange.setEnd(__gd_el, __gd_EndOffset);"
                                 "__gd_EndTree.splice(0,__gd_EndTree.length);"
                                 "flag+=1;"
                               "}"
                               "if(flag>0) {"
                                 "var __gd_sel=window.getSelection();"
                                 "__gd_sel.removeAllRanges();"
                                 "__gd_sel.addRange(__gd_SelRange);"
                               "}"
                               );

  return word;
}

#endif

ResourceToSaveHandler::ResourceToSaveHandler(
    ArticleView * view, sptr< Dictionary::DataRequest > req,
    QString const & fileName ) :
  QObject( view ),
  req( req ),
  fileName( fileName )
{
  connect( this, SIGNAL( statusBarMessage( QString, int, QPixmap ) ),
           view, SIGNAL( statusBarMessage( QString, int, QPixmap ) ) );

  // If DataRequest finsihed immediately, call our handler directly
  if ( req.get()->isFinished() )
  {
    QMetaObject::invokeMethod( this, "downloadFinished", Qt::QueuedConnection );
  }
  else
  {
    connect( req.get(), SIGNAL( finished() ), this, SLOT( downloadFinished() ) );
  }
}

void ResourceToSaveHandler::downloadFinished()
{
  assert( req && req.get()->isFinished() );

  QByteArray resourceData;

  if ( req.get()->dataSize() >= 0 )
  {
    vector< char > const & data = req.get()->getFullData();
    resourceData = QByteArray( data.data(), data.size() );
  }

  // Write data to file

  if ( !resourceData.isEmpty() && !fileName.isEmpty() )
  {
    QFileInfo fileInfo( fileName );
    QDir().mkpath( fileInfo.absoluteDir().absolutePath() );

    QFile file( fileName );
    if ( file.open( QFile::WriteOnly ) )
    {
      file.write( resourceData.data(), resourceData.size() );
      file.close();
    }

    if ( file.error() )
    {
      emit statusBarMessage(
            tr( "ERROR: %1" ).arg( tr( "Resource saving error: " ) + file.errorString() ),
            10000, QPixmap( ":/icons/error.png" ) );
    }
  }
  else
  {
    emit statusBarMessage(
          tr( "ERROR: %1" ).arg( tr( "The referenced resource failed to download." ) ),
          10000, QPixmap( ":/icons/error.png" ) );
  }

  emit done();
  deleteLater();
}
