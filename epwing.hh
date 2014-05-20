#ifndef __EPWING_HH__INCLUDED__
#define __EPWING_HH__INCLUDED__

#include "dictionary.hh"

/// Support for the Epwing dictionaries.
namespace Epwing {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & )
    throw( std::exception );
}

#endif // __EPWING_HH__INCLUDED__
