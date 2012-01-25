/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
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

using std::vector;

/// DictGroupWidget

DictGroupWidget::DictGroupWidget( QWidget * parent,
                                  vector< sptr< Dictionary::Class > > const & dicts,
                                  Config::Group const & group ):
  QWidget( parent ),
  groupId( group.id )
{
  ui.setupUi( this );
  ui.dictionaries->populate( Instances::Group( group, dicts ).dictionaries, dicts );

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

  connect( ui.groupIcon, SIGNAL(activated(int)),this,SLOT(groupIconActivated(int)),
           Qt::QueuedConnection );
}

void DictGroupWidget::groupIconActivated( int index )
{
  if ( index == 1 )
  {
    QList< QByteArray > supImageFormats = QImageReader::supportedImageFormats();

    QString formatList( " (" );

    for( int x = 0; x < supImageFormats.size(); ++x )
      formatList += "*." + QString::fromAscii( supImageFormats[ x ] ) + " ";

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

  return g.makeConfigGroup();
}

/// DictListModel

void DictListModel::populate(
  std::vector< sptr< Dictionary::Class > > const & active,
  std::vector< sptr< Dictionary::Class > > const & available )
{
  dictionaries = active;
  allDicts = &available;

  reset();
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
  sptr< Dictionary::Class > const & item = dictionaries[ index.row() ];

  if ( !item )
    return QVariant();

  switch ( role )
  {
    case Qt::ToolTipRole:
    {
      QString tt = "<b>" + QString::fromUtf8( item->getName().c_str() ) + "</b>";

      QString lfrom( item->getLocalizedNameFrom() );// Language::localizedNameForId( item->getLangFrom() ) );
      QString lto( item->getLocalizedNameTo() );//Language::localizedNameForId( item->getLangTo() ) );
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

  return true;
}

void DictListModel::addRow(const QModelIndex & parent, sptr< Dictionary::Class > dict)
{
  for (int i = 0; i < dictionaries.size(); i++)
  {
    if (dictionaries[i]->getId() == dict->getId())
      return;
  }

  beginInsertRows( parent, dictionaries.size(), dictionaries.size()+1 );
  dictionaries.push_back(dict);
  endInsertRows();
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

    Instances::Group i( g, *allDicts );

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

  reset();
}

void DictListModel::addSelectedUniqueFromModel( QItemSelectionModel * source )
{
  if ( !source )
    return;

  QModelIndexList rows = source->selectedRows();

  if ( !rows.count() )
    return;

  const DictListModel * baseModel = dynamic_cast< const DictListModel * > ( source->model() );
  if ( !baseModel )
    return;

  QVector< std::string > list;
  QVector< std::string > dicts;
  for ( unsigned i = 0; i < dictionaries.size(); i++ )
    dicts.append( dictionaries.at( i )->getId() );

  for ( int i = 0; i < rows.count(); i++ )
  {
    std::string id = baseModel->dictionaries.at( rows.at( i ).row() )->getId();

    if ( !dicts.contains( id ) )
      list.append( id );
  }

  if ( list.empty() )
    return;

  for ( unsigned i = 0; i < allDicts->size(); i++ )
  {
    for ( int j = 0; j < list.size(); j++ )
    {
      if ( allDicts->at( i )->getId() == list.at( j ) )
      {
        dictionaries.push_back( allDicts->at( i ) );
        list.remove( j );
        if ( list.isEmpty() )
        {
          reset();
          return;
        }
        break;
      }
    }
  }

  reset();
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
    reset();
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
  QTabWidget( parent ), nextId( 1 ), allDicts( 0 )
{
#if QT_VERSION >= 0x040500
  setMovable( true );
#endif
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
                                 vector< sptr< Dictionary::Class > > const & allDicts_ )
{
  while( count() )
    removeCurrentGroup();

  allDicts = &allDicts_;

  for( unsigned x = 0; x < groups.size(); ++x )
    addTab( new DictGroupWidget( this, *allDicts, groups[ x ] ), escapeAmps( groups[ x ].name ) );

  nextId = groups.nextId;
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

  insertTab( idx,
             new DictGroupWidget( this, *allDicts, newGroup ),
             escapeAmps( name ) );

  setCurrentIndex( idx );
}

void DictGroupsWidget::addAutoGroups()
{
  if ( !allDicts )
    return;

  if ( QMessageBox::information( this, tr( "Confirmation" ),
         tr( "Are you sure you want to generate a set of groups "
             "based on language pairs?" ), QMessageBox::Yes,
             QMessageBox::Cancel ) != QMessageBox::Yes )
    return;

  QMap< QPair<quint32,quint32>, int > tabMap;

//  ::Initializing init( this, true );
//  QApplication::processEvents();

  for ( int i = 0; i < allDicts->size(); i++ )
  {
    sptr<Dictionary::Class> dict = allDicts->at( i );

    quint32 idfrom = dict->getLangFrom();
    quint32 idto = dict->getLangTo();
    QPair<quint32,quint32> key(idfrom, idto);

    if (tabMap.contains(key))
    {
      setCurrentIndex(tabMap[key]);
    }
    else
    {
      QString lfrom = LangCoder::intToCode2( idfrom );
      QString lto = LangCoder::intToCode2( idto );
      QString name("Unassigned");
      if (lfrom.isEmpty() == false && lto.isEmpty() == false)
        name = lfrom + " - " + lto;

      // search for the language group
      bool found = false;
      for (int j = 0; j < count(); j++)
      {
        if (tabText(j) == name)
        {
          found = true;
          setCurrentIndex(j);
          tabMap[key] = j;
          break;
        }
      }

      // group not found - add it
      if (!found)
      {
        addNewGroup(name);
        int j = currentIndex();
        tabMap[key] = j;
      }

    }

    // add dictionary into the current group
    DictListModel *model = getCurrentModel();
    model->addRow(QModelIndex(), dict);
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
