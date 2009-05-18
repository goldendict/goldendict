/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "instances.hh"
#include <set>

namespace Instances {

using std::set;
using std::string;

Group::Group( Config::Group const & cfgGroup,
              vector< sptr< Dictionary::Class > > const & allDictionaries ):
  id( cfgGroup.id ),
  name( cfgGroup.name ),
  icon( cfgGroup.icon )
{
  for( unsigned x = 0; x < cfgGroup.dictionaries.size(); ++x )
  {
    std::string id = cfgGroup.dictionaries[ x ].id.toStdString();

    bool added = false;

    for( unsigned y = allDictionaries.size(); y--; )
      if ( allDictionaries[ y ]->getId() == id )
      {
        dictionaries.push_back( allDictionaries[ y ] );
        added = true;
        break;
      }

    if ( !added )
    {
      // Try matching by name instead
      std::string name = cfgGroup.dictionaries[ x ].name.toUtf8().data();

      if ( name.size() )
        for( unsigned y = 0; y < allDictionaries.size(); ++y )
          if ( allDictionaries[ y ]->getName() == name )
          {
            dictionaries.push_back( allDictionaries[ y ] );
            break;
          }
    }
  }
}

Group::Group( QString const & name_ ): id( 0 ), name( name_ )
{
}

Config::Group Group::makeConfigGroup()
{
  Config::Group result;

  result.id = id;
  result.name = name;
  result.icon = icon;

  for( unsigned x = 0; x < dictionaries.size(); ++x )
    result.dictionaries.push_back(
      Config::DictionaryRef( dictionaries[ x ]->getId().c_str(),
                             QString::fromUtf8( dictionaries[ x ]->getName().c_str() ) ) );

  return result;
}

Group * Groups::findGroup( unsigned id )
{
  for( unsigned x = 0; x < size(); ++x )
    if ( operator [] ( x ).id == id )
      return &( operator [] ( x ) );

  return 0;
}

Group const * Groups::findGroup( unsigned id ) const
{
  for( unsigned x = 0; x < size(); ++x )
    if ( operator [] ( x ).id == id )
      return &( operator [] ( x ) );

  return 0;
}

void complementDictionaryOrder( Config::Group & group,
                                Config::Group const & inactiveDictionaries,
                                vector< sptr< Dictionary::Class > > const & dicts )
{
  set< string > presentIds;

  for( unsigned x = group.dictionaries.size(); x--; )
    presentIds.insert( group.dictionaries[ x ].id.toStdString() );

  for( unsigned x = inactiveDictionaries.dictionaries.size(); x--; )
    presentIds.insert( inactiveDictionaries.dictionaries[ x ].id.toStdString() );

  for( unsigned x = 0; x < dicts.size(); ++x )
  {
    if ( presentIds.find( dicts[ x ]->getId() ) == presentIds.end() )
      group.dictionaries.push_back(
          Config::DictionaryRef( QString::fromStdString( dicts[ x ]->getId() ),
                                 QString::fromUtf8( dicts[ x ]->getName().c_str() ) ) );
  }
}

void updateNames( Config::Group & group,
                  vector< sptr< Dictionary::Class > > const & allDictionaries )
{

  for( unsigned x = group.dictionaries.size(); x--; )
  {
    std::string id = group.dictionaries[ x ].id.toStdString();

    for( unsigned y = allDictionaries.size(); y--; )
      if ( allDictionaries[ y ]->getId() == id )
      {
        group.dictionaries[ x ].name = QString::fromUtf8( allDictionaries[ y ]->getName().c_str() );
        break;
      }
  }
}

void updateNames( Config::Groups & groups,
                  vector< sptr< Dictionary::Class > > const & allDictionaries )
{
  for( unsigned x = 0; x < groups.size(); ++x )
    updateNames( groups[ x ], allDictionaries );
}

void updateNames( Config::Class & cfg,
                  vector< sptr< Dictionary::Class > > const & allDictionaries )
{
  updateNames( cfg.dictionaryOrder, allDictionaries );
  updateNames( cfg.inactiveDictionaries, allDictionaries );
  updateNames( cfg.groups, allDictionaries );
}

}
