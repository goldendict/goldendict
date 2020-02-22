/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "hotkeyedit.hh"
#include <QKeyEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

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

#ifdef Q_OS_WIN
    if( objectName() == "mainWindowHotkey" || objectName() == "clipboardHotkey" )
    {
        int newkey = VkeyToQTkey( event->nativeVirtualKey() );
        if( newkey )
            key = newkey;
    }
#endif

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

#ifdef Q_OS_WIN

int HotKeyEdit::VkeyToQTkey( quint32 vkey )
{
    if ( vkey >= Qt::Key_A && vkey <= Qt::Key_Z)
        return vkey;

    switch( vkey )
    {
    case VK_NUMPAD0:     return Qt::Key_0;
    case VK_NUMPAD1:     return Qt::Key_1;
    case VK_NUMPAD2:     return Qt::Key_2;
    case VK_NUMPAD3:     return Qt::Key_3;
    case VK_NUMPAD4:     return Qt::Key_4;
    case VK_NUMPAD5:     return Qt::Key_5;
    case VK_NUMPAD6:     return Qt::Key_6;
    case VK_NUMPAD7:     return Qt::Key_7;
    case VK_NUMPAD8:     return Qt::Key_8;
    case VK_NUMPAD9:     return Qt::Key_9;
    case VK_DIVIDE:      return Qt::Key_Slash;
    case VK_MULTIPLY:    return Qt::Key_Asterisk;
    case VK_SUBTRACT:    return Qt::Key_Minus;
    case VK_ADD:         return Qt::Key_Plus;
    case VK_DECIMAL:     return Qt::Key_Period;
    case VK_F1:          return Qt::Key_F1;
    case VK_F2:          return Qt::Key_F2;
    case VK_F3:          return Qt::Key_F3;
    case VK_F4:          return Qt::Key_F4;
    case VK_F5:          return Qt::Key_F5;
    case VK_F6:          return Qt::Key_F6;
    case VK_F7:          return Qt::Key_F7;
    case VK_F8:          return Qt::Key_F8;
    case VK_F9:          return Qt::Key_F9;
    case VK_F10:         return Qt::Key_F10;
    case VK_F11:         return Qt::Key_F11;
    case VK_F12:         return Qt::Key_F12;
    case VK_F13:         return Qt::Key_F13;
    case VK_F14:         return Qt::Key_F14;
    case VK_F15:         return Qt::Key_F15;
    case VK_F16:         return Qt::Key_F16;
    case VK_F17:         return Qt::Key_F17;
    case VK_F18:         return Qt::Key_F18;
    case VK_F19:         return Qt::Key_F19;
    case VK_F20:         return Qt::Key_F20;
    case VK_F21:         return Qt::Key_F21;
    case VK_F22:         return Qt::Key_F22;
    case VK_F23:         return Qt::Key_F23;
    case VK_F24:         return Qt::Key_F24;
    case 0x30:           return Qt::Key_ParenRight;
    case 0x31:           return Qt::Key_Exclam;
    case 0x32:           return Qt::Key_At;
    case 0x33:           return Qt::Key_NumberSign;
    case 0x34:           return Qt::Key_Dollar;
    case 0x35:           return Qt::Key_Percent;
    case 0x36:           return Qt::Key_AsciiCircum;
    case 0x37:           return Qt::Key_Ampersand;
    case 0x38:           return Qt::Key_copyright;
    case 0x39:           return Qt::Key_ParenLeft;
    case VK_OEM_MINUS:   return Qt::Key_Underscore;
    case VK_OEM_PLUS:    return Qt::Key_Equal;
    case VK_OEM_1:       return Qt::Key_Semicolon;
    case VK_OEM_2:       return Qt::Key_Question;
    case VK_OEM_3:       return Qt::Key_QuoteLeft;
    case VK_OEM_4:       return Qt::Key_BracketLeft;
    case VK_OEM_5:       return Qt::Key_Backslash;
    case VK_OEM_6:       return Qt::Key_BracketRight;
    case VK_OEM_7:       return Qt::Key_Apostrophe;
    case VK_OEM_COMMA:   return Qt::Key_Less;
    case VK_OEM_PERIOD:  return Qt::Key_Greater;

    default:;
    }

    return 0;
}

#endif
