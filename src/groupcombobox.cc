/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "groupcombobox.hh"

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

  for( unsigned x  = 0; x < groups.size(); ++x )
  {
    addItem( groups[ x ].makeIcon(),
             groups[ x ].name, groups[ x ].id );

    if ( prevId == groups[ x ].id )
      setCurrentIndex( x );
  }
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
