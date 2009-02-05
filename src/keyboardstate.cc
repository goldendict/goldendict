/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "keyboardstate.hh"
#include <QX11Info>
#include <X11/X.h>
#include <X11/XKBlib.h>

bool KeyboardState::checkModifiersPressed( int mask )
{
  XkbStateRec state;

  XkbGetState( QX11Info::display(), XkbUseCoreKbd, &state );

  return !(
    ( mask & Alt && !( state.base_mods & Mod1Mask ) ) ||
    ( mask & Ctrl && !( state.base_mods & ControlMask ) ) ||
    ( mask & Shift && !( state.base_mods & ShiftMask ) ) ||
    ( mask & Win && !( state.base_mods & Mod4Mask ) ) );
}

