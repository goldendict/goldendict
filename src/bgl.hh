/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __BGL_HH_INCLUDED__
#define __BGL_HH_INCLUDED__

#include "dictionary.hh"

/// Support for the Babylon's .bgl dictionaries.
namespace Bgl {

using std::vector;
using std::string;

class Format: public Dictionary::Format
{
public:

  virtual vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & )
    throw( std::exception );
};

}

#endif
