/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "initializing.hh"
#include <QCloseEvent>

Initializing::Initializing( QWidget * parent ): QDialog( parent )
{
  ui.setupUi( this );
  setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint );
  
  setWindowIcon( QIcon( ":/icons/programicon.png" ) );
}

void Initializing::indexing( QString const & dictionaryName )
{
  ui.operation->setText( tr( "Please wait while indexing dictionary" ) );
  ui.dictionary->setText( dictionaryName );
  show();
}

void Initializing::closeEvent( QCloseEvent * ev )
{
  ev->ignore();
}

void Initializing::reject()
{
}

