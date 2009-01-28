/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QApplication>
#include "mainwindow.hh"
#include "dictionary.hh"

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

  MainWindow m;

  m.show();

  return app.exec();
}


