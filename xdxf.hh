/* This file is (c) 2008-2009 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __XDXF_HH_INCLUDED__
#define __XDXF_HH_INCLUDED__

#include "dictionary.hh"

/// Support for the XDXF (.xdxf(.dz)) files.
namespace Xdxf {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & )
    throw( std::exception );

}

#endif
