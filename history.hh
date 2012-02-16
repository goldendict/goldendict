/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __HISTORY_HH_INCLUDED__
#define __HISTORY_HH_INCLUDED__

#include <QObject>
#include <QList>
#include <QString>

/// Search history
class History: public QObject
{
  Q_OBJECT

public:

  /// An item in history
  struct Item
  {
    /// Group the search was perfomed in
    unsigned groupId;
    /// The word that was searched
    QString word;

    Item(): groupId( 0 )
    {}

    Item( unsigned groupId_, QString const & word_ ):
      groupId( groupId_ ), word( word_ )
    {}

    bool operator == ( Item const & other ) const
    { return word == other.word && groupId == other.groupId; }

    bool operator != ( Item const & other ) const
    { return ! operator == ( other ); }
  };

  /// Indicates an intention to load -- see the relevant History constructor.
  struct Load {};

  /// Constructs an empty history which can hold at most "size" items.
  History( unsigned size = 20 );

  /// Loads history from its file. If load fails, the result would be an empty
  /// history. The size parameter is same as in other constructor.
  History( Load, unsigned size = 20 );

  /// Adds new item. The item is always added at the beginning of the list.
  /// If there was such an item already somewhere on the list, it gets removed
  /// from there. If otherwise the resulting list gets too large, the oldest
  /// item gets removed from the end of the list.
  void addItem( Item const & );

  /// Remove item with given index from list
  void removeItem( int index )
  { items.removeAt( index ); }

  /// Attempts saving history. Returns true if succeeded - false otherwise.
  /// Since history isn't really that valuable, failures can be ignored.
  bool save() const;

  /// Clears history.
  void clear();

  /// Gets the current items. The first one is the newest one on the list.
  QList< Item > const & getItems() const
  { return items; }

  /// Enable/disable add words to hystory
  void enableAdd( bool enable )
  { addingEnabled = enable; }
  bool enabled()
  { return addingEnabled; }

signals:

  /// Signals the changes in items in response to addItem() or clear().
  void itemsChanged();

private:

  QList< Item > items;
  unsigned maxSize;
  bool addingEnabled;
};

#endif
