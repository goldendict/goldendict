/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "initializing.hh"
#include <QCloseEvent>

Initializing::Initializing( QWidget * parent, bool showOnStartup ): QDialog( parent )
{
  ui.setupUi( this );
  setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                  Qt::WindowMinimizeButtonHint );

  #ifndef Q_OS_MAC
    setWindowIcon( QIcon( ":/icons/programicon.png" ) );
  #else
    setWindowIcon( QIcon( ":/icons/macicon.png" ) );
  #endif

  if ( showOnStartup )
  {
    ui.operation->setText( tr( "Please wait..." ) );
    ui.dictionary->hide();
    ui.progressBar->hide();
    show();
  }
}

void Initializing::indexing( QString const & dictionaryName )
{
  ui.operation->setText( tr( "Please wait while indexing dictionary" ) );
  ui.dictionary->setText( dictionaryName );
  ui.dictionary->show();
  ui.progressBar->show();
  adjustSize();
  show();
}

void Initializing::closeEvent( QCloseEvent * ev )
{
  ev->ignore();
}

void Initializing::reject()
{
}

