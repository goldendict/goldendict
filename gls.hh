#ifndef __GLS_HH_INCLUDED__
#define __GLS_HH_INCLUDED__

#include "dictionary.hh"

/// Support for the Dabilon source .GLS files.
namespace Gls {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & )
    throw( std::exception );

}

#endif // __GLS_HH_INCLUDED__
