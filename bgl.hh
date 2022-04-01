/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __BGL_HH_INCLUDED__
#define __BGL_HH_INCLUDED__

#include "dictionary.hh"

/// Support for the Babylon's .bgl dictionaries.
namespace Bgl {

using std::vector;
using std::string;

/// Goes through the given list of file names, trying each one as a possible
/// dictionary. Upon finding one, creates a corresponding dictionary instance.
/// As a result, a list of dictionaries is created.
/// indicesDir indicates a directory where index files can be created, should
/// there be need for them. The index file name would be the same as the
/// dictionary's id, made by makeDictionaryId() from the list of file names.
/// Any exception thrown would terminate the program with an error.
vector< sptr< Dictionary::Class > > makeDictionaries(
                                    vector< string > const & fileNames,
                                    string const & indicesDir,
                                    Dictionary::Initializing & )
  ;

}

#endif
