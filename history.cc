/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "history.hh"
#include "config.hh"
#include "atomic_rename.hh"
#include <QFile>
#include <QDebug>

History::History( unsigned size, unsigned maxItemLength_ ): maxSize( size ),
  maxItemLength( maxItemLength_ ), addingEnabled( true )
{
}

History::History( Load, unsigned size, unsigned maxItemLength_ ): maxSize( size ),
   maxItemLength( maxItemLength_ ), addingEnabled( true )
 , dirty( false )
 , timerId ( 0 )
{
  QFile file( Config::getHistoryFileName() );

  if ( !file.open( QFile::ReadOnly | QIODevice::Text ) )
    return; // No file -- no history

  for( unsigned count = 0 ; count < maxSize; ++count )
  {
    QByteArray lineUtf8 = file.readLine( 4096 );

    if ( lineUtf8.endsWith( '\n' ) )
      lineUtf8.chop( 1 );

    if ( lineUtf8.isEmpty() )
      break;

    QString line = QString::fromUtf8( lineUtf8 );

    int firstSpace = line.indexOf( ' ' );

    if ( firstSpace < 0 || firstSpace + 1 == line.size() )
      // No spaces or value? Bad line. End this.
      break;

    bool isNumber;

    unsigned groupId = line.left( firstSpace ).toUInt( &isNumber, 10 );

    if ( !isNumber )
      break; // That's not right

    items.push_back( Item( groupId, line.right( line.size() - firstSpace - 1 ) ) );
  }
}

History::Item History::getItem( int index )
{
  if ( index < 0 || index >= items.size() )
  {
    return Item();
  }
  return items.at( index );
}

void History::addItem( Item const & item )
{
  // qDebug() << "adding item " << item.word << ", enabled=" << enabled();
  if( !enabled() )
    return;

  if ( (unsigned)item.word.size() > getMaxItemLength() || item.word.isEmpty() )
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

  ensureSizeConstraints();

  dirty = true;

  emit itemsChanged();
}

bool History::ensureSizeConstraints()
{
  bool changed = false;
  while( items.size() > (int)maxSize )
  {
    items.pop_back();
    changed = true;
    dirty = true;
  }

  return changed;
}

void History::setMaxSize( unsigned maxSize_ )
{
  maxSize = maxSize_;
  if ( ensureSizeConstraints() )
  {
    emit itemsChanged();
  }
}

int History::size() const
{
  return items.size();
}

bool History::save()
{
  if( !dirty )
    return true;

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

  dirty = false;

  return renameAtomically( file.fileName(), Config::getHistoryFileName() );
}

void History::clear()
{
  items.clear();
  dirty = true;

  emit itemsChanged();
}

void History::setSaveInterval( unsigned interval )
{
  if( timerId )
  {
    killTimer( timerId );
    timerId = 0;
  }
  if( interval )
  {
    if( dirty )
      save();
    timerId = startTimer( interval * 60000 );
  }
}

void History::timerEvent( QTimerEvent * ev )
{
  Q_UNUSED( ev );
  save();
}
