/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __HOTKEYEDIT_HH_INCLUDED__
#define __HOTKEYEDIT_HH_INCLUDED__

#include "config.hh"
#include <QLineEdit>

// This widget allows grabbing a hotkey
class HotKeyEdit: public QLineEdit
{
  Q_OBJECT

  Qt::KeyboardModifiers currentModifiers;
  int currentKey1, currentKey2;

  bool continuingCombo;

public:

  HotKeyEdit( QWidget * parent = 0 );

  void setHotKey( Config::HotKey const & );
  Config::HotKey getHotKey() const;

protected:

  void keyPressEvent( QKeyEvent * event );
  void keyReleaseEvent( QKeyEvent * event );

private:

  void renderCurrentValue();
  bool eventFilter( QObject *, QEvent * event );
};

#endif
