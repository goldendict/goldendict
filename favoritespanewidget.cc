/* This file is (c) 2017 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QDebug>
#include <QApplication>
#include <QDockWidget>
#include <QKeyEvent>
#include <QClipboard>
#ifdef GD_PUGIXML_XSERIAL
#include "pugixml_Qt.h"
#else
#include <QDomDocument>
#include <QDomNode>
#endif
#include <QMessageBox>
#include <QtAlgorithms>
#include <QMap>
#include <QMenu>
#include <QAction>

#include "favoritespanewidget.hh"
#include "delegate.hh"
#include "gddebug.hh"
#include "atomic_rename.hh"

/************************************************** FavoritesPaneWidget *********************************************/

void FavoritesPaneWidget::setUp( Config::Class * cfg, QMenu * menu )
{
    m_cfg = cfg;
    m_favoritesTree = findChild< TreeView * >( "favoritesTree" );
    QDockWidget * favoritesPane = qobject_cast< QDockWidget * >( parentWidget() );
    m_favoritesTree->setHeaderHidden( true );

    // Delete selected items action
    m_deleteSelectedAction = new QAction( this );
    m_deleteSelectedAction->setText( tr( "&Delete Selected" ) );
    m_deleteSelectedAction->setShortcut( QKeySequence( QKeySequence::Delete ) );
    m_deleteSelectedAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    addAction( m_deleteSelectedAction );
    connect( m_deleteSelectedAction, SIGNAL( triggered() ),
             this, SLOT( deleteSelectedItems() ) );

    // Copy selected items to clipboard
    m_copySelectedToClipboard = new QAction( this );
    m_copySelectedToClipboard->setText( tr( "Copy Selected" ) );
    m_copySelectedToClipboard->setShortcut( QKeySequence( QKeySequence::Copy ) );
    m_copySelectedToClipboard->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    addAction( m_copySelectedToClipboard );
    connect( m_copySelectedToClipboard, SIGNAL( triggered() ),
             this, SLOT( copySelectedItems() ) );

    // Add folder to tree view
    m_addFolder = new QAction( this );
    m_addFolder->setText( tr( "Add folder" ) );
    addAction( m_addFolder );
    connect( m_addFolder, SIGNAL( triggered() ), this, SLOT( addFolder() ) );


    // Handle context menu, reusing some of the top-level window's History menu
    m_favoritesMenu = new QMenu( this );
    m_separator = m_favoritesMenu->addSeparator();
    QListIterator< QAction * > actionsIter( menu->actions() );
    while ( actionsIter.hasNext() )
        m_favoritesMenu->addAction( actionsIter.next() );

    // Make the favorites pane's titlebar

    favoritesLabel.setText( tr( "Favorites:" ) );
    favoritesLabel.setObjectName( "favoritesLabel" );
    if ( layoutDirection() == Qt::LeftToRight )
    {
        favoritesLabel.setAlignment( Qt::AlignLeft );
    }
    else
    {
        favoritesLabel.setAlignment( Qt::AlignRight );
    }

    favoritesPaneTitleBarLayout.addWidget( &favoritesLabel );
    favoritesPaneTitleBarLayout.setContentsMargins(5, 5, 5, 5);
    favoritesPaneTitleBar.setLayout( &favoritesPaneTitleBarLayout );
    favoritesPaneTitleBar.setObjectName("favoritesPaneTitleBar");
    favoritesPane->setTitleBarWidget( &favoritesPaneTitleBar );

    // Favorites tree
    m_favoritesModel = new FavoritesModel( Config::getFavoritiesFileName(), this );

    WordListItemDelegate *listItemDelegate = new WordListItemDelegate( m_favoritesTree);

    QAbstractItemModel * oldModel = m_favoritesTree->model();
    m_favoritesTree->setModel( m_favoritesModel );
    if( oldModel )
        oldModel->deleteLater();

    connect( m_favoritesTree, SIGNAL( expanded( QModelIndex ) ),
             m_favoritesModel, SLOT( itemExpanded( QModelIndex ) ) );

    connect( m_favoritesTree, SIGNAL( collapsed( QModelIndex ) ),
             m_favoritesModel, SLOT( itemCollapsed( QModelIndex ) ) );

    connect( m_favoritesModel, SIGNAL( expandItem( QModelIndex) ),
             m_favoritesTree, SLOT( expand( QModelIndex ) ) );

    m_favoritesModel->checkAllNodesForExpand();
    m_favoritesTree->viewport()->setAcceptDrops( true );
    m_favoritesTree->setDragEnabled( true );
    //  m_favoritesTree->setDragDropMode( QAbstractItemView::InternalMove );
    m_favoritesTree->setDragDropMode( QAbstractItemView::DragDrop );
    m_favoritesTree->setDefaultDropAction( Qt::MoveAction );

    m_favoritesTree->setRootIsDecorated( true );

    m_favoritesTree->setContextMenuPolicy( Qt::CustomContextMenu );
    m_favoritesTree->setSelectionMode( QAbstractItemView::ExtendedSelection );

    m_favoritesTree->setEditTriggers( QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed );

    m_favoritesTree->installEventFilter( this );
    m_favoritesTree->viewport()->installEventFilter( this );

    // list selection and keyboard navigation
    connect( m_favoritesTree, SIGNAL( clicked( QModelIndex const & ) ),
             this, SLOT( onItemClicked( QModelIndex const & ) ) );

    connect ( m_favoritesTree->selectionModel(), SIGNAL( selectionChanged ( QItemSelection const & , QItemSelection const & ) ),
              this, SLOT( onSelectionChanged( QItemSelection const & ) ) );

    connect( m_favoritesTree, SIGNAL( customContextMenuRequested( QPoint const & ) ),
             this, SLOT( showCustomMenu( QPoint const & ) ) );
}

FavoritesPaneWidget::~FavoritesPaneWidget()
{
}

bool FavoritesPaneWidget::eventFilter( QObject * obj, QEvent * ev )
{
    // unused for now

    return QWidget::eventFilter( obj, ev );
}

void FavoritesPaneWidget::copySelectedItems()
{
    QModelIndexList selectedIdxs = m_favoritesTree->selectionModel()->selectedIndexes();

    if ( selectedIdxs.isEmpty() )
    {
        // nothing to do
        return;
    }

    QStringList selectedStrings = m_favoritesModel->getTextForIndexes( selectedIdxs );

    QApplication::clipboard()->setText( selectedStrings.join( QString::fromLatin1( "\n" ) ) );
}

void FavoritesPaneWidget::deleteSelectedItems()
{
    QModelIndexList selectedIdxs = m_favoritesTree->selectionModel()->selectedIndexes();

    if ( selectedIdxs.isEmpty() )
    {
        // nothing to do
        return;
    }

    if( m_cfg->preferences.confirmFavoritesDeletion )
    {
        QMessageBox mb( QMessageBox::Warning, "GoldenDict",
                        tr( "All selected items will be deleted. Continue?" ),
                        QMessageBox::Yes | QMessageBox::No );
        mb.exec();
        if( mb.result() != QMessageBox::Yes )
            return;
    }

    m_favoritesModel->removeItemsForIndexes( selectedIdxs );
}

void FavoritesPaneWidget::showCustomMenu(QPoint const & pos)
{
    QModelIndexList selectedIdxs = m_favoritesTree->selectionModel()->selectedIndexes();

    m_favoritesMenu->removeAction( m_copySelectedToClipboard );
    m_favoritesMenu->removeAction( m_deleteSelectedAction );
    m_favoritesMenu->removeAction( m_addFolder );

    m_separator->setVisible( !selectedIdxs.isEmpty() );

    if ( !selectedIdxs.isEmpty() )
    {
        m_favoritesMenu->insertAction( m_separator, m_copySelectedToClipboard );
        m_favoritesMenu->insertAction( m_separator, m_deleteSelectedAction );
    }

    if( selectedIdxs.size() <= 1 )
    {
        m_favoritesMenu->insertAction( m_separator, m_addFolder );
        m_separator->setVisible( true );
    }

    m_favoritesMenu->exec( m_favoritesTree->mapToGlobal( pos ) );
}

void FavoritesPaneWidget::onSelectionChanged( QItemSelection const & selection )
{
    if ( m_favoritesTree->selectionModel()->selectedIndexes().size() != 1
         || selection.indexes().isEmpty() )
        return;

    itemSelectionChanged = true;
    emitFavoritesItemRequested( selection.indexes().front() );
}

void FavoritesPaneWidget::onItemClicked( QModelIndex const & idx )
{
    if ( !itemSelectionChanged && m_favoritesTree->selectionModel()->selectedIndexes().size() == 1 )
    {
        emitFavoritesItemRequested( idx );
    }
    itemSelectionChanged = false;
}

void FavoritesPaneWidget::emitFavoritesItemRequested( QModelIndex const & idx )
{
    if( m_favoritesModel->itemType( idx ) != TreeItem::Word )
    {
        // Item is not headword
        return;
    }

    QString headword = m_favoritesModel->data( idx, Qt::DisplayRole ).toString();
    QString path = m_favoritesModel->pathToItem( idx );

    if( !headword.isEmpty() )
        emit favoritesItemRequested( headword, path );
}

void FavoritesPaneWidget::addFolder()
{
    QModelIndexList selectedIdx = m_favoritesTree->selectionModel()->selectedIndexes();
    if( selectedIdx.size() > 1 )
        return;

    QModelIndex folderIdx;
    if( selectedIdx.size() )
        folderIdx = m_favoritesModel->addNewFolder( selectedIdx.front() );
    else
        folderIdx = m_favoritesModel->addNewFolder( QModelIndex() );

    if( folderIdx.isValid() )
        m_favoritesTree->edit( folderIdx );
}

void FavoritesPaneWidget::addHeadword( QString const & path, QString const & headword )
{
    m_favoritesModel->addNewHeadword( path, headword );
}

bool FavoritesPaneWidget::removeHeadword( QString const & path, QString const & headword )
{
    return m_favoritesModel->removeHeadword( path, headword );
}

bool FavoritesPaneWidget::isHeadwordPresent( const QString & path, const QString & headword )
{
    return m_favoritesModel->isHeadwordPresent( path, headword );
}

void FavoritesPaneWidget::getDataInXml( QByteArray & dataStr )
{
    m_favoritesModel->getDataInXml( dataStr );
}

void FavoritesPaneWidget::getDataInPlainText( QString & dataStr )
{
    m_favoritesModel->getDataInPlainText( dataStr );
}

bool FavoritesPaneWidget::setDataFromXml( QString const & dataStr )
{
    return m_favoritesModel->setDataFromXml( dataStr );
}

void FavoritesPaneWidget::setSaveInterval( unsigned interval )
{
    if( timerId )
    {
        killTimer( timerId );
        timerId = 0;
    }
    if( interval )
    {
        m_favoritesModel->saveData();
        timerId = startTimer( interval * 60000 );
    }
}

void FavoritesPaneWidget::timerEvent( QTimerEvent * ev )
{
    Q_UNUSED( ev )
    m_favoritesModel->saveData();
}

void FavoritesPaneWidget::saveData()
{
    m_favoritesModel->saveData();
}

/************************************************** TreeItem *********************************************/

TreeItem::TreeItem( const QVariant &data, TreeItem *parent, Type type ) :
    itemData( data ),
    parentItem( parent ),
    m_type( type ),
    m_expanded( false )
{
}

TreeItem::~TreeItem()
{
    qDeleteAll( childItems );
}

void TreeItem::clear()
{
    qDeleteAll( childItems );
    childItems.clear();
}

void TreeItem::appendChild( TreeItem *item )
{
    childItems.append( item );
}

void TreeItem::insertChild( int row, TreeItem * item )
{
    if( row > childItems.count() )
        row = childItems.count();
    childItems.insert( row, item );
}

TreeItem *TreeItem::child( int row ) const
{
    return childItems.value( row );
}

void TreeItem::deleteChild( int row )
{
    if( row < 0 || row >= childItems.count() )
        return;

    TreeItem *it = childItems.at( row );
    childItems.removeAt( row );
    delete it;
}

int TreeItem::childCount() const
{
    return childItems.count();
}

QVariant TreeItem::data() const
{
    return itemData;
}

void TreeItem::setData( const QVariant & newData )
{
    itemData = newData;
}

int TreeItem::row() const
{
    if( parentItem )
        return parentItem->childItems.indexOf( const_cast< TreeItem * >( this ) );

    return 0;
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

Qt::ItemFlags TreeItem::flags() const
{
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable |
            Qt::ItemIsDragEnabled;
    if( m_type == Folder )
        f |= Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
    else
        if( m_type == Root )
            f |= Qt::ItemIsDropEnabled;

    return f;
}

QString TreeItem::fullPath() const
{
    // Get full path from root item
    QString path;
    TreeItem * par = parentItem;
    for( ; ; )
    {
        if( !par )
            break;
        path = par->data().toString() + "/" + path;
        par = par->parentItem;
    }
    return path;
}

TreeItem * TreeItem::duplicateItem( TreeItem * newParent ) const
{
    TreeItem * newItem = new TreeItem( itemData, newParent, m_type );
    if( m_type == Folder )
    {
        QList< TreeItem * >::const_iterator it = childItems.begin();
        for( ; it != childItems.end(); ++it )
            newItem->appendChild( (*it)->duplicateItem( newItem ) );
    }
    return newItem;
}

bool TreeItem::haveAncestor( TreeItem * item )
{
    TreeItem *par = parentItem;
    for( ; ; )
    {
        if( !par )
            break;
        if( par == item )
            return true;
        par = par->parent();
    }
    return false;
}

bool TreeItem::haveSameItem( TreeItem * item, bool allowSelf )
{
    QList< TreeItem * >::const_iterator it = childItems.begin();
    QString name = item->data().toString();
    for( ; it != childItems.end(); ++it )
    {
        if( *it == item && !allowSelf )
            return true;
        if( (*it)->data().toString() == name && (*it)->type() == item->type() && (*it) != item )
            return true;
    }

    return false;
}

QStringList TreeItem::getTextFromAllChilds() const
{
    QStringList list;
    QList< TreeItem * >::const_iterator it = childItems.begin();
    for( ; it != childItems.end(); ++it )
    {
        if( (*it)->type() == Word )
        {
            QString txt = (*it)->data().toString();
            list.append( txt );
        }
        else // Folder
        {
            QStringList childList = (*it)->getTextFromAllChilds();
            list.append( childList );
        }
    }
    return list;
}

/************************************************** FavoritesModel *********************************************/

FavoritesModel::FavoritesModel( QString favoritesFilename, QObject * parent ) :
    QAbstractItemModel( parent ),
    m_favoritesFilename( favoritesFilename ),
    rootItem(  new TreeItem( QVariant(), 0, TreeItem::Root ) ),
    dirty( false )
{
    readData();
    dirty = false;
}

FavoritesModel::~FavoritesModel()
{
    if( rootItem )
        delete rootItem;
}

Qt::ItemFlags FavoritesModel::flags( const QModelIndex &idx ) const
{
    TreeItem * item = getItem( idx );
    return item->flags();
}

QVariant FavoritesModel::headerData( int , Qt::Orientation,
                                     int ) const
{
    return QVariant();
}

QModelIndex FavoritesModel::index( int row, int column, const QModelIndex &parentIdx ) const
{
    //  if(!hasIndex(row, column, parent))
    //    return QModelIndex();

    TreeItem *parentItem = getItem( parentIdx );

    TreeItem *childItem = parentItem->child(row);
    if( childItem )
        return createIndex( row, column, childItem );

    return QModelIndex();
}

QModelIndex FavoritesModel::parent( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    TreeItem *childItem = getItem( index );
    if( childItem == rootItem )
        return QModelIndex();

    TreeItem *parentItem = childItem->parent();

    if( parentItem == rootItem )
        return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}

int FavoritesModel::rowCount(const QModelIndex &parent) const
{
    if ( parent.column() > 0 )
        return 0;

    TreeItem *parentItem = getItem( parent );

    return parentItem->childCount();
}

int FavoritesModel::columnCount(const QModelIndex & ) const
{
    return 1;
}

bool FavoritesModel::removeRows( int row, int count, const QModelIndex &parent )
{
    TreeItem * parentItem = getItem( parent );

    beginRemoveRows( parent, row, row + count - 1 );

    for( int i = 0; i < count; i++ )
        parentItem->deleteChild( row );

    endRemoveRows();

    dirty = true;

    return true;
}

bool FavoritesModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
    if( role != Qt::EditRole || !index.isValid() || value.toString().isEmpty() )
        return false;

    QModelIndex parentIdx = parent( index );
    if( findItemInFolder( value.toString(), TreeItem::Folder, parentIdx ).isValid() )
    {
        // Such folder is already presented in parent folder
        return false;
    }

    TreeItem * item = getItem( index );
    item->setData( value );

    dirty = true;

    return true;
}

QVariant FavoritesModel::data( QModelIndex const & index, int role ) const
{
    if( !index.isValid() )
        return QVariant();

    TreeItem *item = getItem( index );
    if( item == rootItem )
        return QVariant();

    if ( role == Qt::DisplayRole || role == Qt::ToolTipRole )
    {
        return item->data();
    }
    else
        if( role == Qt::DecorationRole )
        {
            if( item->type() == TreeItem::Folder || item->type() == TreeItem::Root )
                return QIcon( ":/icons/folder.png" );

            return QVariant();
        }
    if( role == Qt::EditRole )
    {
        if( item->type() == TreeItem::Folder )
            return item->data();

        return QVariant();
    }

    return QVariant();
}

Qt::DropActions FavoritesModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

void FavoritesModel::readData()
{
#ifdef GD_PUGIXML_XSERIAL
    if(!QFile(m_favoritesFilename).exists())
#else
    // Read data from "favorities" file
    QFile favoritesFile( m_favoritesFilename );
    if( !favoritesFile.open( QFile::ReadOnly ) )
#endif
    {
        gdWarning( "No favorities file found" );
        return;
    }

#ifdef GD_PUGIXML_XSERIAL
    pugi::xml_document dom;
    QString tmpFile( m_favoritesFilename);
    if ( !dom.load_file(tmpFile.toLocal8Bit().data()) )
    {
        // Mailformed file
        gdWarning( "Favorites file parsing error.\n");

        QMessageBox mb( QMessageBox::Warning, "GoldenDict",
                        tr( "Error in favorities file" ),
                        QMessageBox::Ok );
        mb.exec();

        renameAtomically( m_favoritesFilename, m_favoritesFilename + ".bak" );
    }

    pugi::xml_node rootNode = dom.document_element();
#else
    QString errorStr;
    int errorLine, errorColumn;
    QDomDocument dom;

    if ( !dom.setContent( &favoritesFile, false, &errorStr, &errorLine, &errorColumn  ) )
    {
        // Mailformed file
        gdWarning( "Favorites file parsing error: %s at %d,%d\n", errorStr.toUtf8().data(),  errorLine,  errorColumn );

        QMessageBox mb( QMessageBox::Warning, "GoldenDict",
                        tr( "Error in favorities file" ),
                        QMessageBox::Ok );
        mb.exec();

        favoritesFile.close();
        renameAtomically( m_favoritesFilename, m_favoritesFilename + ".bak" );
    }
    else
        favoritesFile.close();

    QDomNode rootNode = dom.documentElement();
#endif

    beginResetModel();

    addFolder( rootItem, rootNode );

    endResetModel();
    dirty = false;
}

void FavoritesModel::saveData()
{
    if( !dirty )
        return;

#ifdef GD_PUGIXML_XSERIAL
    pugi::xml_document dom;
    pugi::xml_node el = dom.append_child("Favorites" );
    storeFolder( rootItem, el );

    QString tmpFile( m_favoritesFilename + ".nwx" );
    if ( !dom.save_file(tmpFile.toLocal8Bit().data(), PUGIXML_TEXT(" "), pugi::format_default, pugi::encoding_utf8) )
    {
        gdWarning( "Can't write favorites file, error: %s", tmpFile.errorString().toUtf8().data() );
        return;
    }

    if( renameAtomically(tmpFile, m_favoritesFilename ) )
        dirty = false;
#else
    QFile tmpFile( m_favoritesFilename + ".tmp" );
    if( !tmpFile.open( QFile::WriteOnly ) )
    {
        gdWarning( "Can't write favorites file, error: %s", tmpFile.errorString().toUtf8().data() );
        return;
    }

    QDomDocument dom;

    QDomElement el = dom.createElement( "Favorites" );
    dom.appendChild( el );
    storeFolder( rootItem, el );

    QByteArray result( dom.toByteArray() );

    if ( tmpFile.write( result ) != result.size() )
    {
        tmpFile.close();
        gdWarning( "Can't write favorites file, error: %s", tmpFile.errorString().toUtf8().data() );
        return;
    }

    tmpFile.close();

    if( renameAtomically( tmpFile.fileName(), m_favoritesFilename ) )
        dirty = false;
#endif
}

#ifdef GD_PUGIXML_XSERIAL
void FavoritesModel::addFolder( TreeItem * parent, pugi::xml_node & node )
{
    pugi::xml_node::iterator el = node.begin();
    for( ; el != node.end(); ++el )
    {
        if( 0 == strcmp(el->name(),"folder") )
        {
            // New subfolder
            QString name = QString::fromUtf8(el->attribute( "name" ).value());
            TreeItem *item = new TreeItem( name, parent, TreeItem::Folder );
            item->setExpanded( el->attribute( "expanded").as_bool() );
            parent->appendChild( item );
            addFolder( item, *el );
        }
        else
        {
            QString word = QString::fromUtf8(el->text().get());
            parent->appendChild( new TreeItem( word, parent, TreeItem::Word ) );
        }
    }
    dirty = true;
}

void FavoritesModel::storeFolder( TreeItem * folder, pugi::xml_node & node )
{
    int n = folder->childCount();
    for( int i = 0; i < n; i++ )
    {
        TreeItem * child = folder->child( i );
        QString name = child->data().toString();
        if( child->type() == TreeItem::Folder )
        {
            pugi::xml_node xn = node.append_child("folder");
            xn.append_attribute("name").set_value(name.toUtf8().data());
            xn.append_attribute("expanded").set_value(child->isExpanded() );
            storeFolder( child, xn );
        }
        else
        {
            node.append_child("headword").text().set(name.toUtf8().data());
        }
    }
}
#else
void FavoritesModel::addFolder( TreeItem *parent, QDomNode &node )
{
    QDomNodeList nodes = node.childNodes();
    for( int i = 0; i < nodes.count(); i++ )
    {
        QDomElement el = nodes.at( i ).toElement();
        if( el.nodeName() == "folder" )
        {
            // New subfolder
            QString name = el.attribute( "name", "" );
            TreeItem *item = new TreeItem( name, parent, TreeItem::Folder );
            item->setExpanded( el.attribute( "expanded", "0" ) == "1" );
            parent->appendChild( item );
            addFolder( item, el );
        }
        else
        {
            QString word = el.text();
            parent->appendChild( new TreeItem( word, parent, TreeItem::Word ) );
        }
    }
    dirty = true;
}

void FavoritesModel::storeFolder( TreeItem * folder, QDomNode & node )
{
    QDomDocument dom = node.ownerDocument();
    int n = folder->childCount();
    for( int i = 0; i < n; i++ )
    {
        TreeItem * child = folder->child( i );
        QString name = child->data().toString();
        if( child->type() == TreeItem::Folder )
        {
            QDomElement el = dom.createElement( "folder" );
            el.setAttribute( "name", name );
            el.setAttribute( "expanded", child->isExpanded() ? "1" : "0" );
            node.appendChild( el );
            storeFolder( child, el );
        }
        else
        {
            QDomElement el = dom.createElement( "headword" );
            el.appendChild( dom.createTextNode( name ) );
            node.appendChild( el );
        }
    }
}
#endif

void FavoritesModel::itemExpanded( const QModelIndex & index )
{
    if( index.isValid() )
    {
        TreeItem *item = getItem( index );
        item->setExpanded( true );
    }
}

void FavoritesModel::itemCollapsed( const QModelIndex & index )
{
    if( index.isValid() )
    {
        TreeItem *item = getItem( index );
        item->setExpanded( false );
    }
}

void FavoritesModel::checkAllNodesForExpand()
{
    checkNodeForExpand( rootItem, QModelIndex() );
}

void FavoritesModel::checkNodeForExpand( const TreeItem * item, const QModelIndex & parent )
{
    for( int i = 0; i < item->childCount(); i++ )
    {
        TreeItem * ch = item->child( i );
        if( ch->type() == TreeItem::Folder && ch->isExpanded() )
        {
            // We need to expand this node...
            QModelIndex idx = index( i, 0, parent );
            emit expandItem( idx );

            // ...and check it for children nodes
            checkNodeForExpand( item->child( i ), idx );
        }
    }
}

QStringList FavoritesModel::mimeTypes() const
{
    return QStringList( QString::fromLatin1( FAVORITES_MIME_TYPE ) );
}

QMimeData *FavoritesModel::mimeData( const QModelIndexList & indexes ) const
{
    FavoritesMimeData *data = new FavoritesMimeData();
    data->setIndexesList( indexes );
    return data;
}

bool FavoritesModel::dropMimeData( const QMimeData *data, Qt::DropAction action,
                                   int row, int, const QModelIndex &par )
{
    if( action == Qt::MoveAction || action == Qt::CopyAction )
    {
        if( data->hasFormat( FAVORITES_MIME_TYPE ) )
        {
            FavoritesMimeData const * mimeData = qobject_cast< FavoritesMimeData const * >( data );
            if( mimeData )
            {
                QModelIndexList const & list = mimeData->getIndexesList();

                if( list.isEmpty() )
                    return false;

                TreeItem * parentItem = getItem( par );
                QModelIndex parentIdx = par;

                if( row < 0 )
                    row = 0;

                QList< QModelIndex >::const_iterator it = list.begin();
                QList< TreeItem * > movedItems;
                for( ; it != list.end(); ++it )
                {
                    TreeItem * item = getItem( *it );

                    // Check if we can copy/move this item
                    if( parentItem->haveAncestor( item ) || parentItem->haveSameItem( item, action == Qt::MoveAction ) )
                        return false;

                    movedItems.append( item );
                }

                // Insert items to new place

                beginInsertRows( parentIdx, row, row + movedItems.count() - 1 );
                for( int i = 0; i < movedItems.count(); i++ )
                {
                    TreeItem * item = movedItems.at( i );
                    TreeItem *newItem = item->duplicateItem( parentItem );
                    parentItem->insertChild( row + i, newItem );
                }
                endInsertRows();

                dirty = true;

                return true;
            }
        }
    }
    return false;
}

QModelIndex FavoritesModel::findItemInFolder( const QString & itemName, int itemType,
                                              const QModelIndex & parentIdx )
{
    TreeItem * parentItem = getItem( parentIdx );
    if(parentItem)
    {
        for( int i = 0; i < parentItem->childCount(); i++ )
        {
            TreeItem * item = parentItem->child( i );
            if( item->data().toString() == itemName && item->type() == itemType )
                return createIndex( i, 0, item );
        }
    }
    return QModelIndex();
}

TreeItem *FavoritesModel::getItem( const QModelIndex &index ) const
{
    if( index.isValid() )
    {
        TreeItem *item = static_cast< TreeItem * >( index.internalPointer() );
        if (item)
            return item;
    }
    return rootItem;
}

QStringList FavoritesModel::getTextForIndexes( const QModelIndexList & idxList ) const
{
    QStringList list;
    QModelIndexList::const_iterator it = idxList.begin();
    for( ; it != idxList.end(); ++it )
    {
        TreeItem *item = getItem( *it );
        if( item->type() == TreeItem::Word )
            list.append( item->data().toString() );
        else
            list.append( item->getTextFromAllChilds() );
    }
    return list;
}

void FavoritesModel::removeItemsForIndexes( const QModelIndexList & idxList )
{
    // We should delete items from lowest tree level and in decreasing order
    // so that first deletions won't affect the indexes for subsequent deletions.

    QMap< int, QModelIndexList > itemsToDelete;
    int lowestLevel = 0;

    QModelIndexList::const_iterator it = idxList.begin();
    for( ; it != idxList.end(); ++it )
    {
        int n = level( *it );
        if( n > lowestLevel )
            lowestLevel = n;
        itemsToDelete[ n ].append( *it );
    }

    for( int i = lowestLevel; i >= 0; i-- )
    {
        QModelIndexList idxSublist = itemsToDelete[ i ];
        qSort( idxSublist.begin(), idxSublist.end(), qGreater< QModelIndex >() );

        it = idxSublist.begin();
        for( ; it != idxSublist.end(); ++it )
        {
            QModelIndex parentIdx = parent( *it );
            removeRows( (*it).row(), 1, parentIdx );
        }
    }
}

QModelIndex FavoritesModel::addNewFolder( const QModelIndex & idx )
{
    QModelIndex parentIdx;
    if( idx.isValid() )
        parentIdx = parent( idx );
    else
        parentIdx = idx;

    QString baseName = QString::fromLatin1( "New folder" );

    // Create unique name

    QString name = baseName;
    if( findItemInFolder( name, TreeItem::Folder, parentIdx ).isValid() )
    {
        int i;
        for( i = 1; i < 1000; i++ )
        {
            name = baseName + QString::number( i );
            if( !findItemInFolder( name, TreeItem::Folder, parentIdx ).isValid() )
                break;
        }
        if( i >= 1000 )
            return QModelIndex();
    }

    // Create folder with unique name

    TreeItem *parentItem = getItem( parentIdx );
    int row;

    if( idx.isValid() )
    {
        // Insert after selected element
        row = idx.row() + 1;
    }
    else
    {
        // No selected element - add to end of root folder
        row = parentItem->childCount();
    }

    beginInsertRows( parentIdx, row, row );
    TreeItem * newFolder = new TreeItem( name, parentItem, TreeItem::Folder );
    parentItem->insertChild( row, newFolder );
    endInsertRows();

    dirty = true;

    return createIndex( row, 0, newFolder );
}

bool FavoritesModel::addNewHeadword( const QString & path, const QString & headword )
{
    QModelIndex parentIdx;

    // Find or create target folder

    QStringList folders = path.split( "/", QString::SkipEmptyParts );
    QStringList::const_iterator it = folders.begin();
    for( ; it != folders.end(); ++it )
        parentIdx = forceFolder( *it, parentIdx );

    // Add headword

    return addHeadword( headword, parentIdx );
}

bool FavoritesModel::removeHeadword( const QString & path, const QString & headword )
{
    QModelIndex idx;

    // Find target folder

    QStringList folders = path.split( "/", QString::SkipEmptyParts );
    QStringList::const_iterator it = folders.begin();
    for( ; it != folders.end(); ++it )
    {
        idx = findItemInFolder( *it, TreeItem::Folder, idx );
        if( !idx.isValid() )
            break;
    }

    if( path.isEmpty() || idx.isValid() )
    {
        idx = findItemInFolder( headword, TreeItem::Word, idx );
        if( idx.isValid() )
        {
            QModelIndexList list;
            list.append( idx );
            removeItemsForIndexes( list );
            return true;
        }
    }

    return false;
}

bool FavoritesModel::isHeadwordPresent( const QString & path, const QString & headword )
{
    QModelIndex idx;

    // Find target folder

    QStringList folders = path.split( "/", QString::SkipEmptyParts );
    QStringList::const_iterator it = folders.begin();
    for( ; it != folders.end(); ++it )
    {
        idx = findItemInFolder( *it, TreeItem::Folder, idx );
        if( !idx.isValid() )
            break;
    }

    if( path.isEmpty() || idx.isValid() )
    {
        idx = findItemInFolder( headword, TreeItem::Word, idx );
        return idx.isValid();
    }

    return false;
}

QModelIndex FavoritesModel::forceFolder( QString const & name, const QModelIndex & parentIdx )
{
    QModelIndex idx = findItemInFolder( name, TreeItem::Folder, parentIdx );
    if( idx.isValid() )
        return idx;

    // Folder not found, create it
    TreeItem * parentItem = getItem( parentIdx );
    TreeItem * newItem = new TreeItem( name, parentItem, TreeItem::Folder );
    int row = parentItem->childCount();

    beginInsertRows( parentIdx, row, row );
    parentItem->appendChild( newItem );
    endInsertRows();

    dirty = true;

    return createIndex( row, 0, newItem );
}

bool FavoritesModel::addHeadword( const QString & word, const QModelIndex & parentIdx )
{
    QModelIndex idx = findItemInFolder( word, TreeItem::Word, parentIdx );
    if( idx.isValid() )
        return false;

    // Headword not found, append it
    TreeItem * parentItem = getItem( parentIdx );
    TreeItem * newItem = new TreeItem( word, parentItem, TreeItem::Word );
    int row = parentItem->childCount();

    beginInsertRows( parentIdx, row, row );
    parentItem->appendChild( newItem );
    endInsertRows();

    dirty = true;

    return true;
}

int FavoritesModel::level( QModelIndex const & idx )
{
    int n = 0;
    QModelIndex parentIdx = parent( idx );
    while( parentIdx.isValid() )
    {
        n++;
        parentIdx = parent( parentIdx );
    }
    return n;
}

QString FavoritesModel::pathToItem( QModelIndex const & idx )
{
    QString path;
    QModelIndex parentIdx = parent( idx );
    while( parentIdx.isValid() )
    {
        if( !path.isEmpty() )
            path = "/" + path;

        path = data( parentIdx, Qt::DisplayRole ).toString() + path;

        parentIdx = parent( parentIdx );
    }
    return path;
}

void FavoritesModel::getDataInXml( QByteArray & dataStr )
{
#ifdef GD_PUGIXML_XSERIAL
    pugi::xml_document dom;
    pugi::xml_node el = dom.append_child("Favorites");
    storeFolder( rootItem, el );
    dataStr.clear();
    xml_writer_bytearray xwb(dataStr);
    dom.save(xwb);
#else
    QDomDocument dom;

    QDomElement el = dom.createElement( "Favorites" );
    dom.appendChild( el );
    storeFolder( rootItem, el );

    dataStr = dom.toByteArray();
#endif
}

void FavoritesModel::getDataInPlainText( QString & dataStr )
{
    QModelIndexList list;
    list.append( QModelIndex() );
    dataStr = getTextForIndexes( list ).join( QString::fromLatin1( "\n" ) );
}

bool FavoritesModel::setDataFromXml( QString const & dataStr )
{
#ifdef GD_PUGIXML_XSERIAL
    pugi::xml_document dom;
    if ( !dom.load_string(dataStr.toUtf8().data()) )
    {
        // Mailformed data
        gdWarning( "XML parsing error.\n");
        return false;
    }
    pugi::xml_node rootNode = dom.document_element();

    beginResetModel();

    rootItem->clear();

    addFolder( rootItem, rootNode );

    endResetModel();
#else
    QString errorStr;
    int errorLine, errorColumn;
    QDomDocument dom;

    if ( !dom.setContent( dataStr, false, &errorStr, &errorLine, &errorColumn  ) )
    {
        // Mailformed data
        gdWarning( "XML parsing error: %s at %d,%d\n", errorStr.toUtf8().data(),  errorLine,  errorColumn );
        return false;
    }

    beginResetModel();

    rootItem->clear();

    QDomNode rootNode = dom.documentElement();
    addFolder( rootItem, rootNode );

    endResetModel();
#endif
    dirty = true;
    return true;
}
