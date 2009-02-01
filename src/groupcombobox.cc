/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "groupcombobox.hh"

GroupComboBox::GroupComboBox( QWidget * parent ): QComboBox( parent )
{
  setSizeAdjustPolicy( AdjustToContents );
}

void GroupComboBox::fill( Instances::Groups const & groups )
{
  QString prev = currentText();

  clear();

  for( unsigned x  = 0; x < groups.size(); ++x )
  {
    QIcon icon = groups[ x ].icon.size() ?
                   QIcon( ":/flags/" + groups[ x ].icon ) : QIcon();

    addItem( icon, groups[ x ].name );

    if ( prev == groups[ x ].name )
      setCurrentIndex( x );
  }
}

