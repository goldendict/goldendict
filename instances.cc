/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "instances.hh"
#include <set>
#include <QBuffer>

namespace Instances {

using std::set;
using std::string;

Group::Group( Config::Group const & cfgGroup,
              std::vector< sptr< Dictionary::Class > > const & allDictionaries,
              Config::Group const & inactiveGroup ):
  id( cfgGroup.id ),
  name( cfgGroup.name ),
  icon( cfgGroup.icon ),
  favoritesFolder( cfgGroup.favoritesFolder ),
  shortcut( cfgGroup.shortcut )
{
  if ( !cfgGroup.iconData.isEmpty() )
    iconData = iconFromData( cfgGroup.iconData );

  QMap<string, sptr< Dictionary::Class > > groupDicts;
  QVector<string> dictOrderList;
  auto dictMap = Dictionary::dictToMap( allDictionaries );

  for( unsigned x = 0; x < (unsigned)cfgGroup.dictionaries.size(); ++x )
  {
    std::string id = cfgGroup.dictionaries[ x ].id.toStdString();

    if( dictMap.contains( id ) )
    {
      groupDicts.insert( id, dictMap[ id ] );
      dictOrderList.push_back(id);
    }
  }

  // Remove inactive dictionaries
  if( !inactiveGroup.dictionaries.isEmpty() )
  {
    set< string > inactiveSet;
    for( int i = 0; i < inactiveGroup.dictionaries.size(); i++ )
    {
      string id=inactiveGroup.dictionaries[ i ].id.toStdString();
      groupDicts.remove(id);
      dictOrderList.removeOne(id);
    }
  }
  for(const auto & dictId : dictOrderList)
  {
    dictionaries.push_back(groupDicts[dictId]);
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
  result.favoritesFolder = favoritesFolder;

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
  for( int x = 0; x < groups.size(); ++x )
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
