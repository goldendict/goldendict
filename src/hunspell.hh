/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __HUNSPELL_HH_INCLUDED__
#define __HUNSPELL_HH_INCLUDED__

#include "dictionary.hh"
#include "config.hh"

/// Support for Hunspell-based morphology.
namespace HunspellMorpho {

using std::vector;
using std::string;

struct DataFiles
{
  QString affFileName, dicFileName; // Absolute, with Qt separators
  QString dictId; // Dictionary id, e.g. "en_US"
  QString dictName; // Localized dictionary name to be displayed, e.g. "English(US) Morphology"

  DataFiles( QString const & affFileName_, QString const & dicFileName_,
             QString const & dictId_, QString const & dictName_ ):
    affFileName( affFileName_ ), dicFileName( dicFileName_ ),
    dictId( dictId_ ), dictName( dictName_ )
  {}
};

/// Finds all the DataFiles it can at the given path (with Qt separators).
vector< DataFiles > findDataFiles( QString const & path );

vector< sptr< Dictionary::Class > > makeDictionaries( Config::Hunspell const & )
  throw( std::exception );

}

#endif
