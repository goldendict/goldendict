/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __DICTDFILES_HH_INCLUDED__
#define __DICTDFILES_HH_INCLUDED__

#include "dictionary.hh"

/// Support for the dictd (.index/dict.dz) files.
namespace DictdFiles {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & )
    throw( std::exception );

}

#endif
