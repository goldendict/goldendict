/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __GROUPS_WIDGETS_HH_INCLUDED__
#define __GROUPS_WIDGETS_HH_INCLUDED__

// Various custom widgets used in the Groups dialog

#include <QListWidget>
#include "config.hh"
#include "dictionary.hh"
#include <vector>

/// A model to be projected into the view, according to Qt's MVC model
class DictListModel: public QAbstractListModel
{
  Q_OBJECT

public:

  DictListModel( QWidget * parent ):
    QAbstractListModel( parent ), isSource( false ), allDicts( 0 )
  {}

  /// Populates the current model with the given dictionaries. This is
  /// ought to be part of construction process.
  void populate( std::vector< sptr< Dictionary::Class > > const & active,
                 std::vector< sptr< Dictionary::Class > > const & available );

  /// Marks that this model is used as an immutable dictionary source
  void setAsSource();
  /// Returns the dictionaries the model currently has listed
  std::vector< sptr< Dictionary::Class > > const & getCurrentDictionaries() const;

  Qt::ItemFlags flags( QModelIndex const &index ) const;
  int rowCount( QModelIndex const & parent ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool insertRows( int row, int count, const QModelIndex & parent );
  bool removeRows( int row, int count, const QModelIndex & parent );
  bool setData( QModelIndex const & index, const QVariant & value, int role );

  Qt::DropActions supportedDropActions() const;

private:

  bool isSource;
  std::vector< sptr< Dictionary::Class > > dictionaries;
  std::vector< sptr< Dictionary::Class > > const * allDicts;
};

/// This widget is for dictionaries' lists, it handles drag-n-drop operations
/// with them etc.
class DictListWidget: public QListView
{
  Q_OBJECT
public:
  DictListWidget( QWidget * parent );
  ~DictListWidget();

  /// Populates the current list with the given dictionaries.
  void populate( std::vector< sptr< Dictionary::Class > > const & active,
                 std::vector< sptr< Dictionary::Class > > const & available );

  /// Marks that this widget is used as an immutable dictionary source
  void setAsSource();

  /// Returns the dictionaries the widget currently has listed
  std::vector< sptr< Dictionary::Class > > const & getCurrentDictionaries() const;


private:

  DictListModel model;
};

#include "ui_dictgroupwidget.h"

/// A widget that is placed into each tab in the Groups dialog.
class DictGroupWidget: public QWidget
{
  Q_OBJECT

public:
  DictGroupWidget( QWidget * parent,
                   std::vector< sptr< Dictionary::Class > > const &,
                   Config::Group const & );

  /// Makes the group's configuration out of the data currently held.
  /// Since the group's name is not part of the widget by design right now
  /// (it is known by the containing tab widget only), it is returned as empty.
  Config::Group makeGroup() const;

private:
  Ui::DictGroupWidget ui;
  unsigned groupId;
};

/// A tab widget with groups inside
class DictGroupsWidget: public QTabWidget
{
  Q_OBJECT

public:

  DictGroupsWidget( QWidget * parent );

  /// Creates all the tabs with the groups
  void populate( Config::Groups const &,
                 std::vector< sptr< Dictionary::Class > > const & allDicts );

  /// Creates new empty group with the given name
  void addNewGroup( QString const & );

  /// Returns currently chosen group's name
  QString getCurrentGroupName() const;

  /// Changes the name of the currently chosen group, if any, to the given one
  void renameCurrentGroup( QString const & );

  /// Removes the currently chosen group, if any
  void removeCurrentGroup();

  /// Creates groups from what is currently set up
  Config::Groups makeGroups() const;

private:

  unsigned nextId;
  std::vector< sptr< Dictionary::Class > > const * allDicts;
};

#endif

