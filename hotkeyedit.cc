/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "hotkeyedit.hh"
#include <QKeyEvent>

HotKeyEdit::HotKeyEdit( QWidget * parent ):
  QLineEdit( parent ),
  currentModifiers( 0 ), currentKey1( 0 ), currentKey2( 0 ),
  continuingCombo( false )
{
  renderCurrentValue();
  installEventFilter( this );
}

void HotKeyEdit::setHotKey( Config::HotKey const & hk )
{
  currentModifiers = hk.modifiers;
  currentKey1 = hk.key1;
  currentKey2 = hk.key2;

  renderCurrentValue();
}

Config::HotKey HotKeyEdit::getHotKey() const
{
  Config::HotKey hk;

  hk.modifiers = currentModifiers;
  hk.key1 = currentKey1;
  hk.key2 = currentKey2;

  return hk;
}

void HotKeyEdit::renderCurrentValue()
{
  QString result;

  if ( currentKey1 )
  {
    result = QKeySequence( currentKey1 | currentModifiers ).toString( QKeySequence::NativeText );

    if ( currentKey2 )
     result += "+" + QKeySequence( currentKey2 ).toString( QKeySequence::NativeText );
  }

  setText( result );
}

void HotKeyEdit::keyPressEvent( QKeyEvent * event )
{
  int key = event->key();
  Qt::KeyboardModifiers modifiers = event->modifiers() & ~Qt::KeypadModifier;

  switch( key )
  {
    case 0:
    case Qt::Key_unknown:
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
      continuingCombo = false;
      QLineEdit::keyPressEvent( event );
    break;

    default:
    {
      if ( !modifiers &&
           ( ( key == Qt::Key_Backspace ) || ( key == Qt::Key_Delete  ) ) )
      {
        // Delete current combo
        currentKey1 = 0;
        currentKey2 = 0;
        currentModifiers = 0;
        continuingCombo = false;
      }
      else
      if ( !continuingCombo )
      {
        if ( modifiers || event->text().isEmpty() ) // Don't allow plain letters
        {
          currentKey2 = 0;
          currentKey1 = key;
          currentModifiers = modifiers;
          continuingCombo = true;
        }
      }
      else
      {
        currentKey2 = key;
        continuingCombo = false;
      }

      renderCurrentValue();
    }
    break;
  }
}

void HotKeyEdit::keyReleaseEvent( QKeyEvent * event )
{
  switch( event->key() )
  {
    case 0:
    case Qt::Key_unknown:
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
      continuingCombo = false;
    break;
  }

  QLineEdit::keyReleaseEvent( event );
}

bool HotKeyEdit::eventFilter( QObject *, QEvent * event )
{
  if( event->type() == QEvent::ShortcutOverride )
  {
    event->accept();
    return true;
  }
  return false;
}
