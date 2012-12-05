/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "instances.hh"
#include <set>
#include <QBuffer>

namespace Instances {

using std::set;
using std::string;

Group::Group( Config::Group const & cfgGroup,
              vector< sptr< Dictionary::Class > > const & allDictionaries ):
  id( cfgGroup.id ),
  name( cfgGroup.name ),
  icon( cfgGroup.icon ),
  shortcut( cfgGroup.shortcut )
{
  if ( !cfgGroup.iconData.isEmpty() )
    iconData = iconFromData( cfgGroup.iconData );

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
      QString qname = cfgGroup.dictionaries[ x ].name;
      std::string name = qname.toUtf8().data();

      if ( !qname.isEmpty() )
      {

        // To avoid duplicates in dictionaries list we don't add dictionary
        // if it with such name was already added or presented in rest of list

        unsigned n;
        for( n = 0; n < dictionaries.size(); n++ )
          if( dictionaries[ n ]->getName() == name )
            break;
        if( n < dictionaries.size() )
          continue;

        for( n = x + 1; n < cfgGroup.dictionaries.size(); n++ )
          if( cfgGroup.dictionaries[ n ].name == qname )
            break;
        if( n < cfgGroup.dictionaries.size() )
          continue;

        for( unsigned y = 0; y < allDictionaries.size(); ++y )
          if ( allDictionaries[ y ]->getName() == name )
          {
            dictionaries.push_back( allDictionaries[ y ] );
            break;
          }
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
  result.shortcut = shortcut;

  if ( !iconData.isNull() )
  {
    QDataStream stream( &result.iconData, QIODevice::WriteOnly );

    stream << iconData;
  }

  for( unsigned x = 0; x < dictionaries.size(); ++x )
    result.dictionaries.push_back(
      Config::DictionaryRef( dictionaries[ x ]->getId().c_str(),
                             QString::fromUtf8( dictionaries[ x ]->getName().c_str() ) ) );

  return result;
}

QIcon Group::makeIcon() const
{
  if ( !iconData.isNull() )
    return iconData;

  QIcon i = icon.size() ?
                 QIcon( ":/flags/" + icon ) : QIcon();

  return i;
}

void Group::checkMutedDictionaries( Config::MutedDictionaries * mutedDictionaries ) const
{
  Config::MutedDictionaries temp;

  for( unsigned x = 0; x < dictionaries.size(); x++ )
  {
    QString id = QString::fromStdString( dictionaries[ x ]->getId() );
    if( mutedDictionaries->contains( id ) )
      temp.insert( id );
  }
  * mutedDictionaries = temp;
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

void complementDictionaryOrder( Group & group,
                                Group const & inactiveDictionaries,
                                vector< sptr< Dictionary::Class > > const & dicts )
{
  set< string > presentIds;

  for( unsigned x = group.dictionaries.size(); x--; )
    presentIds.insert( group.dictionaries[ x ]->getId());

  for( unsigned x = inactiveDictionaries.dictionaries.size(); x--; )
    presentIds.insert( inactiveDictionaries.dictionaries[ x ]->getId() );

  for( unsigned x = 0; x < dicts.size(); ++x )
  {
    if ( presentIds.find( dicts[ x ]->getId() ) == presentIds.end() )
      group.dictionaries.push_back( dicts[ x ] );
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

QIcon iconFromData( QByteArray const & iconData )
{
  QDataStream stream( iconData );

  QIcon result;

  stream >> result;

  return result;
}

}
