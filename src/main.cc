/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QApplication>
#include "mainwindow.hh"
#include "config.hh"

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

  // Try loading a style sheet if there's one

  #if 1
  QFile cssFile( Config::getUserQtCssFileName() );

  if ( cssFile.open( QFile::ReadOnly ) )
    app.setStyleSheet( cssFile.readAll() );

  #endif

  MainWindow m;

  m.show();

  return app.exec();
}


