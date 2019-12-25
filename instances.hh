/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __INSTANCES_HH_INCLUDED__
#define __INSTANCES_HH_INCLUDED__

#include "config.hh"
#include "dictionary.hh"
#include <QIcon>
#include <limits.h>

// This complements Config, providing instances for the stored configurations.
// Simply put, it can convert groups to ones which hold references to
// dictionaries directly, instead of only having their ids, and can also convert
// them back.

namespace Instances {

using std::vector;

struct Group
{
  unsigned id;
  QString name, icon, favoritesFolder;
  QIcon iconData;
  QKeySequence shortcut;
  vector< sptr< Dictionary::Class > > dictionaries;

  /// Instantiates the given group from its configuration. If some dictionary
  /// wasn't found, it just skips it.
  Group( Config::Group const & cfgGroup,
         vector< sptr< Dictionary::Class > > const & allDictionaries,
         Config::Group const & inactiveGroup );

  /// Creates an empty group.
  Group( QString const & name_ );

  virtual ~Group() {}

  /// Makes the configuration group from the current contents.
  Config::Group makeConfigGroup();

  /// Makes an icon object for this group, based on the icon's name or iconData.
  QIcon makeIcon() const;

  /// Remove id's if not presented in group dictionaries
  void checkMutedDictionaries( Config::MutedDictionaries * mutedDictionaries ) const;

  // Some constants

  /// The id of the 'All' group
  static const unsigned AllGroupId = UINT_MAX - 1;

  /// The id of the fictious 'Help' group
  static const unsigned HelpGroupId = UINT_MAX;

  /// Invalid value, used to specify that no group id is specified at all.
  static const unsigned NoGroupId = 0;
};

struct Groups: public vector< Group >
{
  /// Tries finding the given group by its id. Returns the group found, or
  /// 0 if there's no such group.
  Group * findGroup( unsigned id );
  Group const * findGroup( unsigned id ) const;

  virtual ~Groups() {}
};

/// Adds any dictionaries not already present in the given group or in
/// inactiveDictionaires to its end. Meant to be used with dictionaryOrder
/// special group.
void complementDictionaryOrder( Group & dictionaryOrder,
                                Group const & inactiveDictionaries,
                                vector< sptr< Dictionary::Class > > const &
                                allDictionaries );

/// For any dictionaries present in the group, updates their names to match
/// the dictionaries they refer to in their current form, if they exist.
/// If the dictionary instance can't be located, the name is left untouched.
void updateNames( Config::Group &,
                  vector< sptr< Dictionary::Class > > const & allDictionaries );

/// Does updateNames() for a set of given groups.
void updateNames( Config::Groups &,
                  vector< sptr< Dictionary::Class > > const & allDictionaries );

/// Does updateNames() for any relevant dictionary groups present in the
/// configuration.
void updateNames( Config::Class &,
                  vector< sptr< Dictionary::Class > > const & allDictionaries );

/// Creates icon from icon data. Used by Group, but also by others who work
/// with icon data directly.
QIcon iconFromData( QByteArray const & );

}

#endif
