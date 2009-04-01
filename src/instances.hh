/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __INSTANCES_HH_INCLUDED__
#define __INSTANCES_HH_INCLUDED__

#include "config.hh"
#include "dictionary.hh"

// This complements Config, providing instances for the stored configurations.
// Simply put, it can convert groups to ones which hold references to
// dictionaries directly, instead of only having their ids, and can also convert
// them back.

namespace Instances {

using std::vector;

struct Group
{
  QString name, icon;
  vector< sptr< Dictionary::Class > > dictionaries;

  /// Instantiates the given group from its configuration. If some dictionary
  /// wasn't found, it just skips it.
  Group( Config::Group const & cfgGroup,
         vector< sptr< Dictionary::Class > > const & allDictionaries );

  /// Creates an empty group.
  Group( QString const & name_ );

  /// Makes the configuration group from the current contents.
  Config::Group makeConfigGroup();

};

typedef vector< Group > Groups;

/// For any dictionaries present in the group, updates their names to match
/// the dictionaries they refer to in their current form, if they exist.
/// If the dictionary instance can't be located, the name is left untouched.
void updateNames( Config::Group &,
                  vector< sptr< Dictionary::Class > > const & allDictionaries );

}

#endif
