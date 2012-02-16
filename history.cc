/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "history.hh"
#include "config.hh"
#include "atomic_rename.hh"
#include <QFile>

History::History( unsigned size ): maxSize( size ),
addingEnabled( true )
{
}

History::History( Load, unsigned size ): maxSize( size ),
addingEnabled( true )
{
  QFile file( Config::getHistoryFileName() );

  if ( !file.open( QFile::ReadOnly | QIODevice::Text ) )
    return; // No file -- no history

  for( unsigned count = 0 ; count < maxSize; ++count )
  {
    QByteArray lineUtf8 = file.readLine( 4096 );

    if ( lineUtf8.isEmpty() )
      break;

    if ( lineUtf8.endsWith( '\n' ) )
      lineUtf8.chop( 1 );

    QString line = QString::fromUtf8( lineUtf8 );

    int firstSpace = line.indexOf( ' ' );

    if ( firstSpace < 0 )
      // No spaces? Bad line. End this.
      break;

    bool isNumber;

    unsigned groupId = line.left( firstSpace ).toUInt( &isNumber, 10 );

    if ( !isNumber )
      break; // That's not right

    items.push_back( Item( groupId, line.right( line.size() - firstSpace - 1 ) ) );
  }
}

void History::addItem( Item const & item )
{
  if( !enabled() )
    return;

  if ( item.word.size() > 60 )
  {
    // The search looks bogus. Don't save it.
    return;
  }

  if ( items.contains( item ) )
    items.removeAll( item );

  // Special case: if this items differs from the previous one only by group,
  // remove it too.

  if ( items.size() && items.first().word == item.word )
    items.pop_front();

  items.push_front( item );

  while( items.size() > (int)maxSize )
    items.pop_back();

  emit itemsChanged();
}

bool History::save() const
{
  QFile file( Config::getHistoryFileName() + ".tmp" );

  if ( !file.open( QFile::WriteOnly | QIODevice::Text ) )
    return false;

  for( QList< Item >::const_iterator i = items.constBegin();
       i != items.constEnd(); ++i )
  {
    QByteArray line = i->word.toUtf8();

    // Those could ruin our format, so we replace them by spaces. They shouldn't
    // be there anyway.
    line.replace( '\n', ' ' );
    line.replace( '\r', ' ' );

    line = QByteArray::number( i->groupId ) + " " + line + "\n";

    if ( file.write( line ) != line.size() )
      return false;
  }

  file.close();

  return renameAtomically( file.fileName(), Config::getHistoryFileName() );
}

void History::clear()
{
  items.clear();

  emit itemsChanged();
}
