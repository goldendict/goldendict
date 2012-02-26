/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "groupcombobox.hh"
#include <QEvent>
#include <QShortcutEvent>

GroupComboBox::GroupComboBox( QWidget * parent ): QComboBox( parent ),
  popupAction( this )
{
  setSizeAdjustPolicy( AdjustToContents );
  setToolTip( tr( "Choose a Group (Alt+G)" ) );

  popupAction.setShortcut( QKeySequence( "Alt+G" ) );
  connect( &popupAction, SIGNAL( triggered() ),
           this, SLOT( popupGroups() ) );

  addAction( &popupAction );
}

void GroupComboBox::fill( Instances::Groups const & groups )
{
  unsigned prevId = 0;

  if ( count() )
    prevId = itemData( currentIndex() ).toUInt();

  clear();

  for( QMap< int, int >::iterator i = shortcuts.begin(); i != shortcuts.end(); ++i )
    releaseShortcut( i.key() );

  shortcuts.clear();

  for( unsigned x  = 0; x < groups.size(); ++x )
  {
    addItem( groups[ x ].makeIcon(),
             groups[ x ].name, groups[ x ].id );

    if ( prevId == groups[ x ].id )
      setCurrentIndex( x );

    // Create a shortcut

    if ( !groups[ x ].shortcut.isEmpty() )
    {
      int id = grabShortcut( groups[ x ].shortcut );
      setShortcutEnabled( id );

      shortcuts.insert( id, x );
    }
  }
}

bool GroupComboBox::event( QEvent * event )
{
  if ( event->type() == QEvent::Shortcut )
  {
    QShortcutEvent * ev = ( QShortcutEvent * ) event;

    QMap< int, int >::const_iterator i = shortcuts.find( ev->shortcutId() );

    if ( i != shortcuts.end() )
    {
      ev->accept();
      setCurrentIndex( i.value() );
      return true;
    }
  }

  return QComboBox::event( event );

}


void GroupComboBox::setCurrentGroup( unsigned id )
{
  for( int x  = 0; x < count(); ++x )
  {
    if ( itemData( x ).toUInt() == id )
    {
      setCurrentIndex( x );
      break;
    }
  }
}

unsigned GroupComboBox::getCurrentGroup() const
{
  if ( !count() )
    return 0;

  return itemData( currentIndex() ).toUInt();
}

void GroupComboBox::popupGroups()
{
  showPopup();
}
