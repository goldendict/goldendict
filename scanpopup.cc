/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "scanpopup.hh"
#include "folding.hh"
#include "wstring_qt.hh"
#include <QUrl>
#include <QCursor>
#include <QPixmap>
#include <QBitmap>
#include <QMenu>
#include <QMouseEvent>
#include <QDesktopWidget>
#include "dprintf.hh"

#ifdef Q_OS_MACX
#include "macmouseover.hh"
#define MouseOver MacMouseOver
#else
#include "mouseover.hh"
#endif

using std::wstring;

/// We use different window flags under Windows and X11 due to slight differences
/// in their behavior on those platforms.
static Qt::WindowFlags popupWindowFlags =

#ifdef Q_WS_WIN
Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
#else
Qt::Popup
#endif
;

ScanPopup::ScanPopup( QWidget * parent,
                      Config::Class & cfg_,
                      ArticleNetworkAccessManager & articleNetMgr,                      
                      std::vector< sptr< Dictionary::Class > > const & allDictionaries_,
                      Instances::Groups const & groups_,
                      History & history_ ):
  QMainWindow( parent ),
  cfg( cfg_ ),
  isScanningEnabled( false ),
  allDictionaries( allDictionaries_ ),
  groups( groups_ ),
  history( history_ ),
  escapeAction( this ),
  switchExpandModeAction( this ),
  focusTranslateLineAction( this ),
  wordFinder( this ),
  dictionaryBar( this, configEvents, cfg.editDictionaryCommandLine, cfg.preferences.maxDictionaryRefsInContextMenu ),
  mouseEnteredOnce( false ),
  mouseIntercepted( false ),
  hideTimer( this )
{
  ui.setupUi( this );

  mainStatusBar = new MainStatusBar( this );

  ui.queryError->hide();

  definition = new ArticleView( ui.outerFrame, articleNetMgr, allDictionaries,
                                groups, true, cfg,
                                dictionaryBar.toggleViewAction()
                                );

  connect( this, SIGNAL(switchExpandMode() ),
           definition, SLOT( switchExpandOptionalParts() ) );
  connect( this, SIGNAL(setViewExpandMode( bool ) ),
           definition, SLOT( receiveExpandOptionalParts( bool ) ) );
  connect( definition, SIGNAL( setExpandMode( bool ) ),
           this, SIGNAL( setExpandMode( bool ) ) );
  connect( definition, SIGNAL( forceAddWordToHistory( QString ) ),
           this, SIGNAL( forceAddWordToHistory( QString ) ) );
  connect( this, SIGNAL( closeMenu() ),
           definition, SIGNAL( closePopupMenu() ) );
  connect( definition, SIGNAL( sendWordToHistory( QString ) ),
           this, SIGNAL( sendWordToHistory( QString ) ) );
  connect( definition, SIGNAL( typingEvent( QString const & ) ),
           this, SLOT( typingEvent( QString const & ) ) );

  applyZoomFactor();
  
  ui.mainLayout->addWidget( definition );

  ui.translateBox->wordList()->attachFinder( &wordFinder );
  ui.translateBox->wordList()->setFocusPolicy(Qt::ClickFocus);
  ui.translateBox->translateLine()->installEventFilter( this );

  connect( ui.translateBox->translateLine(), SIGNAL( textChanged( QString const & ) ),
           this, SLOT( translateInputChanged( QString const & ) ) );

  connect( ui.translateBox->translateLine(), SIGNAL( returnPressed() ),
           this, SLOT( translateInputFinished() ) );

  connect( ui.translateBox->wordList(), SIGNAL( itemClicked( QListWidgetItem * ) ),
           this, SLOT( wordListItemActivated( QListWidgetItem * ) ) );

  connect( ui.translateBox->wordList(), SIGNAL( itemDoubleClicked ( QListWidgetItem * ) ),
           this, SLOT( wordListItemActivated( QListWidgetItem * ) ) );

  connect( ui.translateBox->wordList(), SIGNAL( statusBarMessage( QString const &, int, QPixmap const & ) ),
           this, SLOT( showStatusBarMessage( QString const &, int, QPixmap const & ) ) );

  ui.pronounceButton->hide();

  ui.groupList->fill( groups );
  ui.groupList->setCurrentGroup( cfg.lastPopupGroupId );

  dictionaryBar.setFloatable( false );

  Instances::Group const * igrp = groups.findGroup( cfg.lastPopupGroupId );
  if( cfg.lastPopupGroupId == Instances::Group::AllGroupId )
  {
    if( igrp )
      igrp->checkMutedDictionaries( &cfg.popupMutedDictionaries );
    dictionaryBar.setMutedDictionaries( &cfg.popupMutedDictionaries );
  }
  else
  {
    Config::Group * grp = cfg.getGroup( cfg.lastPopupGroupId );
    if( igrp && grp )
      igrp->checkMutedDictionaries( &grp->popupMutedDictionaries );
    dictionaryBar.setMutedDictionaries( grp ? &grp->popupMutedDictionaries : 0 );
  }

  addToolBar( Qt::RightToolBarArea, &dictionaryBar );

  connect( &dictionaryBar, SIGNAL(editGroupRequested()),
           this, SLOT(editGroupRequested()) );
  connect( this, SIGNAL( closeMenu() ),
           &dictionaryBar, SIGNAL( closePopupMenu() ) );
  connect( &dictionaryBar, SIGNAL( showDictionaryInfo( QString const & ) ),
           this, SIGNAL( showDictionaryInfo( QString const & ) ) );
  connect( &dictionaryBar, SIGNAL( openDictionaryFolder( QString const & ) ),
           this, SIGNAL( openDictionaryFolder( QString const & ) ) );

  if ( cfg.popupWindowGeometry.size() )
    restoreGeometry( cfg.popupWindowGeometry );

  if ( cfg.popupWindowState.size() )
    restoreState( cfg.popupWindowState, 1 );

  ui.pinButton->setChecked( cfg.pinPopupWindow );

  if ( cfg.pinPopupWindow )
  {
    dictionaryBar.setMovable( true );
    setWindowFlags( Qt::Dialog );
  }
  else
  {
    dictionaryBar.setMovable( false );
    setWindowFlags( popupWindowFlags );
  }

  connect( &configEvents, SIGNAL( mutedDictionariesChanged() ),
           this, SLOT( mutedDictionariesChanged() ) );

  definition->focus();

  #if 0 // Experimental code to give window a non-rectangular shape (i.e.
        // balloon) using a colorkey mask.
  QPixmap pixMask( size() );
  render( &pixMask );

  setMask( pixMask.createMaskFromColor( QColor( 255, 0, 0 ) ) );

  // This helps against flickering
  setAttribute( Qt::WA_NoSystemBackground );
  #endif

  escapeAction.setShortcut( QKeySequence( "Esc" ) );
  addAction( &escapeAction );
  connect( &escapeAction, SIGNAL( triggered() ),
           this, SLOT( escapePressed() ) );

  focusTranslateLineAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );
  addAction( &focusTranslateLineAction );
  focusTranslateLineAction.setShortcuts( QList< QKeySequence >() <<
                                         QKeySequence( "Alt+D" ) <<
                                         QKeySequence( "Ctrl+L" ) );

  connect( &focusTranslateLineAction, SIGNAL( triggered() ),
           this, SLOT( focusTranslateLine() ) );


  switchExpandModeAction.setShortcuts( QList< QKeySequence >() <<
                                       QKeySequence( Qt::CTRL + Qt::Key_8 ) <<
                                       QKeySequence( Qt::CTRL + Qt::Key_Asterisk ) <<
                                       QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_8 ) );

  addAction( &switchExpandModeAction );
  connect( &switchExpandModeAction, SIGNAL( triggered() ),
           this, SLOT(switchExpandOptionalPartsMode() ) );

  connect( ui.groupList, SIGNAL( currentIndexChanged( QString const & ) ),
           this, SLOT( currentGroupChanged( QString const & ) ) );

  connect( &wordFinder, SIGNAL( finished() ),
           this, SLOT( prefixMatchFinished() ) );

  connect( ui.pinButton, SIGNAL( clicked( bool ) ),
           this, SLOT( pinButtonClicked( bool ) ) );

  connect( definition, SIGNAL( pageLoaded( ArticleView * ) ),
           this, SLOT( pageLoaded( ArticleView * ) ) );

  connect( definition, SIGNAL( statusBarMessage( QString const &, int, QPixmap const & ) ),
           this, SLOT( showStatusBarMessage( QString const &, int, QPixmap const & ) ) );

#ifdef Q_WS_X11
  connect( QApplication::clipboard(), SIGNAL( changed( QClipboard::Mode ) ),
           this, SLOT( clipboardChanged( QClipboard::Mode ) ) );
#endif

  connect( &MouseOver::instance(), SIGNAL( hovered( QString const &, bool ) ),
           this, SLOT( mouseHovered( QString const &, bool ) ) );

#ifdef Q_OS_WIN32
  connect( &MouseOver::instance(), SIGNAL( isGoldenDictWindow( HWND ) ),
           this, SIGNAL( isGoldenDictWindow( HWND ) ) );
#endif

  hideTimer.setSingleShot( true );
  hideTimer.setInterval( 400 );

  connect( &hideTimer, SIGNAL( timeout() ),
           this, SLOT( hideTimerExpired() ) );

  altModeExpirationTimer.setSingleShot( true );
  altModeExpirationTimer.setInterval( cfg.preferences.scanPopupAltModeSecs * 1000 );

  connect( &altModeExpirationTimer, SIGNAL( timeout() ),
           this, SLOT( altModeExpired() ) );

  // This one polls constantly for modifiers while alt mode lasts
  altModePollingTimer.setSingleShot( false );
  altModePollingTimer.setInterval( 50 );
  connect( &altModePollingTimer, SIGNAL( timeout() ),
           this, SLOT( altModePoll() ) );

  mouseGrabPollTimer.setSingleShot( false );
  mouseGrabPollTimer.setInterval( 10 );
  connect( &mouseGrabPollTimer, SIGNAL( timeout() ),
           this, SLOT(mouseGrabPoll())  );

  MouseOver::instance().setPreferencesPtr( &( cfg.preferences ) );

  ui.goBackButton->setEnabled( false );
  ui.goForwardButton->setEnabled( false );

}

ScanPopup::~ScanPopup()
{
  // Save state, geometry and pin status
  cfg.popupWindowState = saveState( 1 );
  cfg.popupWindowGeometry = saveGeometry();
  cfg.pinPopupWindow = ui.pinButton->isChecked();

  disableScanning();
}

void ScanPopup::enableScanning()
{
  if ( !isScanningEnabled )
  {
    isScanningEnabled = true;
    MouseOver::instance().enableMouseOver();
  }
}

void ScanPopup::disableScanning()
{
  if ( isScanningEnabled )
  {
    MouseOver::instance().disableMouseOver();
    isScanningEnabled = false;
  }
}

void ScanPopup::applyZoomFactor()
{
  definition->setZoomFactor( cfg.preferences.zoomFactor );
}

void ScanPopup::translateWordFromClipboard()
{
	return translateWordFromClipboard(QClipboard::Clipboard);
}

void ScanPopup::translateWordFromSelection()
{
	return translateWordFromClipboard(QClipboard::Selection);
}

void ScanPopup::editGroupRequested()
{
  emit editGroupRequested( ui.groupList->getCurrentGroup() );
}

void ScanPopup::translateWordFromClipboard(QClipboard::Mode m)
{
  DPRINTF( "translating from clipboard or selection\n" );

  QString subtype = "plain";

  QString str = QApplication::clipboard()->text( subtype, m);

  translateWord( str );
}

void ScanPopup::translateWord( QString const & word )
{
  QString str = pendingInputWord = gd::toQString( Folding::trimWhitespaceOrPunct( gd::toWString( word ) ) ).simplified();

  if ( !str.size() )
    return; // Nothing there

  // In case we had any timers engaged before, cancel them now.
  altModePollingTimer.stop();
  altModeExpirationTimer.stop();

  inputWord = str;
  engagePopup( false,
#ifdef Q_WS_WIN
      true // We only focus popup under Windows when activated via Ctrl+C+C
           // -- on Linux it already has an implicit focus
#else
      false
#endif
      );
}

void ScanPopup::clipboardChanged( QClipboard::Mode m )
{
  if ( !isScanningEnabled )
    return;
  
  DPRINTF( "clipboard changed\n" );

  QString subtype = "plain";

  handleInputWord( QApplication::clipboard()->text( subtype, m ) );
}

void ScanPopup::mouseHovered( QString const & str, bool forcePopup )
{
  handleInputWord( str, forcePopup );
}

void ScanPopup::handleInputWord( QString const & str, bool forcePopup )
{
  QString sanitizedStr = gd::toQString( Folding::trimWhitespaceOrPunct( gd::toWString( str ) ) );

  if ( isVisible() && sanitizedStr == inputWord )
  {
    // Attempt to translate the same word we already have shown in scan popup.
    // Ignore it, as it is probably a spurious mouseover event.
    return;
  }

  pendingInputWord = sanitizedStr;

  if ( !pendingInputWord.size() )
  {
    if ( cfg.preferences.scanPopupAltMode )
    {
      // In case we had any timers engaged before, cancel them now, since
      // we're not going to translate anything anymore.
      altModePollingTimer.stop();
      altModeExpirationTimer.stop();
    }
    return;
  }

  // Check key modifiers

  if ( cfg.preferences.enableScanPopupModifiers && !checkModifiersPressed( cfg.preferences.scanPopupModifiers ) )
  {
    if ( cfg.preferences.scanPopupAltMode )
    {
      altModePollingTimer.start();
      altModeExpirationTimer.start();
    }

    return;
  }

  inputWord = pendingInputWord;
  engagePopup( forcePopup );
}

void ScanPopup::engagePopup( bool forcePopup, bool giveFocus )
{
  if( cfg.preferences.scanToMainWindow && !forcePopup )
  {
    // Send translated word to main window istead of show popup
    emit sendWordToMainWindow( inputWord );
    return;
  }

  definition->setSelectionBySingleClick( cfg.preferences.selectWordBySingleClick );

  /// Too large strings make window expand which is probably not what user
  /// wants
  ui.translateBox->setText( inputWord, false );
 
  if ( !isVisible() )
  {
    // Need to show the window

    if ( !ui.pinButton->isChecked() )
    {
      // Decide where should the window land

      QPoint currentPos = QCursor::pos();

      QRect desktop = QApplication::desktop()->screenGeometry();

      QSize windowSize = geometry().size();

      int x, y;

      /// Try the to-the-right placement
      if ( currentPos.x() + 4 + windowSize.width() <= desktop.topRight().x() )
        x = currentPos.x() + 4;
      else
      /// Try the to-the-left placement
      if ( currentPos.x() - 4 - windowSize.width() >= desktop.x() )
        x = currentPos.x() - 4 - windowSize.width();
      else
      // Center it
        x = desktop.x() + ( desktop.width() - windowSize.width() ) / 2;

      /// Try the to-the-bottom placement
      if ( currentPos.y() + 15 + windowSize.height() <= desktop.bottomLeft().y() )
        y = currentPos.y() + 15;
      else
      /// Try the to-the-top placement
      if ( currentPos.y() - 15 - windowSize.height() >= desktop.y() )
        y = currentPos.y() - 15 - windowSize.height();
      else
      // Center it
        y = desktop.y() + ( desktop.height() - windowSize.height() ) / 2;

      move( x, y );
    }

    show();

    if ( giveFocus )
    {
      activateWindow();
      raise();
    }

    if ( !ui.pinButton->isChecked() )
    {
      mouseEnteredOnce = false;
      // Need to monitor the mouse so we know when to hide the window
      interceptMouse();
    }

    // This produced some funky mouse grip-related bugs so we commented it out
    //QApplication::processEvents(); // Make window appear immediately no matter what
  }
  else
  if ( ui.pinButton->isChecked() )
  {
    // Pinned-down window isn't always on top, so we need to raise it
    show();
    activateWindow();
    raise();
  }

  if ( ui.pinButton->isChecked() )
       setWindowTitle( tr( "%1 - %2" ).arg( elideInputWord(), "GoldenDict" ) );

  showTranslationFor( inputWord );
}

QString ScanPopup::elideInputWord()
{
  return inputWord.size() > 32 ? inputWord.mid( 0, 32 ) + "..." : inputWord;
}

void ScanPopup::currentGroupChanged( QString const & )
{
    cfg.lastPopupGroupId = ui.groupList->getCurrentGroup();
    Instances::Group const * igrp = groups.findGroup( cfg.lastPopupGroupId );
    if( cfg.lastPopupGroupId == Instances::Group::AllGroupId )
    {
      if( igrp )
        igrp->checkMutedDictionaries( &cfg.popupMutedDictionaries );
      dictionaryBar.setMutedDictionaries( &cfg.popupMutedDictionaries );
    }
    else
    {
      Config::Group * grp = cfg.getGroup( cfg.lastPopupGroupId );
      if( grp )
      {
        if( igrp )
          igrp->checkMutedDictionaries( &grp->popupMutedDictionaries );
        dictionaryBar.setMutedDictionaries( &grp->popupMutedDictionaries );
      }
      else
        dictionaryBar.setMutedDictionaries( 0 );
    }

  updateDictionaryBar();

  if ( isVisible() )
  {
    translateInputChanged( ui.translateBox->translateLine()->text() );
    translateInputFinished();
  }

  cfg.lastPopupGroupId = ui.groupList->getCurrentGroup();
}

void ScanPopup::wordListItemActivated( QListWidgetItem * item )
{
  showTranslationFor( item->text() );
}

void ScanPopup::translateInputChanged( QString const & text )
{
  mainStatusBar->clearMessage();
  ui.translateBox->wordList()->setCurrentItem( 0, QItemSelectionModel::Clear );

  QString req = text.trimmed();

  if ( !req.size() )
  {
    // An empty request always results in an empty result
    wordFinder.cancel();
    ui.translateBox->wordList()->clear();
    ui.translateBox->wordList()->unsetCursor();

    // Reset the noResults mark if it's on right now
    if ( ui.translateBox->translateLine()->property( "noResults" ).toBool() )
    {
      ui.translateBox->translateLine()->setProperty( "noResults", false );
      setStyleSheet( styleSheet() );
    }
    return;
  }

  ui.translateBox->wordList()->setCursor( Qt::WaitCursor );

  wordFinder.prefixMatch( req, getActiveDicts() );
}

void ScanPopup::translateInputFinished()
{
  inputWord = ui.translateBox->translateLine()->text().trimmed();
  showTranslationFor( inputWord );
}

void ScanPopup::showTranslationFor( QString const & inputWord )
{
  ui.pronounceButton->hide();

  definition->showDefinition( inputWord, ui.groupList->getCurrentGroup() );
  definition->focus();

  // Add to history
  emit sendWordToHistory( inputWord.trimmed() );
}

vector< sptr< Dictionary::Class > > const & ScanPopup::getActiveDicts()
{
  int current = ui.groupList->currentIndex();

  if ( current < 0 || current >= (int) groups.size() )
  {
    // This shouldn't ever happen
    return allDictionaries;
  }

  Config::MutedDictionaries const * mutedDictionaries = dictionaryBar.getMutedDictionaries();
  if ( !dictionaryBar.toggleViewAction()->isChecked() || mutedDictionaries == 0 )
    return groups[ current ].dictionaries;
  else
  {
    vector< sptr< Dictionary::Class > > const & activeDicts =
      groups[ current ].dictionaries;

    // Populate the special dictionariesUnmuted array with only unmuted
    // dictionaries

    dictionariesUnmuted.clear();
    dictionariesUnmuted.reserve( activeDicts.size() );

    for( unsigned x = 0; x < activeDicts.size(); ++x )
      if ( !mutedDictionaries->contains(
              QString::fromStdString( activeDicts[ x ]->getId() ) ) )
        dictionariesUnmuted.push_back( activeDicts[ x ] );

    return dictionariesUnmuted;
  }
}

void ScanPopup::typingEvent( QString const & t )
{
  if ( t == "\n" || t == "\r" )
  {
    focusTranslateLine();
  }
  else
  {
    ui.translateBox->translateLine()->setFocus();
    ui.translateBox->setText( t, true );
    ui.translateBox->translateLine()->setCursorPosition( t.size() );
  }
}

bool ScanPopup::eventFilter( QObject * watched, QEvent * event )
{
  if ( event->type() == QEvent::FocusIn && watched == ui.translateBox->translateLine() )
  {
    QFocusEvent * focusEvent = static_cast< QFocusEvent * >( event );

    // select all on mouse click
    if ( focusEvent->reason() == Qt::MouseFocusReason ) {
      QTimer::singleShot(0, this, SLOT(focusTranslateLine()));
    }
    return false;
  }

  if ( mouseIntercepted )
  {
    // We're only interested in our events

    if ( event->type() == QEvent::MouseMove )
    {
//    DPRINTF( "Object: %s\n", watched->objectName().toUtf8().data() );
      QMouseEvent * mouseEvent = ( QMouseEvent * ) event;
      reactOnMouseMove( mouseEvent->globalPos() );
    }
  }

  return QMainWindow::eventFilter( watched, event );
}

void ScanPopup::reactOnMouseMove( QPoint const & p )
{
  if ( geometry().contains( p ) )
  {
//        DPRINTF( "got inside\n" );

    hideTimer.stop();
    mouseEnteredOnce = true;
    uninterceptMouse();
  }
  else
  {
//        DPRINTF( "outside\n" );
    // We're in grab mode and outside the window - calculate the
    // distance from it. We might want to hide it.

    // When the mouse has entered once, we don't allow it stayng outside,
    // but we give a grace period for it to return.
    int proximity = mouseEnteredOnce ? 0 : 60;

    // Note: watched == this ensures no other child objects popping out are
    // receiving this event, meaning there's basically nothing under the
    // cursor.
    if ( /*watched == this &&*/
         !frameGeometry().adjusted( -proximity, -proximity, proximity, proximity ).
         contains( p ) )
    {
      // We've way too far from the window -- hide the popup

      // If the mouse never entered the popup, hide the window instantly --
      // the user just moved the cursor further away from the window.

      if ( !mouseEnteredOnce )
        hideWindow();
      else
        hideTimer.start();
    }
  }
}

void ScanPopup::mousePressEvent( QMouseEvent * ev )
{
  // With mouse grabs, the press can occur anywhere on the screen, which
  // might mean hiding the window.

  if ( !frameGeometry().contains( ev->globalPos() ) )
  {
    hideWindow();

    return;
  }

  if ( ev->button() == Qt::LeftButton )
  {
    startPos = ev->globalPos();
    setCursor( Qt::ClosedHandCursor );
  }

  QMainWindow::mousePressEvent( ev );
}

void ScanPopup::mouseMoveEvent( QMouseEvent * event )
{
  if ( event->buttons() && cursor().shape() == Qt::ClosedHandCursor )
  {
    QPoint newPos = event->globalPos();

    QPoint delta = newPos - startPos;

    startPos = newPos;

    // Move the window

    move( pos() + delta );
  }
 
  QMainWindow::mouseMoveEvent( event );
}

void ScanPopup::mouseReleaseEvent( QMouseEvent * ev )
{
  unsetCursor();
  QMainWindow::mouseReleaseEvent( ev );
}

void ScanPopup::leaveEvent( QEvent * event )
{
  QMainWindow::leaveEvent( event );

  // We hide the popup when the mouse leaves it.

  // Combo-boxes seem to generate leave events for their parents when
  // unfolded, so we check coordinates as well.
  // If the dialog is pinned, we don't hide the popup.
  // If some mouse buttons are pressed, we don't hide the popup either,
  // since it indicates the move operation is underway.
  if ( !ui.pinButton->isChecked() && !geometry().contains( QCursor::pos() ) &&
       QApplication::mouseButtons() == Qt::NoButton )
  {
    hideTimer.start();
  }
}

void ScanPopup::enterEvent( QEvent * event )
{
  QMainWindow::enterEvent( event );

  if ( mouseEnteredOnce )
  {
    // We "enter" first time via our event filter. This seems to evade some
    // unexpected behavior under Windows.

    // If there was a countdown to hide the window, stop it.
    hideTimer.stop();
  }
}

void ScanPopup::requestWindowFocus()
{
  // One of the rare, actually working workarounds for requesting a user keyboard focus on X11,
  // works for Qt::Popup windows, exactly like our Scan Popup (in unpinned state).
  // Modern window managers actively resist to automatically focus pop-up windows.
#ifdef Q_WS_X11
  if ( !ui.pinButton->isChecked() )
  {
    QMenu m( this );
    m.addAction( "" );
    m.show();
    m.hide();
  }
#endif
}

void ScanPopup::showEvent( QShowEvent * ev )
{
  QMainWindow::showEvent( ev );

  QTimer::singleShot(100, this, SLOT( requestWindowFocus() ) );
  
  if ( groups.size() <= 1 ) // Only the default group? Hide then.
    ui.groupList->hide();

  if ( ui.showDictionaryBar->isChecked() != dictionaryBar.isVisible() )
  {
    ui.showDictionaryBar->setChecked( dictionaryBar.isVisible() );
    updateDictionaryBar();
  }
}

void ScanPopup::prefixMatchFinished()
{
  // Check that there's a window there at all
  if ( isVisible() )
  {
    if ( wordFinder.getErrorString().size() )
    {
      ui.queryError->setToolTip( wordFinder.getErrorString() );
      ui.queryError->show();
    }
    else
      ui.queryError->hide();
  }
}

void ScanPopup::on_pronounceButton_clicked()
{
  definition->playSound();
}

void ScanPopup::pinButtonClicked( bool checked )
{
  if ( checked )
  {
    uninterceptMouse();

    setWindowFlags( Qt::Dialog );
    setWindowTitle( tr( "%1 - %2" ).arg( elideInputWord(), "GoldenDict" ) );
    dictionaryBar.setMovable( true );
    hideTimer.stop();
  }
  else
  {
    dictionaryBar.setMovable( false );
    setWindowFlags( popupWindowFlags );

    mouseEnteredOnce = true;
  }

  show();
}

void ScanPopup::focusTranslateLine()
{
  if ( !isActiveWindow() )
    activateWindow();

  ui.translateBox->translateLine()->setFocus();
  ui.translateBox->translateLine()->selectAll();
}

void ScanPopup::on_showDictionaryBar_clicked( bool checked )
{
  dictionaryBar.setVisible( checked );
  updateDictionaryBar();
  definition->updateMutedContents();
}

void ScanPopup::hideTimerExpired()
{
  if ( isVisible() )
    hideWindow();
}

void ScanPopup::altModeExpired()
{
  // The alt mode duration has expired, so there's no need to poll for modifiers
  // anymore.
  altModePollingTimer.stop();
}

void ScanPopup::altModePoll()
{
  if ( !pendingInputWord.size() )
  {
    altModePollingTimer.stop();
    altModeExpirationTimer.stop();
  }
  else
  if ( checkModifiersPressed( cfg.preferences.scanPopupModifiers ) )
  {
    altModePollingTimer.stop();
    altModeExpirationTimer.stop();

    inputWord = pendingInputWord;
    engagePopup( false );
  }
}

void ScanPopup::pageLoaded( ArticleView * )
{
  ui.pronounceButton->setVisible( definition->hasSound() );

  updateBackForwardButtons();

  if ( cfg.preferences.pronounceOnLoadPopup )
    definition->playSound();
}

void ScanPopup::showStatusBarMessage( QString const & message, int timeout, QPixmap const & icon )
{
  mainStatusBar->showMessage( message, timeout, icon );
}

void ScanPopup::escapePressed()
{
  if ( !definition->closeSearch() )
    hideWindow();
}

void ScanPopup::hideWindow()
{
  uninterceptMouse();

  emit closeMenu();
  hideTimer.stop();
  unsetCursor();
  ui.translateBox->setPopupEnabled( false );
  ui.translateBox->translateLine()->deselect();
  hide();
}

void ScanPopup::interceptMouse()
{
  if ( !mouseIntercepted )
  {
    // We used to grab the mouse -- but this doesn't always work reliably
    // (e.g. doesn't work at all in Windows 7 for some reason). Therefore
    // we use a polling timer now.

//    grabMouse();
    mouseGrabPollTimer.start();

    qApp->installEventFilter( this );

    mouseIntercepted = true;
  }
}

void ScanPopup::mouseGrabPoll()
{
  if ( mouseIntercepted )
    reactOnMouseMove( QCursor::pos() );
}

void ScanPopup::uninterceptMouse()
{
  if ( mouseIntercepted )
  {
    qApp->removeEventFilter( this );
    mouseGrabPollTimer.stop();
//    releaseMouse();

    mouseIntercepted = false;
  }
}

void ScanPopup::updateDictionaryBar()
{
  if ( !dictionaryBar.toggleViewAction()->isChecked() )
    return; // It's not enabled, therefore hidden -- don't waste time

  unsigned currentId = ui.groupList->getCurrentGroup();
  Instances::Group const * grp = groups.findGroup( currentId );

  if ( grp ) // Should always be !0, but check as a safeguard
    dictionaryBar.setDictionaries( grp->dictionaries );

  if( currentId == Instances::Group::AllGroupId )
    dictionaryBar.setMutedDictionaries( &cfg.popupMutedDictionaries );
  else
  {
    Config::Group * grp = cfg.getGroup( currentId );
    dictionaryBar.setMutedDictionaries( grp ? &grp->popupMutedDictionaries : 0 );
  }

  setDictionaryIconSize();
}

void ScanPopup::mutedDictionariesChanged()
{
  if ( dictionaryBar.toggleViewAction()->isChecked() )
    definition->updateMutedContents();
}

void ScanPopup::on_sendWordButton_clicked()
{
  if ( !isVisible() )
    return;
  if( !ui.pinButton->isChecked() )
  {
    definition->closeSearch();
    hideWindow();
  }
  emit sendWordToMainWindow( definition->getTitle() );
}

void ScanPopup::switchExpandOptionalPartsMode()
{
  if( isVisible() )
    emit switchExpandMode();
}

void ScanPopup::updateBackForwardButtons()
{
  ui.goBackButton->setEnabled(definition->canGoBack());
  ui.goForwardButton->setEnabled(definition->canGoForward());
}

void ScanPopup::on_goBackButton_clicked()
{
  definition->back();
}

void ScanPopup::on_goForwardButton_clicked()
{
  definition->forward();
}

void ScanPopup::setDictionaryIconSize()
{
  int extent = cfg.usingSmallIconsInToolbars ?
               QApplication::style()->pixelMetric( QStyle::PM_SmallIconSize ) :
               21;
  dictionaryBar.setDictionaryIconSize( extent );
}
