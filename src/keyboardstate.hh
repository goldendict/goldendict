/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __KEYBOARDSTATE_HH_INCLUDED__
#define __KEYBOARDSTATE_HH_INCLUDED__

/// Since Qt doesn't provide a way to test for keyboard modifiers state
/// when the app isn't in focus, we have to implement this separately for
/// each platform.
class KeyboardState
{
public:

  enum
  {
    Alt = 1,
    Ctrl = 2,
    Shift = 4,
    Win = 8
  } Modifier;

  /// Returns true if all Modifiers present within the given mask are pressed
  /// right now.
  bool checkModifiersPressed( int mask );
};

#endif
