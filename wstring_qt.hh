/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __WSTRING_QT_HH_INCLUDED__
#define __WSTRING_QT_HH_INCLUDED__

/// This file adds conversions between gd::wstring and QString. See wstring.hh
/// for more details on gd::wstring.

#include "wstring.hh"
#include <QString>

namespace gd
{
  QString toQString( wstring const & );
  wstring toWString( QString const & );
  wstring normalize( wstring const & );
}

#endif
