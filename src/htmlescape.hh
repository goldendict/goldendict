/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __HTMLESCAPE_HH_INCLUDED__
#define __HTMLESCAPE_HH_INCLUDED__

#include <string>

namespace Html {

using std::string;

// Replaces &, <, > and " by their html entity equivalents
// The " is not really required to be escaped in html, but is replaced anyway
// to make the result suitable for inserting as attributes' values.
string escape( string const & );

}

#endif
