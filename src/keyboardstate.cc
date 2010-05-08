/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "keyboardstate.hh"
#include <QObject> // To get Qt Q_OS defines

#ifdef Q_OS_WIN32
#include <windows.h>
#else
#include <QX11Info>
#include <X11/X.h>
#include <X11/XKBlib.h>
#endif

bool KeyboardState::checkModifiersPressed( int mask )
{
  #ifdef Q_OS_WIN32

  return !(
    ( mask & Alt && !( GetAsyncKeyState( VK_MENU ) & 0x8000 ) ) ||
    ( mask & Ctrl && !( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) ) ||
    ( mask & Shift && !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) ||
    ( mask & LeftAlt && !( GetAsyncKeyState( VK_LMENU ) & 0x8000 ) ) ||
    ( mask & RightAlt && !( GetAsyncKeyState( VK_RMENU ) & 0x8000 ) ) ||
    ( mask & LeftCtrl && !( GetAsyncKeyState( VK_LCONTROL ) & 0x8000 ) ) ||
    ( mask & RightCtrl && !( GetAsyncKeyState( VK_RCONTROL ) & 0x8000 ) ) ||
    ( mask & LeftShift && !( GetAsyncKeyState( VK_LSHIFT ) & 0x8000 ) ) ||
    ( mask & RightShift && !( GetAsyncKeyState( VK_RSHIFT ) & 0x8000 ) ) );

  #else
  XkbStateRec state;

  XkbGetState( QX11Info::display(), XkbUseCoreKbd, &state );

  return !(
    ( mask & Alt && !( state.base_mods & Mod1Mask ) ) ||
    ( mask & Ctrl && !( state.base_mods & ControlMask ) ) ||
    ( mask & Shift && !( state.base_mods & ShiftMask ) ) ||
    ( mask & Win && !( state.base_mods & Mod4Mask ) ) );
  #endif
}

