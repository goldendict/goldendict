/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __GROUPCOMBOBOX_HH_INCLUDED__
#define __GROUPCOMBOBOX_HH_INCLUDED__

#include <QComboBox>
#include "instances.hh"

/// This is a combo box which is for choosing the dictionary group
class GroupComboBox: public QComboBox
{
  Q_OBJECT

public:

  GroupComboBox( QWidget * parent );

  /// Fills combo-box with the given groups
  void fill( Instances::Groups const & );
};

#endif

