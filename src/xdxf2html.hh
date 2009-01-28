/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __XDXF2HTML_HH_INCLUDED__
#define __XDXF2HTML_HH_INCLUDED__

#include <string>

/// Xdxf is an xml file format. Since we display html, we'd like to be able
/// to convert articles with such a markup to an html.
namespace Xdxf2Html {

using std::string;

/// Converts the given xdxf markup to an html one. This is currently used
/// for Stardict's 'x' records.
string convert( string const & );

}

#endif
