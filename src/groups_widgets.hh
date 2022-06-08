/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __GROUPS_WIDGETS_HH_INCLUDED__
#define __GROUPS_WIDGETS_HH_INCLUDED__

// Various custom widgets used in the Groups dialog

#include <vector>

#include <QAction>
#include <QListWidget>
#include <QSortFilterProxyModel>

#include "config.hh"
#include "dict/dictionary.hh"
#include "extlineedit.hh"

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
  void populate( std::vector< sptr< Dictionary::Class > > const & active );

  /// Marks that this model is used as an immutable dictionary source
  void setAsSource();
  bool sourceModel() const { return isSource; }

  /// Returns the dictionaries the model currently has listed
  std::vector< sptr< Dictionary::Class > > const & getCurrentDictionaries() const;

  void removeSelectedRows( QItemSelectionModel * source );
  void addSelectedUniqueFromModel( QItemSelectionModel * source );

  Qt::ItemFlags flags( QModelIndex const &index ) const;
  int rowCount( QModelIndex const & parent ) const;
  QVariant data( QModelIndex const & index, int role ) const;
  bool insertRows( int row, int count, const QModelIndex & parent );
  bool removeRows( int row, int count, const QModelIndex & parent );
  bool setData( QModelIndex const & index, const QVariant & value, int role );

  void addRow(const QModelIndex & parent, sptr< Dictionary::Class > dict);

  Qt::DropActions supportedDropActions() const;

  void filterDuplicates();

private:

  bool isSource;
  std::vector< sptr< Dictionary::Class > > dictionaries;
  std::vector< sptr< Dictionary::Class > > const * allDicts;

signals:
  void contentChanged();
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
  void populate( std::vector< sptr< Dictionary::Class > > const & active );

  /// Marks that this widget is used as an immutable dictionary source
  void setAsSource();

  /// Returns the dictionaries the widget currently has listed
  std::vector< sptr< Dictionary::Class > > const & getCurrentDictionaries() const;

  DictListModel * getModel()
  { return & model; }

signals:
  void gotFocus();

protected:
  virtual void dropEvent( QDropEvent * event );
  virtual void focusInEvent(QFocusEvent *);

  // We need these to to handle drag-and-drop focus issues
  virtual void rowsInserted( QModelIndex const & parent, int start, int end );
  virtual void rowsAboutToBeRemoved( QModelIndex const & parent, int start, int end );

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

  DictListModel * getModel() const
  { return ui.dictionaries->getModel(); }

  QItemSelectionModel * getSelectionModel() const
  { return ui.dictionaries->selectionModel(); }

private slots:

  void groupIconActivated( int );
  void showDictInfo( const QPoint & pos );
  void removeCurrentItem( QModelIndex const & );

private:
  Ui::DictGroupWidget ui;
  unsigned groupId;

signals:
  void showDictionaryInfo( QString const & id );
};

/// A tab widget with groups inside
class DictGroupsWidget: public QTabWidget
{
  Q_OBJECT

public:

  DictGroupsWidget( QWidget * parent );

  /// Creates all the tabs with the groups
  void populate( Config::Groups const &,
                 std::vector< sptr< Dictionary::Class > > const & allDicts,
                 std::vector< sptr< Dictionary::Class > > const & activeDicts );

  /// Creates new empty group with the given name
  void addNewGroup( QString const & );

  /// Creates new empty group with the given name if no such group
  /// and return it index
  int addUniqueGroup( QString const & name );

  void addAutoGroups();

  /// Returns currently chosen group's name
  QString getCurrentGroupName() const;

  /// Changes the name of the currently chosen group, if any, to the given one
  void renameCurrentGroup( QString const & );

  /// Removes the currently chosen group, if any
  void removeCurrentGroup();

  /// Removes all the groups
  void removeAllGroups();

  /// Creates groups from what is currently set up
  Config::Groups makeGroups() const;

  DictListModel * getCurrentModel() const;

  QItemSelectionModel * getCurrentSelectionModel() const;

private:

  /// Add source group to target group
  void combineGroups( int source, int target );

  unsigned nextId;
  std::vector< sptr< Dictionary::Class > > const * allDicts;
  std::vector< sptr< Dictionary::Class > > const * activeDicts;

private slots:
  void contextMenu( QPoint const & );
  void tabDataChanged();

signals:
  void showDictionaryInfo( QString const & id );
};

class QuickFilterLine: public ExtLineEdit
{
  Q_OBJECT

public:

  QuickFilterLine( QWidget * parent );
  ~QuickFilterLine();

  /// Sets the source view to filter
  void applyTo( QAbstractItemView * source );

  QAction * getFocusAction() { return & m_focusAction; }

  QModelIndex mapToSource( QModelIndex const & idx );

protected:
  virtual void keyPressEvent( QKeyEvent * event );

private:
  QSortFilterProxyModel m_proxyModel;
  QAction m_focusAction;
  QAbstractItemView * m_source;

private slots:
  void filterChangedInternal();
  void emitFilterChanged();
  void focusFilterLine();

signals:
  void filterChanged(QString const & filter);


};

#endif
