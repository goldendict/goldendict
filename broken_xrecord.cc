/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "broken_xrecord.hh"

#include <QtGui>

#ifdef HAVE_X11
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtGui/private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#endif

bool isRECORDBroken()
{
#ifdef HAVE_X11

 char const * vendor = ServerVendor( QX11Info::display() );

 if ( vendor && strstr( vendor, "X.Org" ) )
 {
   int release = VendorRelease( QX11Info::display() );

   return release >= 10600000 && release < 10701000;
 }

#endif

  return false;
}
