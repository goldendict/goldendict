/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __GROUPS_HH_INCLUDED__
#define __GROUPS_HH_INCLUDED__

#include "ui_groups.h"
#include "config.hh"
#include "dictionary.hh"
#include "groups_widgets.hh"
#include <QToolButton>

class Groups: public QWidget
{
  Q_OBJECT

public:
  Groups( QWidget * parent,
          std::vector< sptr< Dictionary::Class > > const &,
          Config::Groups const &,
          Config::Group const & order );

  /// Instructs the dialog to position itself on editing the given group.
  void editGroup( unsigned id );

  /// Should be called when the dictionary order has changed to reflect on
  /// that changes. It would only do anything if the order has actually
  /// changed.
  void updateDictionaryOrder( Config::Group const & order );

  Config::Groups getGroups() const;

private:
  Ui::Groups ui;
  std::vector< sptr< Dictionary::Class > > const & dicts;
  Config::Groups groups;

  // Reacts to the event that the number of groups is possibly changed
  void countChanged();

private slots:
  void addNew();
  void renameCurrent();
  void removeCurrent();
  void removeAll();
  void addToGroup();
  void removeFromGroup();
  void addAutoGroups();
};

#endif
