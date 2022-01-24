/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "scanpopup.hh"
#include "folding.hh"
#include <QCursor>
#include <QPixmap>
#include <QBitmap>
#include <QMenu>
#include <QMouseEvent>
#include <QDesktopWidget>
#include "gddebug.hh"
#include "gestures.hh"

#ifdef Q_OS_MAC
#include "macmouseover.hh"
#define MouseOver MacMouseOver
#else
#include "mouseover.hh"
#endif

using std::wstring;

/// We use different window flags under Windows and X11 due to slight differences
/// in their behavior on those platforms.
static const Qt::WindowFlags defaultUnpinnedWindowFlags =

#if defined (Q_OS_WIN)
Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
#else
Qt::Popup
#endif
;

static const Qt::WindowFlags pinnedWindowFlags =
#ifdef HAVE_X11
/// With the Qt::Dialog flag, scan popup is always on top of the main window
/// on Linux/X11 with Qt 4, Qt 5 since version 5.12.1 (QTBUG-74309).
/// Qt::Window allows to use the scan popup and the main window independently.
Qt::Window
#else
Qt::Dialog
#endif
;

#ifdef HAVE_X11
static bool ownsClipboardMode( QClipboard::Mode mode )
{
  const QClipboard & clipboard = *QApplication::clipboard();
  switch( mode )
  {
    case QClipboard::Clipboard:
      return clipboard.ownsClipboard();
    case QClipboard::Selection:
      return clipboard.ownsSelection();
    case QClipboard::FindBuffer:
      return clipboard.ownsFindBuffer();
  }

  gdWarning( "Unknown clipboard mode: %d\n", static_cast< int >( mode ) );
  return false;
}
#endif

ScanPopup::ScanPopup( QWidget * parent,
                      Config::Class & cfg_,
                      ArticleNetworkAccessManager & articleNetMgr,
                      AudioPlayerPtr const & audioPlayer_,
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
  openSearchAction( this ),
  wordFinder( this ),
  dictionaryBar( this, configEvents, cfg.editDictionaryCommandLine, cfg.preferences.maxDictionaryRefsInContextMenu ),
  mouseEnteredOnce( false ),
  mouseIntercepted( false ),
  hideTimer( this ),
  starIcon( ":/icons/star.svg" ),
  blueStarIcon( ":/icons/star_blue.svg" )
{
  ui.setupUi( this );

  openSearchAction.setShortcut( QKeySequence( "Ctrl+F" ) );

  if( layoutDirection() == Qt::RightToLeft )
  {
    // Adjust button icons for Right-To-Left layout
    ui.goBackButton->setIcon( QIcon( ":/icons/next.svg" ) );
    ui.goForwardButton->setIcon( QIcon( ":/icons/previous.svg" ) );
  }

  mainStatusBar = new MainStatusBar( this );

  ui.queryError->hide();

  definition = new ArticleView( ui.outerFrame, articleNetMgr, audioPlayer_,
                                allDictionaries, groups, true, cfg,
                                openSearchAction,
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

  wordListDefaultFont = ui.translateBox->wordList()->font();
  translateLineDefaultFont = ui.translateBox->font();
  groupListDefaultFont = ui.groupList->font();

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

  ui.onTopButton->setChecked( cfg.popupWindowAlwaysOnTop );
  ui.onTopButton->setVisible( cfg.pinPopupWindow );
  connect( ui.onTopButton, SIGNAL( clicked( bool ) ), this, SLOT( alwaysOnTopClicked( bool ) ) );

  ui.pinButton->setChecked( cfg.pinPopupWindow );

  if ( cfg.pinPopupWindow )
  {
    dictionaryBar.setMovable( true );
    Qt::WindowFlags flags = pinnedWindowFlags;
    if( cfg.popupWindowAlwaysOnTop )
      flags |= Qt::WindowStaysOnTopHint;
    setWindowFlags( flags );
  }
  else
  {
    dictionaryBar.setMovable( false );
    setWindowFlags( unpinnedWindowFlags() );
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

  QAction * const focusArticleViewAction = new QAction( this );
  focusArticleViewAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  focusArticleViewAction->setShortcut( QKeySequence( "Ctrl+N" ) );
  addAction( focusArticleViewAction );
  connect( focusArticleViewAction, SIGNAL( triggered() ), definition, SLOT( focus() ) );

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

  connect( definition, SIGNAL( titleChanged(  ArticleView *, QString const & ) ),
           this, SLOT( titleChanged(  ArticleView *, QString const & ) ) );

#ifdef HAVE_X11
  connect( QApplication::clipboard(), SIGNAL( changed( QClipboard::Mode ) ),
           this, SLOT( clipboardChanged( QClipboard::Mode ) ) );
#else
  if( cfg.preferences.trackClipboardChanges )
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

  grabGesture( Gestures::GDPinchGestureType );
  grabGesture( Gestures::GDSwipeGestureType );

#ifdef HAVE_X11
  scanFlag = new ScanFlag( this );

  connect( this, SIGNAL( showScanFlag( bool ) ),
           scanFlag, SLOT( showScanFlag() ) );

  connect( this, SIGNAL( hideScanFlag() ),
           scanFlag, SLOT( hideWindow() ) );

  connect( scanFlag, SIGNAL( showScanPopup() ),
           this, SLOT( showEngagePopup() ) );

  delayTimer.setSingleShot( true );
  delayTimer.setInterval( 200 );

  connect( &delayTimer, SIGNAL( timeout() ),
    this, SLOT( delayShow() ) );
#endif

  applyZoomFactor();
  applyWordsZoomLevel();
}

ScanPopup::~ScanPopup()
{
  saveConfigData();

  disableScanning();

  ungrabGesture( Gestures::GDPinchGestureType );
  ungrabGesture( Gestures::GDSwipeGestureType );
}

void ScanPopup::saveConfigData()
{
  // Save state, geometry and pin status
  cfg.popupWindowState = saveState( 1 );
  cfg.popupWindowGeometry = saveGeometry();
  cfg.pinPopupWindow = ui.pinButton->isChecked();
  cfg.popupWindowAlwaysOnTop = ui.onTopButton->isChecked();
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

void ScanPopup::applyWordsZoomLevel()
{
  QFont font( wordListDefaultFont );
  int ps = font.pointSize();

  if ( cfg.preferences.wordsZoomLevel != 0 )
  {
    ps += cfg.preferences.wordsZoomLevel;
    if ( ps < 1 )
      ps = 1;
    font.setPointSize( ps );
  }

  if ( ui.translateBox->wordList()->font().pointSize() != ps )
    ui.translateBox->wordList()->setFont( font );

  font = translateLineDefaultFont;
  ps = font.pointSize();

  if ( cfg.preferences.wordsZoomLevel != 0 )
  {
    ps += cfg.preferences.wordsZoomLevel;
    if ( ps < 1 )
      ps = 1;
    font.setPointSize( ps );
  }

  if ( ui.translateBox->translateLine()->font().pointSize() != ps )
    ui.translateBox->translateLine()->setFont( font );

  font = groupListDefaultFont;
  ps = font.pointSize();

  if ( cfg.preferences.wordsZoomLevel != 0 )
  {
    ps += cfg.preferences.wordsZoomLevel;
    if ( ps < 1 )
      ps = 1;
    font.setPointSize( ps );
  }

  if ( ui.groupList->font().pointSize() != ps )
  {
    disconnect( ui.groupList, SIGNAL( currentIndexChanged( QString const & ) ),
                this, SLOT( currentGroupChanged( QString const & ) ) );
    int n = ui.groupList->currentIndex();
    ui.groupList->clear();
    ui.groupList->setFont( font );
    ui.groupList->fill( groups );
    ui.groupList->setCurrentIndex( n );
    connect( ui.groupList, SIGNAL( currentIndexChanged( QString const & ) ),
             this, SLOT( currentGroupChanged( QString const & ) ) );
  }

  ui.outerFrame->layout()->activate();
}

Qt::WindowFlags ScanPopup::unpinnedWindowFlags() const
{
#ifdef ENABLE_SPWF_CUSTOMIZATION
  const Config::ScanPopupWindowFlags spwf = cfg.preferences.scanPopupUnpinnedWindowFlags;
  Qt::WindowFlags result;
  if( spwf == Config::SPWF_Popup )
    result = Qt::Popup;
  else
  if( spwf == Config::SPWF_Tool )
    result = Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
  else
    return defaultUnpinnedWindowFlags; // Ignore BypassWMHint option.

  if( cfg.preferences.scanPopupUnpinnedBypassWMHint )
    result |= Qt::X11BypassWindowManagerHint;
  return result;
#else
  return defaultUnpinnedWindowFlags;
#endif
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
  GD_DPRINTF( "translating from clipboard or selection\n" );

  QString subtype = "plain";

  QString str = QApplication::clipboard()->text( subtype, m);

  translateWord( str );
}

void ScanPopup::translateWord( QString const & word )
{
  pendingInputPhrase = cfg.preferences.sanitizeInputPhrase( word );

  if ( !pendingInputPhrase.isValid() )
    return; // Nothing there

  // In case we had any timers engaged before, cancel them now.
  altModePollingTimer.stop();
  altModeExpirationTimer.stop();

#ifdef HAVE_X11
  emit hideScanFlag();
#endif

  inputPhrase = pendingInputPhrase;
  engagePopup( false,
#ifdef Q_OS_WIN
      true // We only focus popup under Windows when activated via Ctrl+C+C
           // -- on Linux it already has an implicit focus
#else
      false
#endif
      );
}

#ifdef HAVE_X11
void ScanPopup::delayShow()
{
  QString subtype = "plain";
  handleInputWord( QApplication::clipboard()->text( subtype, QClipboard::Selection ) );
}
#endif

void ScanPopup::clipboardChanged( QClipboard::Mode m )
{
  if ( !isScanningEnabled )
    return;
#ifdef HAVE_X11
  if( cfg.preferences.ignoreOwnClipboardChanges && ownsClipboardMode( m ) )
    return;
#endif

  GD_DPRINTF( "clipboard changed\n" );

#ifdef HAVE_X11
  if( m == QClipboard::Selection )
  {
    // Use delay show to prevent multiple popups while selection in progress
    delayTimer.start();
    return;
  }
#endif

  QString subtype = "plain";

  handleInputWord( QApplication::clipboard()->text( subtype, m ) );
}

void ScanPopup::mouseHovered( QString const & str, bool forcePopup )
{
  handleInputWord( str, forcePopup );
}

void ScanPopup::handleInputWord( QString const & str, bool forcePopup )
{
  Config::InputPhrase sanitizedPhrase = cfg.preferences.sanitizeInputPhrase( str );

  if ( isVisible() && sanitizedPhrase == inputPhrase )
  {
    // Attempt to translate the same word we already have shown in scan popup.
    // Ignore it, as it is probably a spurious mouseover event.
    return;
  }

  pendingInputPhrase = sanitizedPhrase;

  if ( !pendingInputPhrase.isValid() )
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

#ifdef HAVE_X11
  if ( cfg.preferences.showScanFlag ) {
    inputPhrase = pendingInputPhrase;
    emit showScanFlag( forcePopup );
    return;
  }
#endif

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

  inputPhrase = pendingInputPhrase;
  engagePopup( forcePopup );
}

#ifdef HAVE_X11
void ScanPopup::showEngagePopup()
{
  engagePopup(false);
}
#endif

void ScanPopup::engagePopup( bool forcePopup, bool giveFocus )
{
  if( cfg.preferences.scanToMainWindow && !forcePopup )
  {
    // Send translated word to main window istead of show popup
    emit sendPhraseToMainWindow( inputPhrase );
    return;
  }

  definition->setSelectionBySingleClick( cfg.preferences.selectWordBySingleClick );

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

#ifdef ENABLE_SPWF_CUSTOMIZATION
    // Ensure that the window always has focus on X11 with Qt::Tool flag.
    // This also often prevents the window from disappearing prematurely with Qt::Popup flag,
    // especially when combined with Qt::X11BypassWindowManagerHint flag.
    if ( !ui.pinButton->isChecked() )
      giveFocus = true;
#endif

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
#ifdef ENABLE_SPWF_CUSTOMIZATION
  else
  if ( ( windowFlags() & Qt::Tool ) == Qt::Tool )
  {
    // Ensure that the window with Qt::Tool flag always has focus on X11.
    activateWindow();
    raise();
  }
#endif

  if ( ui.pinButton->isChecked() )
       setWindowTitle( tr( "%1 - %2" ).arg( elideInputWord(), "GoldenDict" ) );

  /// Too large strings make window expand which is probably not what user
  /// wants
  ui.translateBox->setText( Folding::escapeWildcardSymbols( inputPhrase.phrase ), false );
  translateBoxSuffix = inputPhrase.punctuationSuffix;

  showTranslationFor( inputPhrase );
}

QString ScanPopup::elideInputWord()
{
  QString const & inputWord = inputPhrase.phrase;
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
    updateSuggestionList();
    translateInputFinished();
  }

  cfg.lastPopupGroupId = ui.groupList->getCurrentGroup();
}

void ScanPopup::wordListItemActivated( QListWidgetItem * item )
{
  showTranslationFor( Config::InputPhrase::fromPhrase( item->text() ) );
}

void ScanPopup::translateInputChanged( QString const & text )
{
  updateSuggestionList( text );
  translateBoxSuffix = QString();
}

void ScanPopup::updateSuggestionList()
{
  updateSuggestionList( ui.translateBox->translateLine()->text() );
}

void ScanPopup::updateSuggestionList( QString const & text )
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
  inputPhrase.phrase = Folding::unescapeWildcardSymbols( ui.translateBox->translateLine()->text().trimmed() );
  inputPhrase.punctuationSuffix = translateBoxSuffix;
  showTranslationFor( inputPhrase );
}

void ScanPopup::showTranslationFor( Config::InputPhrase const & inputPhrase )
{
  ui.pronounceButton->hide();

  unsigned groupId = ui.groupList->getCurrentGroup();
  definition->showDefinition( inputPhrase, groupId );
  definition->focus();
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
      ui.translateBox->translateLine()->clear();
      ui.translateBox->translateLine()->setFocus();
      //    ui.translateBox->setText( t, true );
      //    ui.translateBox->translateLine()->setCursorPosition( t.size() );
  }
}

bool ScanPopup::eventFilter( QObject * watched, QEvent * event )
{
  if ( watched == ui.translateBox->translateLine() )
  {
    if ( event->type() == QEvent::FocusIn )
    {
      QFocusEvent * focusEvent = static_cast< QFocusEvent * >( event );

      // select all on mouse click
      if ( focusEvent->reason() == Qt::MouseFocusReason ) {
        QTimer::singleShot(0, this, SLOT(focusTranslateLine()));
      }
      return false;
    }

    if ( event->type() == QEvent::Resize )
    {
      // The UI looks ugly when group combobox is higher than translate line.
      // Make the height of the combobox the same as the line edit's height.
      // The fonts of these UI items should be kept in sync by applyWordsZoomLevel()
      // so that text in the combobox is not clipped.
      const QResizeEvent * const resizeEvent = static_cast< const QResizeEvent * >( event );
      ui.groupList->setFixedHeight( resizeEvent->size().height() );
      return false;
    }
  }

  if ( mouseIntercepted )
  {
    // We're only interested in our events

    if ( event->type() == QEvent::MouseMove )
    {
//    GD_DPRINTF( "Object: %s\n", watched->objectName().toUtf8().data() );
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
//        GD_DPRINTF( "got inside\n" );

    hideTimer.stop();
    mouseEnteredOnce = true;
    uninterceptMouse();
  }
  else
  {
//        GD_DPRINTF( "outside\n" );
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

    ui.onTopButton->setVisible( true );
    Qt::WindowFlags flags = pinnedWindowFlags;
    if( ui.onTopButton->isChecked() )
      flags |= Qt::WindowStaysOnTopHint;
    setWindowFlags( flags );

    setWindowTitle( tr( "%1 - %2" ).arg( elideInputWord(), "GoldenDict" ) );
    dictionaryBar.setMovable( true );
    hideTimer.stop();
  }
  else
  {
    ui.onTopButton->setVisible( false );
    dictionaryBar.setMovable( false );
    setWindowFlags( unpinnedWindowFlags() );

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
  if ( !pendingInputPhrase.isValid() )
  {
    altModePollingTimer.stop();
    altModeExpirationTimer.stop();
  }
  else
  if ( checkModifiersPressed( cfg.preferences.scanPopupModifiers ) )
  {
    altModePollingTimer.stop();
    altModeExpirationTimer.stop();

    inputPhrase = pendingInputPhrase;
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
  updateSuggestionList();
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
  emit sendPhraseToMainWindow( definition->getPhrase() );
}

void ScanPopup::on_sendWordToFavoritesButton_clicked()
{
  if ( !isVisible() )
    return;
  emit sendWordToFavorites( definition->getTitle(), cfg.lastPopupGroupId );

  ui.sendWordToFavoritesButton->setIcon( blueStarIcon );
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
               QApplication::style()->pixelMetric(QStyle::PM_ToolBarIconSize);
  dictionaryBar.setDictionaryIconSize( extent );
}

void ScanPopup::setGroupByName( QString const & name )
{
  int i;
  for( i = 0; i < ui.groupList->count(); i++ )
  {
    if( ui.groupList->itemText( i ) == name )
    {
      ui.groupList->setCurrentIndex( i );
      break;
    }
  }
  if( i >= ui.groupList->count() )
    gdWarning( "Group \"%s\" for popup window is not found\n", name.toUtf8().data() );
}

void ScanPopup::alwaysOnTopClicked( bool checked )
{
  bool wasVisible = isVisible();
  if( ui.pinButton->isChecked() )
  {
    Qt::WindowFlags flags = this->windowFlags();
    if( checked )
      setWindowFlags(flags | Qt::WindowStaysOnTopHint );
    else
      setWindowFlags(flags ^ Qt::WindowStaysOnTopHint );
    if( wasVisible )
      show();
  }
}

void ScanPopup::titleChanged( ArticleView *, QString const & title )
{
  unsigned groupId = ui.groupList->getCurrentGroup();

  // Set icon for "Add to Favorites" button
  ui.sendWordToFavoritesButton->setIcon( isWordPresentedInFavorites( title, groupId ) ?
                                         blueStarIcon : starIcon );
}
