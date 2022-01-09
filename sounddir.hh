/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SOUNDDIR_HH_INCLUDED__
#define __SOUNDDIR_HH_INCLUDED__

#include "dictionary.hh"
#include "config.hh"

/// Support for sound dirs, arbitrary directories full of audio files.
namespace SoundDir {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries( Config::SoundDirs const &,
                                                      string const & indicesDir,
                                                      Dictionary::Initializing & )
  ;

}

#endif
