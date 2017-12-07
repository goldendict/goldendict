/* This file is (c) 2017 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FAVORITIESPANEWIDGET_HH__INCLUDED__
#define __FAVORITIESPANEWIDGET_HH__INCLUDED__

#include <QWidget>
#include <QSize>
#include <QAbstractItemModel>
#include <QTreeView>
#include <QLabel>
#include <QHBoxLayout>
#include <QMenu>
#include <QDomNode>
#include <QList>
#include <QMimeData>

#include <config.hh>
#include "delegate.hh"

class FavoritesModel;

class FavoritesPaneWidget : public QWidget
{
  Q_OBJECT
public:
  FavoritesPaneWidget( QWidget * parent = 0 ): QWidget( parent ),
    itemSelectionChanged( false )
  , listItemDelegate( 0 )
  , m_favoritesModel( 0 )
  , timerId( 0 )
  {}

  virtual ~FavoritesPaneWidget();

  virtual QSize sizeHint() const
  { return QSize( 204, 204 ); }

  void setUp( Config::Class * cfg, QMenu * menu );

  void addHeadword( QString const & path, QString const & headword );

  bool removeHeadword( QString const & path, QString const & headword );

  // Export/import Favorites
  void getDataInXml( QByteArray & dataStr );
  void getDataInPlainText( QString & dataStr );
  bool setDataFromXml( QString const & dataStr );

  void setFocusOnTree()
  { m_favoritesTree->setFocus(); }

  // Set interval for periodical save
  void setSaveInterval( unsigned interval );

  // Return true if headwors is already presented in Favorites
  bool isHeadwordPresent( QString const & path, QString const & headword );

signals:
  void favoritesItemRequested( QString const & word, QString const & faforitesFolder );

protected:
  virtual void timerEvent( QTimerEvent * ev );

private slots:
  void emitFavoritesItemRequested(QModelIndex const &);
  void onSelectionChanged(QItemSelection const & selection);
  void onItemClicked(QModelIndex const & idx);
  void showCustomMenu(QPoint const & pos);
  void deleteSelectedItems();
  void copySelectedItems();
  void addFolder();

private:
  virtual bool eventFilter( QObject *, QEvent * );

  Config::Class * m_cfg ;
  QTreeView * m_favoritesTree;
  QMenu * m_favoritesMenu;
  QAction * m_deleteSelectedAction;
  QAction * m_separator;
  QAction * m_copySelectedToClipboard;
  QAction * m_addFolder;

  QWidget favoritesPaneTitleBar;
  QHBoxLayout favoritesPaneTitleBarLayout;
  QLabel favoritesLabel;

  /// needed to avoid multiple notifications
  /// when selecting items via mouse and keyboard
  bool itemSelectionChanged;

  WordListItemDelegate * listItemDelegate;
  FavoritesModel * m_favoritesModel;

  int timerId;
};


class TreeItem
{
public:
  enum Type { Word, Folder, Root };

  TreeItem( const QVariant &data, TreeItem *parent = 0, Type type_ = Word );
  ~TreeItem();

  void appendChild( TreeItem * child );

  void insertChild( int row, TreeItem * item );

  // Remove child from list and delete it
  void deleteChild( int row );

  TreeItem * child( int row ) const;
  int childCount() const;
  QVariant data() const;
  void setData( const QVariant & newData );
  int row() const;
  TreeItem * parent();

  Type type() const
  { return m_type; }

  Qt::ItemFlags flags() const;

  void setExpanded( bool expanded )
  { m_expanded = expanded; }

  bool isExpanded() const
  { return m_expanded; }

  // Full path from root folder
  QString fullPath() const;

  // Duplicate item with all childs
  TreeItem * duplicateItem( TreeItem * newParent ) const;

  // Check if item is ancestor of this element
  bool haveAncestor( TreeItem * item );

  // Check if same item already presented between childs
  bool haveSameItem( TreeItem * item, bool allowSelf );

  // Retrieve text from all childs
  QStringList getTextFromAllChilds() const;

private:
  QList< TreeItem * > childItems;
  QVariant itemData;
  TreeItem *parentItem;
  Type m_type;
  bool m_expanded;
};

class FavoritesModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit FavoritesModel( QString favoritesFilename, QObject * parent = 0 );
  ~FavoritesModel();

  QVariant data( const QModelIndex &index, int role ) const;
  Qt::ItemFlags flags( const QModelIndex &index ) const;
  QVariant headerData( int section, Qt::Orientation orientation,
                       int role = Qt::DisplayRole ) const;
  QModelIndex index( int row, int column,
                     const QModelIndex &parent = QModelIndex() ) const;
  QModelIndex parent( const QModelIndex &index ) const;
  int rowCount( const QModelIndex &parent = QModelIndex() ) const;
  int columnCount( const QModelIndex &parent = QModelIndex() ) const;
  bool removeRows( int row, int count, const QModelIndex &parent );
  bool setData( const QModelIndex &index, const QVariant &value, int role );

  // Drag & drop support
  Qt::DropActions supportedDropActions() const;
  QStringList mimeTypes() const;
  QMimeData *mimeData(const QModelIndexList &indexes) const;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                    int row, int column, const QModelIndex &par);

  // Restore nodes expanded state after data loading
  void checkNodeForExpand( const TreeItem * item, const QModelIndex &parent );
  void checkAllNodesForExpand();

  // Retrieve text data for indexes
  QStringList getTextForIndexes( QModelIndexList const & idxList ) const;

  // Delete items for indexes
  void removeItemsForIndexes( QModelIndexList const & idxList );

  // Add new folder beside item and return its index
  // or empty index if fail
  QModelIndex addNewFolder( QModelIndex const & idx );

  // Add new headword to given folder
  // return false if it already exists there
  bool addNewHeadword( QString const & path, QString const & headword );

  // Remove headword from given folder
  // return false if failed
  bool removeHeadword( QString const & path, QString const & headword );

  // Return true if headwors is already presented in Favorites
  bool isHeadwordPresent( QString const & path, QString const & headword );

  // Return path in the tree to item
  QString pathToItem( QModelIndex const & idx );

  TreeItem::Type itemType( QModelIndex const & idx )
  { return getItem( idx )->type(); }

  // Export/import Favorites
  void getDataInXml( QByteArray & dataStr );
  void getDataInPlainText( QString & dataStr );
  bool setDataFromXml( QString const & dataStr );

  void saveData();

public slots:
  void itemCollapsed ( const QModelIndex & index );
  void itemExpanded ( const QModelIndex & index );

signals:
  void expandItem( const QModelIndex & index );

protected:
  void readData();
  void addFolder( TreeItem * parent, QDomNode & node );
  void storeFolder( TreeItem * folder, QDomNode & node );

  // Find item in folder
  QModelIndex findItemInFolder( QString const & itemName, int itemType,
                                QModelIndex const & parentIdx );

  TreeItem *getItem( const QModelIndex &index ) const;

  // Find folder with given name or create it if folder not exist
  QModelIndex forceFolder( QString const & name, QModelIndex const & parentIdx );

  // Add headword to given folder
  // return false if such headwordalready exists
  bool addHeadword( QString const & word, QModelIndex const & parentIdx );

  // Return tree level for item
  int level( QModelIndex const & idx );

private:
  QString m_favoritesFilename;
  TreeItem * rootItem;
  QDomDocument dom;
  bool dirty;
};

#define FAVORITES_MIME_TYPE "application/x-goldendict-tree-items"

class FavoritesMimeData : public QMimeData
{
  Q_OBJECT
public:
  FavoritesMimeData() : QMimeData()
  {}

  virtual QStringList formats() const
  { return QStringList( QString::fromLatin1( FAVORITES_MIME_TYPE ) ); }

  virtual bool hasFormat(const QString & mimetype) const
  { return mimetype.compare( QString::fromLatin1( FAVORITES_MIME_TYPE ) ) == 0; }

  void setIndexesList( QModelIndexList const & list )
  { indexes.clear(); indexes = list; }

  QModelIndexList const & getIndexesList() const
  { return indexes; }

private:
  QStringList mimeFormats;
  QModelIndexList indexes;
};

#endif // __FAVORITIESPANEWIDGET_HH__INCLUDED__
