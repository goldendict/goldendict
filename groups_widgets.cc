/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "groups_widgets.hh"

#include "instances.hh"
#include "config.hh"
#include "langcoder.hh"
#include "language.hh"
#include "fsencoding.hh"

//#include "initializing.hh"

#include <QMenu>
#include <QDir>
#include <QIcon>
#include <QMap>
#include <QVector>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

using std::vector;

/// DictGroupWidget

DictGroupWidget::DictGroupWidget( QWidget * parent,
                                  vector< sptr< Dictionary::Class > > const & dicts,
                                  Config::Group const & group ):
  QWidget( parent ),
  groupId( group.id )
{
  ui.setupUi( this );
  ui.dictionaries->populate( Instances::Group( group, dicts, Config::Group() ).dictionaries, dicts );

  // Populate icons' list

  QStringList icons = QDir( ":/flags/" ).entryList( QDir::Files, QDir::NoSort );

  ui.groupIcon->addItem( tr( "None" ), "" );

  bool usesIconData = !group.iconData.isEmpty();

  if ( !usesIconData )
    ui.groupIcon->addItem( tr( "From file..." ), "" );
  else
    ui.groupIcon->addItem( Instances::iconFromData( group.iconData ), group.icon, group.icon );

  for( int x = 0; x < icons.size(); ++x )
  {
    QString n( icons[ x ] );
    n.chop( 4 );
    n[ 0 ] = n[ 0 ].toUpper();

    ui.groupIcon->addItem( QIcon( ":/flags/" + icons[ x ] ), n, icons[ x ] );

    if ( !usesIconData && icons[ x ] == group.icon )
      ui.groupIcon->setCurrentIndex( x + 2 );
  }

  if ( usesIconData )
    ui.groupIcon->setCurrentIndex( 1 );

  ui.shortcut->setHotKey( Config::HotKey( group.shortcut ) );

  ui.favoritesFolder->setText( group.favoritesFolder );

  connect( ui.groupIcon, SIGNAL(activated(int)),this,SLOT(groupIconActivated(int)),
           Qt::QueuedConnection );

  ui.dictionaries->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( ui.dictionaries, SIGNAL( customContextMenuRequested( QPoint ) ),
           this, SLOT( showDictInfo( QPoint ) ) );

  connect( ui.dictionaries, SIGNAL( doubleClicked( QModelIndex ) ),
           this, SLOT( removeCurrentItem( QModelIndex ) ) );
}

void DictGroupWidget::groupIconActivated( int index )
{
  if ( index == 1 )
  {
    QList< QByteArray > supImageFormats = QImageReader::supportedImageFormats();

    QString formatList( " (" );

    for( int x = 0; x < supImageFormats.size(); ++x )
      formatList += "*." + QString::fromLatin1( supImageFormats[ x ] ) + " ";

    formatList.chop( 1 );
    formatList.append( ")" );

    QString chosenFile =
        QFileDialog::getOpenFileName( this, tr( "Choose a file to use as group icon" ),
                                      QString(),
                                      tr( "Images" ) + formatList + ";;" +
                                      tr( "All files" ) + " (*.*)" );

    if ( !chosenFile.isEmpty() )
    {
      QIcon icon( chosenFile );

      if ( icon.isNull() )
        QMessageBox::critical( this, tr( "Error" ), tr( "Can't read the specified image file." ) );
      else
      {
        ui.groupIcon->setItemIcon( 1, icon );

        QString baseName = QFileInfo( chosenFile ).completeBaseName();
        ui.groupIcon->setItemText( 1, baseName );
        ui.groupIcon->setItemData( 1, baseName );
      }
    }
  }
}

Config::Group DictGroupWidget::makeGroup() const
{
  Instances::Group g( "" );

  g.id = groupId;

  g.dictionaries = ui.dictionaries->getCurrentDictionaries();

  int currentIndex = ui.groupIcon->currentIndex();

  if ( currentIndex == 1 ) // File
    g.iconData = ui.groupIcon->itemIcon( currentIndex );

  g.icon = ui.groupIcon->itemData( currentIndex ).toString();

  g.shortcut = ui.shortcut->getHotKey().toKeySequence();

  g.favoritesFolder = ui.favoritesFolder->text().replace( '\\', '/' );

  return g.makeConfigGroup();
}

void DictGroupWidget::showDictInfo( QPoint const & pos )
{
  QVariant data = ui.dictionaries->getModel()->data( ui.dictionaries->indexAt( pos ), Qt::EditRole );
  QString id;
  if( data.canConvert< QString >() )
    id = data.toString();

  if( !id.isEmpty() )
  {
    vector< sptr< Dictionary::Class > > const & dicts = ui.dictionaries->getCurrentDictionaries();
    unsigned n;
    for( n = 0; n < dicts.size(); n++ )
      if( id.compare( QString::fromUtf8( dicts.at( n )->getId().c_str() ) ) == 0 )
        break;
    if( n < dicts.size() )
      emit showDictionaryInfo( id );
  }
}

void DictGroupWidget::removeCurrentItem( QModelIndex const & index )
{
  (void)index;
  ui.dictionaries->getModel()->removeSelectedRows( ui.dictionaries->selectionModel() );
}

/// DictListModel

void DictListModel::populate(
  std::vector< sptr< Dictionary::Class > > const & active,
  std::vector< sptr< Dictionary::Class > > const & available )
{
  dictionaries = active;
  allDicts = &available;

  beginResetModel();
  endResetModel();
}

void DictListModel::populate(
  std::vector< sptr< Dictionary::Class > > const & active )
{
  dictionaries = active;
  beginResetModel();
  endResetModel();
}

void DictListModel::setAsSource()
{
  isSource = true;
}

std::vector< sptr< Dictionary::Class > > const &
  DictListModel::getCurrentDictionaries() const
{
  return dictionaries;
}

Qt::ItemFlags DictListModel::flags( QModelIndex const & index ) const
{
  Qt::ItemFlags defaultFlags = QAbstractListModel::flags( index );

  if (index.isValid())
     return Qt::ItemIsDragEnabled | defaultFlags;
  else
     return Qt::ItemIsDropEnabled | defaultFlags;
}

int DictListModel::rowCount( QModelIndex const & ) const
{
  return dictionaries.size();
}

QVariant DictListModel::data( QModelIndex const & index, int role ) const
{
  if( index.row() < 0 )
    return QVariant();
  
  sptr< Dictionary::Class > const & item = dictionaries[ index.row() ];

  if ( !item )
    return QVariant();

  switch ( role )
  {
    case Qt::ToolTipRole:
    {
      QString tt = "<b>" + QString::fromUtf8( item->getName().c_str() ) + "</b>";

      QString lfrom( Language::localizedNameForId( item->getLangFrom() ) );
      QString lto( Language::localizedNameForId( item->getLangTo() ) );
      if ( !lfrom.isEmpty() )
      {
        if ( lfrom == lto )
          tt += "<br>" + lfrom;
        else
          tt += "<br>" + lfrom + " - " + lto;
      }

      int entries = item->getArticleCount();
      if ( !entries )
        entries = item->getWordCount();
      if ( entries )
        tt += "<br>" + tr( "%1 entries" ).arg( entries );

      const std::vector< std::string > & dirs = item->getDictionaryFilenames();

      if ( dirs.size() )
      {
        tt += "<hr>";
        tt += FsEncoding::decode( dirs.at( 0 ).c_str() );
      }

      tt.replace( " ", "&nbsp;" );
      return tt;
    }

    case Qt::DisplayRole :
      return QString::fromUtf8( item->getName().c_str() );

    case Qt::EditRole :
      return QString::fromUtf8( item->getId().c_str() );

    case Qt::DecorationRole:
      // make all icons of the same size to avoid visual size/alignment problems
      return item->getIcon().pixmap( 32 ).scaledToHeight( 21, Qt::SmoothTransformation );

    default:;
  }

  return QVariant();
}

bool DictListModel::insertRows( int row, int count, const QModelIndex & parent )
{
  if ( isSource )
    return false;

  beginInsertRows( parent, row, row + count - 1 );
  dictionaries.insert( dictionaries.begin() + row, count,
                       sptr< Dictionary::Class >() );
  endInsertRows();
  emit contentChanged();
  return true;
}

void DictListModel::addRow(const QModelIndex & parent, sptr< Dictionary::Class > dict)
{
  for (unsigned i = 0; i < dictionaries.size(); i++)
  {
    if (dictionaries[i]->getId() == dict->getId())
      return;
  }

  beginInsertRows( parent, dictionaries.size(), dictionaries.size()+1 );
  dictionaries.push_back(dict);
  endInsertRows();
  emit contentChanged();
}

bool DictListModel::removeRows( int row, int count,
                                const QModelIndex & parent )
{
  if ( isSource )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );
  dictionaries.erase( dictionaries.begin() + row,
                      dictionaries.begin() + row + count );
  endRemoveRows();
  emit contentChanged();
  return true;
}

bool DictListModel::setData( QModelIndex const & index, const QVariant & value,
                             int role )
{
  if ( isSource || !allDicts || !index.isValid() ||
       index.row() >= (int)dictionaries.size() )
    return false;

  if ( ( role == Qt::DisplayRole ) || ( role ==  Qt::DecorationRole ) )
  {
    // Allow changing that, but do nothing
    return true;
  }

  if ( role == Qt::EditRole )
  {
    Config::Group g;

    g.dictionaries.push_back( Config::DictionaryRef( value.toString(), QString() ) );

    Instances::Group i( g, *allDicts, Config::Group() );

    if ( i.dictionaries.size() == 1 )
    {
      // Found that dictionary
      dictionaries[ index.row() ] = i.dictionaries.front();

      emit dataChanged( index, index );

      return true;
    }
  }

  return false;
}

Qt::DropActions DictListModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

void DictListModel::removeSelectedRows( QItemSelectionModel * source )
{
  if ( !source )
    return;

  QModelIndexList rows = source->selectedRows();

  if ( !rows.count() )
    return;

  for ( int i = rows.count()-1; i >= 0; --i )
  {
    dictionaries.erase( dictionaries.begin() + rows.at( i ).row() );
  }

  beginResetModel();
  endResetModel();
  emit contentChanged();
}

void DictListModel::addSelectedUniqueFromModel( QItemSelectionModel * source )
{
  if ( !source )
    return;

  QModelIndexList rows = source->selectedRows();

  if ( !rows.count() )
    return;

  const QSortFilterProxyModel * proxyModel = dynamic_cast< const QSortFilterProxyModel * > ( source->model() );

  const DictListModel * baseModel;

  if ( proxyModel )
  {
    baseModel = dynamic_cast< const DictListModel * > ( proxyModel->sourceModel() );
  }
  else
  {
    baseModel = dynamic_cast< const DictListModel * > ( source->model() );
  }

  if ( !baseModel )
    return;

  QVector< std::string > list;
  QVector< std::string > dicts;
  for ( unsigned i = 0; i < dictionaries.size(); i++ )
    dicts.append( dictionaries.at( i )->getId() );

  for ( int i = 0; i < rows.count(); i++ )
  {
    QModelIndex idx = proxyModel ? proxyModel->mapToSource(rows.at( i )) : rows.at( i );
    std::string id = baseModel->dictionaries.at( idx.row() )->getId();

    if ( !dicts.contains( id ) )
      list.append( id );
  }

  if ( list.empty() )
    return;

  for ( int j = 0; j < list.size(); j++ )
  {
    for ( unsigned i = 0; i < allDicts->size(); i++ )
    {
      if ( allDicts->at( i )->getId() == list.at( j ) )
      {
        dictionaries.push_back( allDicts->at( i ) );
        break;
      }
    }
  }

  beginResetModel();
  endResetModel();
  emit contentChanged();
}

void DictListModel::filterDuplicates()
{
  QSet< QString > ids;
  bool doReset = false;

  for ( unsigned i = 0; i < dictionaries.size(); i++ )
  {
    QString id = QString::fromStdString( dictionaries.at( i )->getId() );

    if ( ids.contains( id ) )
    {
      dictionaries.erase( dictionaries.begin() + i-- );
      doReset = true;
      continue;
    }

    ids.insert( id );
  }

  if ( doReset )
  {
    beginResetModel();
    endResetModel();
    emit contentChanged();
  }
}

/// DictListWidget

DictListWidget::DictListWidget( QWidget * parent ): QListView( parent ),
  model( this )
{
  setModel( &model );

  setSelectionMode( ExtendedSelection );

  setDragEnabled( true );
  setAcceptDrops( true );
  setDropIndicatorShown( true );
}

DictListWidget::~DictListWidget()
{
  setModel( 0 );
}

void DictListWidget::populate(
  std::vector< sptr< Dictionary::Class > > const & active,
  std::vector< sptr< Dictionary::Class > > const & available )
{
  model.populate( active, available );
}

void DictListWidget::populate(
  std::vector< sptr< Dictionary::Class > > const & active )
{
  model.populate( active );
}

void DictListWidget::setAsSource()
{
  setDropIndicatorShown( false );
  model.setAsSource();
}

std::vector< sptr< Dictionary::Class > > const &
  DictListWidget::getCurrentDictionaries() const
{
  return model.getCurrentDictionaries();
}

void DictListWidget::dropEvent( QDropEvent * event )
{
  DictListWidget * sourceList = dynamic_cast< DictListWidget * > ( event->source() );

  QListView::dropEvent( event );

  if ( sourceList != this )
  {
    model.filterDuplicates();
  }
}

void DictListWidget::focusInEvent( QFocusEvent * )
{
  emit gotFocus();
}

void DictListWidget::rowsInserted( QModelIndex const & parent, int start, int end )
{
  QListView::rowsInserted( parent, start, end );

  // When inserting new rows, make the first of them current
  selectionModel()->setCurrentIndex( model.index( start, 0, parent ),
                                     QItemSelectionModel::NoUpdate );
}

void DictListWidget::rowsAboutToBeRemoved( QModelIndex const & parent, int start, int end )
{
  // When removing rows, if the current row is among the removed ones, select
  // an item just before the first row to be removed, if there's one.
  QModelIndex current = currentIndex();

  if ( current.isValid() && current.row() &&
       current.row() >= start && current.row() <= end )
    selectionModel()->setCurrentIndex( model.index( current.row() - 1, 0, parent ),
                                       QItemSelectionModel::NoUpdate );

  QListView::rowsAboutToBeRemoved( parent, start, end );
}


// DictGroupsWidget

DictGroupsWidget::DictGroupsWidget( QWidget * parent ):
  QTabWidget( parent ), nextId( 1 ), allDicts( 0 ), activeDicts( 0 )
{
  setMovable( true );
  setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, SIGNAL( customContextMenuRequested( QPoint ) ),
           this, SLOT( contextMenu( QPoint ) ) );
}

namespace {

QString escapeAmps( QString const & str )
{
  QString result( str );
  result.replace( "&", "&&" );
  return result;
}

QString unescapeAmps( QString const & str )
{
  QString result( str );
  result.replace( "&&", "&" );
  return result;
}

}

void DictGroupsWidget::populate( Config::Groups const & groups,
                                 vector< sptr< Dictionary::Class > > const & allDicts_,
                                 vector< sptr< Dictionary::Class > > const & activeDicts_ )
{
  while( count() )
    removeCurrentGroup();

  allDicts = &allDicts_;
  activeDicts = &activeDicts_;

  for( int x = 0; x < groups.size(); ++x )
  {
    DictGroupWidget *gr = new DictGroupWidget( this, *allDicts, groups[ x ] );
    addTab( gr, escapeAmps( groups[ x ].name ) );
    connect( gr, SIGNAL( showDictionaryInfo( QString const & ) ),
             this, SIGNAL( showDictionaryInfo( QString const & ) ) );
    connect( gr->getModel(), SIGNAL( contentChanged() ), this, SLOT( tabDataChanged() ) );

    setCurrentIndex( x );
    QString toolTipStr = "\"" + tabText( x ) + "\"\n" + tr( "Dictionaries: " )
                         + QString::number( getCurrentModel()->getCurrentDictionaries().size() );
    setTabToolTip( x, toolTipStr );
  }

  nextId = groups.nextId;

  setCurrentIndex( 0 );

  setUsesScrollButtons( count() > 3 );
}

/// Creates groups from what is currently set up
Config::Groups DictGroupsWidget::makeGroups() const
{
  Config::Groups result;

  result.nextId = nextId;

  for( int x = 0; x < count(); ++x )
  {
    result.push_back( dynamic_cast< DictGroupWidget & >( *widget( x ) ).makeGroup() );
    result.back().name = unescapeAmps( tabText( x ) );
  }

  return result;
}

DictListModel * DictGroupsWidget::getCurrentModel() const
{
  int current = currentIndex();

  if ( current >= 0 )
  {
    DictGroupWidget * w = ( DictGroupWidget * ) widget( current );
    return w->getModel();
  }

  return 0;
}

QItemSelectionModel * DictGroupsWidget::getCurrentSelectionModel() const
{
  int current = currentIndex();

  if ( current >= 0 )
  {
    DictGroupWidget * w = ( DictGroupWidget * ) widget( current );
    return w->getSelectionModel();
  }

  return 0;
}


void DictGroupsWidget::addNewGroup( QString const & name )
{
  if ( !allDicts )
    return;

  int idx = currentIndex() + 1;

  Config::Group newGroup;

  newGroup.id = nextId++;

  DictGroupWidget *gr = new DictGroupWidget( this, *allDicts, newGroup );
  insertTab( idx, gr, escapeAmps( name ) );
  connect( gr, SIGNAL( showDictionaryInfo( QString const & ) ),
           this, SIGNAL( showDictionaryInfo( QString const & ) ) );

  setCurrentIndex( idx );

  connect( gr->getModel(), SIGNAL( contentChanged() ), this, SLOT( tabDataChanged() ) );

  QString toolTipStr = "\"" + tabText( idx ) + "\"\n" + tr( "Dictionaries: " )
                       + QString::number( getCurrentModel()->getCurrentDictionaries().size() );
  setTabToolTip( idx, toolTipStr );

  setUsesScrollButtons( count() > 3 );
}

int DictGroupsWidget::addUniqueGroup( const QString & name )
{
  for( int n = 0; n < count(); n++ )
    if( tabText( n ) == name )
    {
      setCurrentIndex( n );
      return n;
    }

  addNewGroup( name );
  return currentIndex();
}

void DictGroupsWidget::addAutoGroups()
{
  if( !activeDicts )
    return;

  if ( QMessageBox::information( this, tr( "Confirmation" ),
         tr( "Are you sure you want to generate a set of groups "
             "based on language pairs?" ), QMessageBox::Yes,
             QMessageBox::Cancel ) != QMessageBox::Yes )
    return;

  QMap< QString, QVector< sptr<Dictionary::Class> > > dictMap;
  QMap< QString, QVector< sptr<Dictionary::Class> > > morphoMap;

  // Put active dictionaries into lists

  for ( unsigned i = 0; i < activeDicts->size(); i++ )
  {
    sptr<Dictionary::Class> dict = activeDicts->at( i );

    int idFrom = dict->getLangFrom();
    int idTo = dict->getLangTo();
    if( idFrom == 0)
    {
      // Attempt to find language pair in dictionary name

      QPair<quint32,quint32> ids = LangCoder::findIdsForName( QString::fromUtf8( dict->getName().c_str() ) );
      idFrom = ids.first;
      idTo = ids.second;
    }

    QString name( tr( "Unassigned" ) );
    if ( idFrom != 0 && idTo != 0 )
    {
      QString lfrom = LangCoder::intToCode2( idFrom );
      QString lto = LangCoder::intToCode2( idTo );
      lfrom[ 0 ] = lfrom[ 0 ].toTitleCase();
      lto[ 0 ] = lto[ 0 ].toTitleCase();
      name = lfrom + " - " + lto;
    }
    else if( !dict->getDictionaryFilenames().empty() )
    {
      // Handle special case - morphology dictionaries

      QString fileName = QFileInfo( FsEncoding::decode( dict->getDictionaryFilenames()[ 0 ].c_str() ) ).fileName();
      if( fileName.endsWith( ".aff", Qt::CaseInsensitive ) )
      {
        QString code = fileName.left( 2 ).toLower();
        QVector<sptr<Dictionary::Class> > vd = morphoMap[ code ];
        vd.append( dict );
        morphoMap[ code ] = vd;
        continue;
      }
    }

    QVector<sptr<Dictionary::Class> > vd = dictMap[ name ];
    vd.append( dict );
    dictMap[ name ] = vd;
  }

  QStringList groupList = dictMap.uniqueKeys();
  QStringList morphoList = morphoMap.uniqueKeys();

  // Insert morphology dictionaries into corresponding lists

  for( QStringList::ConstIterator ln = morphoList.begin(); ln != morphoList.end(); ++ln )
  {
    for( QStringList::ConstIterator gr = groupList.begin(); gr != groupList.end(); ++gr )
      if( ln->compare( gr->left( 2 ), Qt::CaseInsensitive ) == 0 )
      {
        QVector<sptr<Dictionary::Class> > vdg = dictMap[ *gr ];
        vdg += morphoMap[ *ln ];
        dictMap[ *gr ] = vdg;
      }
  }

  // Make groups

  for( QStringList::ConstIterator gr = groupList.begin(); gr != groupList.end(); ++gr )
  {
    if( count() )
      setCurrentIndex( count() - 1 );

    addUniqueGroup( *gr );

    // add dictionaries into the current group
    QVector< sptr<Dictionary::Class> > vd = dictMap[ *gr ];
    DictListModel *model = getCurrentModel();
    for( int i = 0; i < vd.count(); i++ )
      model->addRow(QModelIndex(), vd.at( i ) );
  }
}

QString DictGroupsWidget::getCurrentGroupName() const
{
  int current = currentIndex();

  if ( current >= 0 )
    return unescapeAmps( tabText( current ) );

  return QString();
}

void DictGroupsWidget::renameCurrentGroup( QString const & name )
{
  int current = currentIndex();

  if ( current >= 0 )
    setTabText( current, escapeAmps( name ) );
}

void DictGroupsWidget::removeCurrentGroup()
{
  int current = currentIndex();

  if ( current >= 0 )
  {
    QWidget * w = widget( current );
    removeTab( current );
    delete w;
  }

  setUsesScrollButtons( count() > 3 );
}

void DictGroupsWidget::removeAllGroups()
{
  while ( count() )
  {
    QWidget * w = widget( 0 );
    removeTab( 0 );
    delete w;
  }
}

void DictGroupsWidget::combineGroups( int source, int target )
{
  if( source < 0 || source >= count() || target < 0 || target >= count() )
    return;

  setCurrentIndex( source );
  vector< sptr< Dictionary::Class > > const & dicts = getCurrentModel()->getCurrentDictionaries();

  setCurrentIndex( target );
  DictListModel *model = getCurrentModel();

  disconnect( model, SIGNAL( contentChanged() ), this, SLOT( tabDataChanged() ) );

  for( unsigned i = 0; i < dicts.size(); i++ )
    model->addRow( QModelIndex(), dicts[ i ] );

  connect( model, SIGNAL( contentChanged() ), this, SLOT( tabDataChanged() ) );

  QString toolTipStr = "\"" + tabText( target ) + "\"\n" + tr( "Dictionaries: " )
                       + QString::number( model->getCurrentDictionaries().size() );
  setTabToolTip( target, toolTipStr );
}

void DictGroupsWidget::contextMenu( QPoint const & pos )
{
  int clickedGroup = tabBar()->tabAt( pos );
  if( clickedGroup < 0 )
    return;
  QString name = tabText( clickedGroup );
  if( name.length() != 7 || name.mid( 2, 3 ) != " - " )
    return;

  QMenu menu( this );

  QAction *combineSourceAction = new QAction( QString( tr( "Combine groups by source language to \"%1->\"" ) )
                                              .arg( name.left( 2 ) ), &menu );
  combineSourceAction->setEnabled( false );

  QString grLeft = name.left( 2 );
  QString grRight = name.right( 2 );
  for( int i = 0; i < count(); i++ )
  {
    QString str = tabText( i );
    if( i != clickedGroup && str.length() == 7 && str.mid( 2, 3 ) == " - " && str.startsWith( grLeft ) )
    {
      combineSourceAction->setEnabled( true );
      break;
    }
  }
  menu.addAction( combineSourceAction );

  QAction *combineTargetAction = new QAction( QString( tr( "Combine groups by target language to \"->%1\"" ) )
                                              .arg( name.right( 2 ) ), &menu );
  combineTargetAction->setEnabled( false );

  for( int i = 0; i < count(); i++ )
  {
    QString str = tabText( i );
    if( i != clickedGroup && str.length() == 7 && str.mid( 2, 3 ) == " - " && str.endsWith( grRight ) )
    {
      combineTargetAction->setEnabled( true );
      break;
    }
  }
  menu.addAction( combineTargetAction );

  QAction *combineTwoSidedAction = NULL;
  if( grLeft != grRight )
  {
    combineTwoSidedAction = new QAction( QString( tr( "Make two-side translate group \"%1-%2-%1\"" ) )
                                         .arg( grLeft ).arg( grRight ), &menu );

    combineTwoSidedAction->setEnabled( false );

    QString str = grRight + " - " + grLeft;
    for( int i = 0; i < count(); i++ )
    {
      if( str == tabText( i ) )
      {
        combineTwoSidedAction->setEnabled( true );
        break;
      }
    }

    menu.addAction( combineTwoSidedAction );
  }

  QAction *combineFirstAction = new QAction( QString( tr( "Combine groups with \"%1\"" ) )
                                             .arg( grLeft ), &menu );
  combineFirstAction->setEnabled( false );
  for( int i = 0; i < count(); i++ )
  {
    QString str = tabText( i );
    if( i != clickedGroup && str.length() == 7 && str.mid( 2, 3 ) == " - " &&
        ( str.startsWith( grLeft ) || str.endsWith( grLeft ) ) )
    {
      combineFirstAction->setEnabled( true );
      break;
    }
  }
  menu.addAction( combineFirstAction );

  QAction *combineSecondAction = NULL;

  if( grLeft != grRight )
  {
    combineSecondAction = new QAction( QString( tr( "Combine groups with \"%1\"" ) )
                                        .arg( grRight ), &menu );
    combineSecondAction->setEnabled( false );

    for( int i = 0; i < count(); i++ )
    {
      QString str = tabText( i );
      if( i != clickedGroup && str.length() == 7 && str.mid( 2, 3 ) == " - " &&
          ( str.startsWith( grRight ) || str.endsWith( grRight ) ) )
      {
        combineSecondAction->setEnabled( true );
        break;
      }
    }
    menu.addAction( combineSecondAction );
  }

  QAction *result = menu.exec( mapToGlobal( pos ) );

  setUpdatesEnabled( false );
  int targetGroup;

  if( result && result == combineSourceAction )
  {
    setCurrentIndex( clickedGroup );
    targetGroup = addUniqueGroup( grLeft + "->" );

    for( int i = 0; i < count(); i++ )
    {
      QString str = tabText( i );
      if( str.length() == 7 && str.mid( 2, 3 ) == " - " && str.startsWith( grLeft ) )
        combineGroups( i, targetGroup );
    }

    setCurrentIndex( targetGroup );
  }
  else
  if( result && result == combineTargetAction )
  {
    setCurrentIndex( clickedGroup );
    targetGroup = addUniqueGroup( "->" + grRight );

    for( int i = 0; i < count(); i++ )
    {
      QString str = tabText( i );
      if( str.length() == 7 && str.mid( 2, 3 ) == " - " && str.endsWith( grRight ) )
        combineGroups( i, targetGroup );
    }

    setCurrentIndex( targetGroup );
  }
  else
  if( result && result == combineTwoSidedAction )
  {
    setCurrentIndex( clickedGroup );
    targetGroup = addUniqueGroup( name + " - " + grLeft );
    QString str = grRight + " - " + grLeft;

    for( int i = 0; i < count(); i++ )
      if( tabText( i ) == name || tabText( i ) == str )
        combineGroups( i, targetGroup );

    setCurrentIndex( targetGroup );
  }
  else
  if( result && ( result == combineFirstAction || result == combineSecondAction ) )
  {
    QString const & grBase = result == combineFirstAction ? grLeft : grRight;
    setCurrentIndex( clickedGroup );
    targetGroup = addUniqueGroup( grBase );

    for( int i = 0; i < count(); i++ )
    {
      QString str = tabText( i );
      if( str.length() == 7 && str.mid( 2, 3 ) == " - " &&
          ( str.startsWith( grBase ) || str.endsWith( grBase ) ) )
        combineGroups( i, targetGroup );
    }

    setCurrentIndex( targetGroup );
  }

  setUpdatesEnabled( true );
}

void DictGroupsWidget::tabDataChanged()
{
  QString toolTipStr = "\"" + tabText( currentIndex() ) + "\"\n" + tr( "Dictionaries: " )
                       + QString::number( getCurrentModel()->getCurrentDictionaries().size() );
  setTabToolTip( currentIndex(), toolTipStr );
}

QuickFilterLine::QuickFilterLine( QWidget * parent ): ExtLineEdit( parent ), m_focusAction(this)
{
  m_proxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );

  setPlaceholderText( tr( "Dictionary search/filter (Ctrl+F)" ) );

  m_focusAction.setShortcut( QKeySequence( "Ctrl+F" ) );
  connect( &m_focusAction, SIGNAL( triggered() ),
           this, SLOT( focusFilterLine() ) );

  QPixmap image(":/icons/system-search.svg");
  setButtonPixmap(ExtLineEdit::Left, image.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  setButtonToolTip(ExtLineEdit::Left, tr("Quick Search"));
  setButtonVisible(ExtLineEdit::Left, true);

  QPixmap right(":/icons/clear.svg");
  setButtonPixmap(ExtLineEdit::Right, right);
  setButtonToolTip(ExtLineEdit::Right, tr("Clear Search"));
  setButtonVisible(ExtLineEdit::Right, true);
  setButtonAutoHide(ExtLineEdit::Right, true);
  connect( this, SIGNAL( rightButtonClicked() ), this, SLOT( clear() ) );

  setFocusPolicy(Qt::StrongFocus);

  connect (this, SIGNAL( textChanged( QString const & ) ),
      this, SLOT( filterChangedInternal() ) );
}

QuickFilterLine::~QuickFilterLine()
{
}

void QuickFilterLine::applyTo( QAbstractItemView * source )
{
  m_source = source;
  m_proxyModel.setSourceModel( source->model() );
  source->setModel( &m_proxyModel );
}

QModelIndex QuickFilterLine::mapToSource( QModelIndex const & idx )
{
  if ( &m_proxyModel == idx.model() )
  {
    return m_proxyModel.mapToSource( idx );
  }
  else
  {
    return idx;
  }
}

void QuickFilterLine::filterChangedInternal()
{
  // emit signal in async manner, to avoid UI slowdown
  QTimer::singleShot( 1, this, SLOT( emitFilterChanged() ) );
}

void QuickFilterLine::emitFilterChanged()
{
  m_proxyModel.setFilterFixedString(text());
  emit filterChanged( text() );
}

void QuickFilterLine::focusFilterLine()
{
  setFocus();
  selectAll();
}

void QuickFilterLine::keyPressEvent( QKeyEvent * event )
{
  switch ( event->key() )
  {
    case Qt::Key_Down:
      if ( m_source )
      {
        m_source->setCurrentIndex( m_source->model()->index( 0,0 ) );
        m_source->setFocus();
      }
      break;
    default:
      ExtLineEdit::keyPressEvent( event );
  }
}
