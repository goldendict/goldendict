#include "hotkeywrapper.hh"

//////////////////////////////////////////////////////////////////////////

QHotkeyApplication::QHotkeyApplication(int & argc, char ** argv) : QApplication(argc,argv)
{
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

HotkeyStruct::HotkeyStruct(quint32 key, quint32 key2, quint32 modifier, const QObject *member, const char *receiver)
{
  this->key = key;
  this->key2 = key2;
  this->modifier = modifier;
  this->member = member;
  this->receiver = receiver;
}

//////////////////////////////////////////////////////////////////////////

HotkeyWrapper::HotkeyWrapper(QObject *parent) : QObject(parent),
    state2(false)
{
  init();

  (static_cast<QHotkeyApplication*>(qApp))->registerWrapper(this);
}

void HotkeyWrapper::waitKey2()
{
  state2 = false;
}

bool HotkeyWrapper::checkState(quint32 vk, quint32 mod)
{
  if (state2) {	// wait for 2nd key
    state2 = false;
    if (state2waiter.key2 == vk && state2waiter.modifier == mod) {
       connect(this, SIGNAL(hotkeyActivated()), state2waiter.member, state2waiter.receiver);
       emit hotkeyActivated();
       disconnect(state2waiter.member, state2waiter.receiver);
       return true;
    }
  }

  for (int i = 0; i < hotkeys.count(); i++) {
    const HotkeyStruct &hs = hotkeys.at(i);

    if (hs.key == vk && hs.modifier == mod) {
      if (hs.key2 == 0) {
         connect(this, SIGNAL(hotkeyActivated()), hs.member, hs.receiver);
         emit hotkeyActivated();
         disconnect(hs.member, hs.receiver);
         return true;
      }

      state2 = true;
      state2waiter = hs;
      QTimer::singleShot(500, this, SLOT(waitKey2()));
      return true;
    }
  }

  state2 = false;
  return false;
}

//////////////////////////////////////////////////////////////////////////

#ifdef Q_WS_WIN

#include "windows.h"

void HotkeyWrapper::init()
{
  QWidget *root = qApp->topLevelWidgets().value(0);
  hwnd = (HWND)root->winId();
}

bool HotkeyWrapper::setGlobalKey(int key, int key2, Qt::KeyboardModifiers modifier, const QObject *member, const char *receiver)
{
  if (!member || !receiver)
    return false;

  static int id = 0;

  quint32 mod = 0;
  if (modifier & Qt::CTRL)
    mod |= MOD_CONTROL;
  if (modifier & Qt::ALT)
    mod |= MOD_ALT;
  if (modifier & Qt::SHIFT)
    mod |= MOD_SHIFT;

  quint32 vk = nativeKey(key);
  quint32 vk2 = nativeKey(key2);

  hotkeys.append(HotkeyStruct(vk, vk2, mod, member, receiver));

  if (!RegisterHotKey(hwnd, id++, mod, vk))
    return false;

  if (vk2)
    return RegisterHotKey(hwnd, id++, mod, vk2);

  return true;
}

bool HotkeyWrapper::winEvent ( MSG * message, long * result )
{
  if (message->message == WM_HOTKEY)
    return HotkeyWrapper::checkState((message->lParam >> 16), (message->lParam & 0xffff));

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

HotkeyWrapper::~HotkeyWrapper()
{
  for (int i = 0; i < hotkeys.count(); i++)
    UnregisterHotKey(hwnd, hotkeys.at(i).key);

  (static_cast<QHotkeyApplication*>(qApp))->unregisterWrapper(this);
}



bool QHotkeyApplication::winEventFilter ( MSG * message, long * result )
{
  if (message->message == WM_HOTKEY)
  {
    for (int i = 0; i < hotkeyWrappers.size(); i++)
    {
      if (hotkeyWrappers.at(i)->winEvent( message, result ))
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
}

bool HotkeyWrapper::setGlobalKey(int key, int key2, Qt::KeyboardModifiers modifier, const QObject *member, const char *receiver)
{
  if (!member || !receiver)
      return false;

  int vk = nativeKey(key);
  int vk2 = nativeKey(key2);

  quint32 mod = 0;
  if (modifier & Qt::ShiftModifier)
      mod |= ShiftMask;
  if (modifier & Qt::ControlModifier)
      mod |= ControlMask;
  if (modifier & Qt::AltModifier)
      mod |= Mod1Mask;

  hotkeys.append(HotkeyStruct(vk, vk2, mod, member, receiver));

  Display* display = QX11Info::display();
  Window root = QX11Info::appRootWindow();

 //   qDebug() << "Grab " << vk << " " << mod << " " << root;

  XGrabKey(display, vk, mod, root, True, GrabModeAsync, GrabModeAsync);

  if (key2)
      XGrabKey(display, vk2, mod, root, True, GrabModeAsync, GrabModeAsync);

  return true;
}

quint32 HotkeyWrapper::nativeKey(int key)
{
  Display* display = QX11Info::display();
  return XKeysymToKeycode(display, XStringToKeysym(QKeySequence(key).toString().toLatin1().data()));
}

bool HotkeyWrapper::x11Event ( XEvent * event )
{
  if (event->type == KeyPress)
    return HotkeyWrapper::checkState(event->xkey.keycode, event->xkey.state);

  return false;
}

HotkeyWrapper::~HotkeyWrapper()
{
  Display* display = QX11Info::display();
  Window root = QX11Info::appRootWindow();

  for (int i = 0; i < hotkeys.count(); i++)
    XUngrabKey(display, hotkeys.at(i).key, hotkeys.at(i).modifier, root);

  (static_cast<QHotkeyApplication*>(qApp))->unregisterWrapper(this);
}


bool QHotkeyApplication::x11EventFilter ( XEvent * event )
{
  if (event->type == KeyPress)
  {
    for (int i = 0; i < hotkeyWrappers.size(); i++)
    {
      if (hotkeyWrappers.at(i)->x11Event(event))
        return true;
    }
  }

  return QApplication::x11EventFilter(event);
}

#endif

