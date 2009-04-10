/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "groupcombobox.hh"

GroupComboBox::GroupComboBox( QWidget * parent ): QComboBox( parent )
{
  setSizeAdjustPolicy( AdjustToContents );
}

void GroupComboBox::fill( Instances::Groups const & groups )
{
  unsigned prevId = 0;

  if ( count() )
    prevId = itemData( currentIndex() ).toUInt();

  clear();

  for( unsigned x  = 0; x < groups.size(); ++x )
  {
    QIcon icon = groups[ x ].icon.size() ?
                   QIcon( ":/flags/" + groups[ x ].icon ) : QIcon();

    addItem( icon, groups[ x ].name, groups[ x ].id );

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

