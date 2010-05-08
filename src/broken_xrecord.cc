/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "broken_xrecord.hh"

#include <QtGui>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <QX11Info>
#endif

bool isRECORDBroken()
{
#ifdef Q_WS_X11

 char const * vendor = ServerVendor( QX11Info::display() );

 if ( vendor && strstr( vendor, "X.Org" ) )
 {
   int release = VendorRelease( QX11Info::display() );

   return release >= 10600000 && release < 10701000;
 }

#endif

  return false;
}
