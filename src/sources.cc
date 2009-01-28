/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "sources.hh"
#include <QFileDialog>
#include <QMessageBox>

Sources::Sources( QWidget * parent, Config::Paths const & paths_ ): QDialog( parent ),
  paths( paths_ )
{
  ui.setupUi( this );

  for( Config::Paths::const_iterator i = paths.begin(); i != paths.end(); ++i )
    ui.dictionaries->addItem( *i );

  ui.dictionaries->setDragEnabled( true );
  ui.dictionaries->setAcceptDrops( true );
  ui.dictionaries->setDropIndicatorShown( true );

  connect( ui.buttons, SIGNAL( accepted() ),
            this, SLOT( accept() ) );
  connect( ui.buttons, SIGNAL( rejected() ),
            this, SLOT( reject() ) );

  connect( ui.add, SIGNAL( clicked() ),
            this, SLOT( add() ) );
  connect( ui.remove, SIGNAL( clicked() ),
            this, SLOT( remove() ) );
}

void Sources::add()
{
  QString dir = 
    QFileDialog::getExistingDirectory( this, tr( "Choose a directory" ) );

  if ( !dir.isEmpty() )
  {
    paths.push_back( dir );
    ui.dictionaries->addItem( dir );
  }
}

void Sources::remove()
{
  int row = ui.dictionaries->currentRow();

  if ( row >= 0 && row < (int)paths.size() &&
      QMessageBox::question( this, tr( "Confirm removal" ),
                             tr( "Remove directory <b>%1</b> from the list?" ).arg( paths[ row ] ),
                             QMessageBox::Ok,
                             QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    if( QListWidgetItem * item = ui.dictionaries->takeItem( row ) )
      delete item;
    paths.erase( paths.begin() + row );
  }
}

