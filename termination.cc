
/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "termination.hh"
#include <exception>
#include <QtCore>


static void termHandler()
{
  QString message( "GoldenDict has crashed with an unexpected exception\n\n" );
  qDebug() << message;

  if( logFilePtr && logFilePtr->isOpen() )
    logFilePtr->close();

  abort();
}

void installTerminationHandler()
{
  std::set_terminate( termHandler );
}
