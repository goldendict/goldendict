/* This file is (c) 2013 Timon Wong <timon86.wang AT gmail DOT com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __MDX_HH_INCLUDED__
#define __MDX_HH_INCLUDED__

#include "dictionary.hh"

namespace Mdx
{

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries( vector< string > const & fileNames,
                                                      string const & indicesDir,
                                                      Dictionary::Initializing & ) throw( std::exception );

}

#endif // __MDX_HH_INCLUDED__
