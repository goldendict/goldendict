#ifdef __WIN32 // Q_OS_WIN32 isn't available at this point
#define _WIN32_WINNT 0x0430
#include <windows.h>
#endif

#include "hotkeywrapper.hh"
#include "dprintf.hh"

#ifdef Q_WS_X11
#include <X11/Xlibint.h>
#endif

//////////////////////////////////////////////////////////////////////////

QHotkeyApplication::QHotkeyApplication( int & argc, char ** argv ):
  QtSingleApplication( argc, argv )
{
}

QHotkeyApplication::QHotkeyApplication( QString const & id,
                                        int & argc, char ** argv ):
  QtSingleApplication( id, argc, argv )
{
}

void QHotkeyApplication::addDataCommiter( DataCommitter & d )
{
  dataCommitters.append( &d );
}

void QHotkeyApplication::removeDataCommiter( DataCommitter & d )
{
  dataCommitters.removeAll( &d );
}

void QHotkeyApplication::commitData( QSessionManager & s )
{
  for( int x = 0; x < dataCommitters.size(); ++x )
    dataCommitters[ x ]->commitData( s );
}

void QHotkeyApplication::registerWrapper(HotkeyWrapper *wrapper)
{
  if (wrapper && !hotkeyWrappers.contains(wrapper))
    hotkeyWrappers.append(wrapper);
}

void QHotkeyApplication::unregisterWrapper(HotkeyWrapper *wrapper)
{
  if (wrapper && hotkeyWrappers.contains(wrapper))
    hotkeyWrappers.removeAll(wrapper);
}

//////////////////////////////////////////////////////////////////////////

HotkeyStruct::HotkeyStruct( quint32 key_, quint32 key2_, quint32 modifier_,
                            int handle_, int id_ ):
  key( key_ ),
  key2( key2_ ),
  modifier( modifier_ ),
  handle( handle_ ),
  id( id_ )
{
}

//////////////////////////////////////////////////////////////////////////

#ifndef Q_WS_MAC
HotkeyWrapper::HotkeyWrapper(QObject *parent) : QThread( parent ),
    state2(false)
{
#ifdef Q_OS_WIN
  hwnd=(HWND)((static_cast<QMainWindow*>(parent))->winId());
#else
  init();
#endif
  (static_cast<QHotkeyApplication*>(qApp))->registerWrapper(this);
}

HotkeyWrapper::~HotkeyWrapper()
{
  unregister();
}

void HotkeyWrapper::waitKey2()
{
  state2 = false;

#ifdef Q_WS_X11

  if ( keyToUngrab != grabbedKeys.end() )
  {
    ungrabKey( keyToUngrab );
    keyToUngrab = grabbedKeys.end();
  }

#endif
}

bool HotkeyWrapper::checkState(quint32 vk, quint32 mod)
{
  if ( state2 )
  { // wait for 2nd key

    waitKey2(); // Cancel the 2nd-key wait stage

    if (state2waiter.key2 == vk && state2waiter.modifier == mod)
    {
      emit hotkeyActivated( state2waiter.handle );
      return true;
    }
  }

  for (int i = 0; i < hotkeys.count(); i++) {

    const HotkeyStruct &hs = hotkeys.at(i);

    if (hs.key == vk && hs.modifier == mod) {

      #ifdef Q_WS_WIN32
      // If that was a copy-to-clipboard shortcut, re-emit it back so it could
      // reach its original destination so it could be acted upon.
      if ( ( vk == VK_INSERT || vk == 'c' || vk == 'C' ) && mod == MOD_CONTROL )
      {
        INPUT i[ 2 ];
  
        memset( i, 0, sizeof( i ) );
  
        i[ 0 ].type = INPUT_KEYBOARD;
        i[ 0 ].ki.wVk = 'C';
        i[ 1 ].type = INPUT_KEYBOARD;
        i[ 1 ].ki.wVk = 'C';
        i[ 1 ].ki.dwFlags = KEYEVENTF_KEYUP;
  
        UnregisterHotKey( hwnd, hs.id );
        SendInput( 2, i, sizeof( *i ) );
        RegisterHotKey(hwnd, hs.id, hs.modifier, hs.key);
      }
      #endif

      if (hs.key2 == 0) {
         emit hotkeyActivated( hs.handle );
         return true;
      }

      state2 = true;
      state2waiter = hs;
      QTimer::singleShot(500, this, SLOT(waitKey2()));

      #ifdef Q_WS_X11

      // Grab the second key, unless it's grabbed already
      // Note that we only grab the clipboard key only if
      // the sequence didn't begin with it

      if ( ( isCopyToClipboardKey( hs.key, hs.modifier ) ||
             !isCopyToClipboardKey( hs.key2, hs.modifier ) ) &&
           !isKeyGrabbed( hs.key2, hs.modifier ) )
        keyToUngrab = grabKey( hs.key2, hs.modifier );

      #endif

      return true;
    }
  }

  state2 = false;
  return false;
}

//////////////////////////////////////////////////////////////////////////

#ifdef Q_WS_WIN

void HotkeyWrapper::init()
{
  QWidget *root = qApp->topLevelWidgets().value(0);
  hwnd = (HWND)root->winId();
}

bool HotkeyWrapper::setGlobalKey( int key, int key2,
                                  Qt::KeyboardModifiers modifier, int handle )
{
  if ( !key )
    return false; // We don't monitor empty combinations

  static int id = 0;
  if( id > 0xBFFF - 1 )
    id = 0;

  quint32 mod = 0;
  if (modifier & Qt::CTRL)
    mod |= MOD_CONTROL;
  if (modifier & Qt::ALT)
    mod |= MOD_ALT;
  if (modifier & Qt::SHIFT)
    mod |= MOD_SHIFT;
  if (modifier & Qt::META)
    mod |= MOD_WIN;

  quint32 vk = nativeKey( key );
  quint32 vk2 = key2 ? nativeKey( key2 ) : 0;

  hotkeys.append( HotkeyStruct( vk, vk2, mod, handle, id ) );

  if (!RegisterHotKey(hwnd, id++, mod, vk))
    return false;

  if ( key2 && key2 != key )
    return RegisterHotKey(hwnd, id++, mod, vk2);

  return true;
}

bool HotkeyWrapper::winEvent ( MSG * message, long * result )
{
  if (message->message == WM_HOTKEY)
    return checkState( (message->lParam >> 16), (message->lParam & 0xffff) );

  return false;
}

quint32 HotkeyWrapper::nativeKey(int key)
{
  if (key >= Qt::Key_0 && key <= Qt::Key_9)
    return key;

  if (key >= Qt::Key_A && key <= Qt::Key_Z)
    return key;

  switch (key) {
    case Qt::Key_Space:     return VK_SPACE;
    case Qt::Key_Asterisk:  return VK_MULTIPLY;
    case Qt::Key_Plus:      return VK_ADD;
    case Qt::Key_Comma:     return VK_SEPARATOR;
    case Qt::Key_Minus:     return VK_SUBTRACT;
    case Qt::Key_Slash:     return VK_DIVIDE;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:   return VK_TAB;
    case Qt::Key_Backspace: return VK_BACK;
    case Qt::Key_Return:
    case Qt::Key_Escape:    return VK_ESCAPE;
    case Qt::Key_Enter:     return VK_RETURN;
    case Qt::Key_Insert:    return VK_INSERT;
    case Qt::Key_Delete:    return VK_DELETE;
    case Qt::Key_Pause:     return VK_PAUSE;
    case Qt::Key_Print:     return VK_PRINT;
    case Qt::Key_Clear:     return VK_CLEAR;
    case Qt::Key_Home:      return VK_HOME;
    case Qt::Key_End:       return VK_END;
    case Qt::Key_Up:        return VK_UP;
    case Qt::Key_Down:      return VK_DOWN;
    case Qt::Key_Left:      return VK_LEFT;
    case Qt::Key_Right:     return VK_RIGHT;
    case Qt::Key_PageUp:    return VK_PRIOR;
    case Qt::Key_PageDown:  return VK_NEXT;
    case Qt::Key_F1:        return VK_F1;
    case Qt::Key_F2:        return VK_F2;
    case Qt::Key_F3:        return VK_F3;
    case Qt::Key_F4:        return VK_F4;
    case Qt::Key_F5:        return VK_F5;
    case Qt::Key_F6:        return VK_F6;
    case Qt::Key_F7:        return VK_F7;
    case Qt::Key_F8:        return VK_F8;
    case Qt::Key_F9:        return VK_F9;
    case Qt::Key_F10:       return VK_F10;
    case Qt::Key_F11:       return VK_F11;
    case Qt::Key_F12:       return VK_F12;
    case Qt::Key_F13:       return VK_F13;
    case Qt::Key_F14:       return VK_F14;
    case Qt::Key_F15:       return VK_F15;
    case Qt::Key_F16:       return VK_F16;
    case Qt::Key_F17:       return VK_F17;
    case Qt::Key_F18:       return VK_F18;
    case Qt::Key_F19:       return VK_F19;
    case Qt::Key_F20:       return VK_F20;
    case Qt::Key_F21:       return VK_F21;
    case Qt::Key_F22:       return VK_F22;
    case Qt::Key_F23:       return VK_F23;
    case Qt::Key_F24:       return VK_F24;
    default:;
  }

  return key;
}

void HotkeyWrapper::unregister()
{
  for (int i = 0; i < hotkeys.count(); i++)
  {
    HotkeyStruct const & hk = hotkeys.at( i );

    UnregisterHotKey( hwnd, hk.id );

    if ( hk.key2 && hk.key2 != hk.key )
      UnregisterHotKey( hwnd, hk.id+1 );
  }

  (static_cast<QHotkeyApplication*>(qApp))->unregisterWrapper(this);
}



bool QHotkeyApplication::winEventFilter ( MSG * message, long * result )
{
  if (message->message == WM_HOTKEY)
  {
    for (int i = 0; i < hotkeyWrappers.size(); i++)
    {
      if ( hotkeyWrappers.at(i)->winEvent( message, result ) )
        return true;
    }
  }

  return QApplication::winEventFilter( message, result );
}

//////////////////////////////////////////////////////////////////////////

#else

//////////////////////////////////////////////////////////////////////////

#include <X11/keysym.h>

void HotkeyWrapper::init()
{
  keyToUngrab = grabbedKeys.end();

  // We use RECORD extension instead of XGrabKey. That's because XGrabKey
  // prevents other clients from getting their input if it's grabbed.

  Display * display = QX11Info::display();

  lShiftCode = XKeysymToKeycode( display, XK_Shift_L );
  rShiftCode = XKeysymToKeycode( display, XK_Shift_R );

  lCtrlCode = XKeysymToKeycode( display, XK_Control_L );
  rCtrlCode = XKeysymToKeycode( display, XK_Control_R );

  lAltCode = XKeysymToKeycode( display, XK_Alt_L );
  rAltCode = XKeysymToKeycode( display, XK_Alt_R );

  cCode = XKeysymToKeycode( display, XK_c );
  insertCode = XKeysymToKeycode( display, XK_Insert );
  kpInsertCode = XKeysymToKeycode( display, XK_KP_Insert );

  currentModifiers = 0;

  // This one will be used to read the recorded content
  dataDisplay = XOpenDisplay( 0 );

  if ( !dataDisplay )
    throw exInit();

  recordRange = XRecordAllocRange();

  if ( !recordRange )
  {
    XCloseDisplay( dataDisplay );
    throw exInit();
  }

  recordRange->device_events.first = KeyPress;
  recordRange->device_events.last = KeyRelease;
  recordClientSpec = XRecordAllClients;

  recordContext = XRecordCreateContext( display, 0,
                                        &recordClientSpec, 1,
                                        &recordRange, 1 );

  if ( !recordContext )
  {
    XFree( recordRange );
    XCloseDisplay( dataDisplay );
    throw exInit();
  }

  // This is required to ensure context was indeed created
  XSync( display, False );

  connect( this, SIGNAL( keyRecorded( quint32, quint32 ) ),
           this, SLOT( checkState( quint32, quint32 ) ),
           Qt::QueuedConnection );

  start();
}

void HotkeyWrapper::run() // Runs in a separate thread
{
  if ( !XRecordEnableContext( dataDisplay, recordContext,
                              recordEventCallback,
                              (XPointer) this ) )
    DPRINTF( "Failed to enable record context\n" );
}


void HotkeyWrapper::recordEventCallback( XPointer ptr, XRecordInterceptData * data )
{
  ((HotkeyWrapper * )ptr)->handleRecordEvent( data );
}

void HotkeyWrapper::handleRecordEvent( XRecordInterceptData * data )
{
  if ( data->category == XRecordFromServer )
  {
    xEvent * event = ( xEvent * ) data->data;

    if ( event->u.u.type == KeyPress )
    {
      KeyCode key = event->u.u.detail;

      if ( key == lShiftCode ||
           key == rShiftCode )
        currentModifiers |= ShiftMask;
      else
      if ( key == lCtrlCode ||
           key == rCtrlCode )
        currentModifiers |= ControlMask;
      else
      if ( key == lAltCode ||
           key == rAltCode )
        currentModifiers |= Mod1Mask;
      else
      {
        // Here we employ a kind of hack translating KP_Insert key
        // to just Insert. This allows reacting to both Insert keys.
        if ( key == kpInsertCode )
          key = insertCode;

        emit keyRecorded( key, currentModifiers );
      }
    }
    else
    if ( event->u.u.type == KeyRelease )
    {
      KeyCode key = event->u.u.detail;

      if ( key == lShiftCode ||
           key == rShiftCode )
        currentModifiers &= ~ShiftMask;
      else
      if ( key == lCtrlCode ||
           key == rCtrlCode )
        currentModifiers &= ~ControlMask;
      else
      if ( key == lAltCode ||
           key == rAltCode )
        currentModifiers &= ~Mod1Mask;
    }
  }

  XRecordFreeData( data );
}

bool HotkeyWrapper::setGlobalKey( int key, int key2,
                                  Qt::KeyboardModifiers modifier, int handle )
{
  if ( !key )
    return false; // We don't monitor empty combinations

  int vk = nativeKey( key );
  int vk2 = key2 ? nativeKey( key2 ) : 0;

  quint32 mod = 0;
  if (modifier & Qt::ShiftModifier)
      mod |= ShiftMask;
  if (modifier & Qt::ControlModifier)
      mod |= ControlMask;
  if (modifier & Qt::AltModifier)
      mod |= Mod1Mask;

  hotkeys.append( HotkeyStruct( vk, vk2, mod, handle, 0 ) );

  if ( !isCopyToClipboardKey( vk, mod ) )
    grabKey( vk, mod ); // Make sure it doesn't get caught by other apps

  return true;
}

bool HotkeyWrapper::isCopyToClipboardKey( quint32 keyCode, quint32 modifiers ) const
{
  return modifiers == ControlMask &&
         ( keyCode == cCode || keyCode == insertCode || keyCode == kpInsertCode );
}

bool HotkeyWrapper::isKeyGrabbed( quint32 keyCode, quint32 modifiers ) const
{
  GrabbedKeys::const_iterator i = grabbedKeys.find( std::make_pair( keyCode, modifiers ) );

  return i != grabbedKeys.end();
}

HotkeyWrapper::GrabbedKeys::iterator HotkeyWrapper::grabKey( quint32 keyCode,
                                                             quint32 modifiers )
{
  std::pair< GrabbedKeys::iterator, bool > result =
    grabbedKeys.insert( std::make_pair( keyCode, modifiers ) );

  if ( result.second )
  {
    XGrabKey( QX11Info::display(), keyCode, modifiers, QX11Info::appRootWindow(),
              True, GrabModeAsync, GrabModeAsync );
  }

  return result.first;
}

void HotkeyWrapper::ungrabKey( GrabbedKeys::iterator i )
{
  XUngrabKey( QX11Info::display(), i->first, i->second, QX11Info::appRootWindow() );

  grabbedKeys.erase( i );
}

quint32 HotkeyWrapper::nativeKey(int key)
{
  QString keySymName;

  switch( key )
  {
    case Qt::Key_Insert:
      keySymName = "Insert";
    break;
    default:
      keySymName = QKeySequence( key ).toString();
    break;
  }

  Display * display = QX11Info::display();
  return XKeysymToKeycode( display, XStringToKeysym( keySymName.toLatin1().data() ) );
}

void HotkeyWrapper::unregister()
{
  Display * display = QX11Info::display();

  XRecordDisableContext( display, recordContext );
  XSync( display, False );

  wait();

  XRecordFreeContext( display, recordContext );
  XFree( recordRange );
  XCloseDisplay( dataDisplay );

  while( grabbedKeys.size() )
    ungrabKey( grabbedKeys.begin() );

  (static_cast<QHotkeyApplication*>(qApp))->unregisterWrapper(this);
}

#endif
#endif
