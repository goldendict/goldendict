/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "groups.hh"
#include <QMessageBox>
#include <QInputDialog>

using std::vector;

Groups::Groups( QWidget * parent,
                vector< sptr< Dictionary::Class > > const & dicts_,
                Config::Groups const & groups_ ): QDialog( parent ),
  dicts( dicts_ ), groups( groups_ )
{
  ui.setupUi( this );

  // Populate the dictionaries' list

  ui.dictionaries->setAsSource();
  ui.dictionaries->populate( dicts, dicts );

  // Populate groups' widget

  ui.groups->populate( groups, dicts );

  connect( ui.addGroup, SIGNAL( clicked() ),
           this, SLOT( addNew() ) );
  connect( ui.renameGroup, SIGNAL( clicked() ),
           this, SLOT( renameCurrent() ) );
  connect( ui.removeGroup, SIGNAL( clicked() ),
           this, SLOT( removeCurrent() ) );

  countChanged();
}

Config::Groups Groups::getGroups() const
{
  return ui.groups->makeGroups();
}

void Groups::countChanged()
{
  bool en = ui.groups->count();

  ui.renameGroup->setEnabled( en );
  ui.removeGroup->setEnabled( en );
}

void Groups::addNew()
{
  bool ok;

  QString name = QInputDialog::getText( this, tr( "Add group" ),
                                  tr("Give a name for the new group:"), QLineEdit::Normal,
                                  "", &ok );

  if ( ok )
  {
    ui.groups->addNewGroup( name );
    countChanged();
  }
}

void Groups::renameCurrent()
{
  int current = ui.groups->currentIndex();

  if ( current < 0 )
    return;

  bool ok;

  QString name = QInputDialog::getText( this, tr("Rename group"),
                                  tr("Give a new name for the group:"), QLineEdit::Normal,
                                  ui.groups->tabText( current ), &ok );

  if ( ok )
    ui.groups->renameCurrentGroup( name );
}

void Groups::removeCurrent()
{
  int current = ui.groups->currentIndex();

  if ( current >= 0 && QMessageBox::question( this, tr( "Remove group" ),
         tr( "Are you sure you want to remove the group <b>%1</b>?" ).arg( ui.groups->tabText( current ) ),
         QMessageBox::Yes, QMessageBox::Cancel ) == QMessageBox::Yes )
  {
    ui.groups->removeCurrentGroup();
    countChanged();
  }
}
