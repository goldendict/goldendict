/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "scanpopup.hh"
#include <QUrl>

ScanPopup::ScanPopup( QWidget * parent,
                      ArticleNetworkAccessManager & articleNetMgr_ ):
  QDialog( parent ), articleNetMgr( articleNetMgr_ )
{
  ui.setupUi( this );

  //setWindowFlags( Qt::Tool );

  ui.definition->page()->setNetworkAccessManager( &articleNetMgr );

  #if 0 // Since this is unconditional this bugs a lot
  connect( QApplication::clipboard(), SIGNAL( changed( QClipboard::Mode ) ),
           this, SLOT( clipboardChanged( QClipboard::Mode ) ) );
  #endif
}

void ScanPopup::clipboardChanged( QClipboard::Mode m )
{
  printf( "clipboard changed\n" );

  QString subtype = "plain";

  QString inWord = QApplication::clipboard()->text( subtype, m );

  if ( !inWord.size() )
    return;

  setWindowTitle( inWord );

  show();
  activateWindow();
  //raise();

  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  req.addQueryItem( "word", inWord );
  req.addQueryItem( "group",
                    "whatever" );

  ui.definition->load( req );
}

