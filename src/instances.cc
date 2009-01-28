/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "instances.hh"

namespace Instances {

Group::Group( Config::Group const & cfgGroup,
              vector< sptr< Dictionary::Class > > const & allDictionaries ):
  name( cfgGroup.name ),
  icon( cfgGroup.icon )
{
  for( unsigned x = 0; x < cfgGroup.dictionaries.size(); ++x )
  {
    std::string id = cfgGroup.dictionaries[ x ].toStdString();

    for( unsigned y = allDictionaries.size(); y--; )
      if ( allDictionaries[ y ]->getId() == id )
      {
        dictionaries.push_back( allDictionaries[ y ] );
        break;
      }
  }
}

Group::Group( QString const & name_ ): name( name_ )
{
}

Config::Group Group::makeConfigGroup()
{
  Config::Group result;

  result.name = name;
  result.icon = icon;

  for( unsigned x = 0; x < dictionaries.size(); ++x )
    result.dictionaries.push_back( dictionaries[ x ]->getId().c_str() );

  return result;
}

}
