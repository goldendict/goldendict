/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "groups_widgets.hh"

#include "instances.hh"
#include "config.hh"
#include "langcoder.hh"

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

  ui.groupIcon->addItem( "None", "" );

  for( int x = 0; x < icons.size(); ++x )
  {
    QString n( icons[ x ] );
    n.chop( 4 );
    n[ 0 ] = n[ 0 ].toUpper();

    ui.groupIcon->addItem( QIcon( ":/flags/" + icons[ x ] ), n, icons[ x ] );

    if ( icons[ x ] == group.icon )
      ui.groupIcon->setCurrentIndex( x + 1 );
  }
}

Config::Group DictGroupWidget::makeGroup() const
{
  Instances::Group g( "" );

  g.id = groupId;

  g.dictionaries = ui.dictionaries->getCurrentDictionaries();

  g.icon = ui.groupIcon->itemData( ui.groupIcon->currentIndex() ).toString();

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
    case Qt::DisplayRole :
      return QString::fromUtf8( item->getName().c_str() );
//          + QString("  lang: %1 %2").arg( langCoder.decode(item->getLangFrom()),
//                                        langCoder.decode(item->getLangTo()) );

    case Qt::EditRole :
      return QString::fromUtf8( item->getId().c_str() );

    case Qt::DecorationRole:
      return item->getIcon();

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

// DictGroupsWidget

DictGroupsWidget::DictGroupsWidget( QWidget * parent ):
  QTabWidget( parent ), nextId( 1 ), allDicts( 0 )
{
  setMovable( true );
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
