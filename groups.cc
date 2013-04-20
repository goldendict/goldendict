/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "groups.hh"
#include "instances.hh"
#include "dictionary.hh"
#include <QMessageBox>
#include <QInputDialog>

using std::vector;

Groups::Groups( QWidget * parent,
                vector< sptr< Dictionary::Class > > const & dicts_,
                Config::Groups const & groups_,
                Config::Group const & order ): QWidget( parent ),
  dicts( dicts_ ), groups( groups_ )
{
  ui.setupUi( this );

  // Populate the dictionaries' list

  ui.dictionaries->setAsSource();
  ui.dictionaries->populate( Instances::Group( order, dicts ).dictionaries,
                             dicts );

  ui.searchLine->applyTo( ui.dictionaries );
  addAction( ui.searchLine->getFocusAction() );

  // Populate groups' widget

  ui.groups->populate( groups, dicts, ui.dictionaries->getCurrentDictionaries() );

  connect( ui.addGroup, SIGNAL( clicked() ),
           this, SLOT( addNew() ) );
  connect( ui.renameGroup, SIGNAL( clicked() ),
           this, SLOT( renameCurrent() ) );
  connect( ui.removeGroup, SIGNAL( clicked() ),
           this, SLOT( removeCurrent() ) );
  connect( ui.removeAllGroups, SIGNAL( clicked() ),
           this, SLOT( removeAll() ) );
  connect( ui.addDictsToGroup, SIGNAL( clicked() ),
           this, SLOT( addToGroup() ) );
  connect( ui.dictionaries, SIGNAL(  doubleClicked(const QModelIndex &) ),
           this, SLOT( addToGroup() ) );
  connect( ui.removeDictsFromGroup, SIGNAL( clicked() ),
           this, SLOT( removeFromGroup() ) );
  connect( ui.autoGroups, SIGNAL( clicked() ),
           this, SLOT( addAutoGroups() ) );
  connect( ui.groups, SIGNAL( showDictionaryInfo( QString const & ) ),
           this, SIGNAL( showDictionaryInfo( QString const & ) ) );

  ui.dictionaries->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( ui.dictionaries, SIGNAL( customContextMenuRequested( QPoint ) ),
           this, SLOT( showDictInfo( QPoint ) ) );

  countChanged();
}

void Groups::editGroup( unsigned id )
{
  for( int x = 0; x < groups.size(); ++x )
  {
    if ( groups[ x ].id == id )
    {
      ui.groups->setCurrentIndex( x );
      ui.groups->currentWidget()->setFocus();
      break;
    }
  }
}

void Groups::updateDictionaryOrder( Config::Group const & order )
{
  // Make sure it differs from what we have

  Instances::Group newOrder( order, dicts );

  if ( ui.dictionaries->getCurrentDictionaries() != newOrder.dictionaries )
  {
    // Repopulate
    ui.dictionaries->populate( Instances::Group( order, dicts ).dictionaries,
                               dicts );
  }
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
  ui.removeAllGroups->setEnabled( en );

  int stretch = ui.groups->count() / 5;
  if( stretch > 3 ) stretch = 3;
  ui.gridLayout->setColumnStretch( 2, stretch );
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

void Groups::addAutoGroups()
{
  ui.groups->addAutoGroups();
  countChanged();
}

void Groups::renameCurrent()
{
  int current = ui.groups->currentIndex();

  if ( current < 0 )
    return;

  bool ok;

  QString name = QInputDialog::getText( this, tr("Rename group"),
                                  tr("Give a new name for the group:"), QLineEdit::Normal,
                                  ui.groups->getCurrentGroupName(), &ok );

  if ( ok )
    ui.groups->renameCurrentGroup( name );
}

void Groups::removeCurrent()
{
  int current = ui.groups->currentIndex();

  if ( current >= 0 && QMessageBox::question( this, tr( "Remove group" ),
         tr( "Are you sure you want to remove the group <b>%1</b>?" ).arg( ui.groups->getCurrentGroupName() ),
         QMessageBox::Yes, QMessageBox::Cancel ) == QMessageBox::Yes )
  {
    ui.groups->removeCurrentGroup();
    countChanged();
  }
}

void Groups::removeAll()
{
  int current = ui.groups->currentIndex();

  if ( current >= 0 && QMessageBox::question( this, tr( "Remove all groups" ),
         tr( "Are you sure you want to remove all the groups?" ),
         QMessageBox::Yes, QMessageBox::Cancel ) == QMessageBox::Yes )
  {
    ui.groups->removeAllGroups();
    countChanged();
  }
}

void Groups::addToGroup()
{
  int current = ui.groups->currentIndex();

  if ( current >= 0 )
  {
    ui.groups->getCurrentModel()->addSelectedUniqueFromModel( ui.dictionaries->selectionModel() );
  }
}

void Groups::removeFromGroup()
{
  int current = ui.groups->currentIndex();

  if ( current >= 0 )
  {
    ui.groups->getCurrentModel()->removeSelectedRows( ui.groups->getCurrentSelectionModel() );
  }
}

void Groups::showDictInfo( QPoint const & pos )
{
  QVariant data = ui.dictionaries->getModel()->data(
        ui.searchLine->mapToSource(ui.dictionaries->indexAt( pos ) ), Qt::EditRole );
  QString id;
  if( data.canConvert< QString >() )
    id = data.toString();

  if( !id.isEmpty() )
  {
    vector< sptr< Dictionary::Class > > const & dicts = ui.dictionaries->getCurrentDictionaries();
    unsigned n;
    for( n = 0; n < dicts.size(); n++ )
      if( id.compare( QString::fromUtf8( dicts.at( n )->getId().c_str() ) ) == 0 )
        break;
    if( n < dicts.size() )
      emit showDictionaryInfo( id );
  }
}


