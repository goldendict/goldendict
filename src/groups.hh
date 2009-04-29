/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __GROUPS_HH_INCLUDED__
#define __GROUPS_HH_INCLUDED__

#include "ui_groups.h"
#include "config.hh"
#include "dictionary.hh"
#include "groups_widgets.hh"
#include <QToolButton>

class Groups: public QDialog
{
  Q_OBJECT

public:
  Groups( QWidget * parent,
          std::vector< sptr< Dictionary::Class > > const &,
          Config::Groups const & );

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
};

#endif
