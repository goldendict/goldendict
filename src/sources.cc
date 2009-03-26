/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "sources.hh"
#include <QFileDialog>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDateTime>

Sources::Sources( QWidget * parent, Config::Paths const & paths_,
                  Config::MediaWikis const & mediawikis ): QDialog( parent ),
  mediawikisModel( this, mediawikis ), paths( paths_ )
{
  ui.setupUi( this );

  ui.mediaWikis->setTabKeyNavigation( true );
  ui.mediaWikis->setModel( &mediawikisModel );
  ui.mediaWikis->resizeColumnToContents( 0 );
  ui.mediaWikis->resizeColumnToContents( 1 );
  ui.mediaWikis->resizeColumnToContents( 2 );

  for( Config::Paths::const_iterator i = paths.begin(); i != paths.end(); ++i )
    ui.dictionaries->addItem( *i );

  ui.dictionaries->setDragEnabled( true );
  ui.dictionaries->setAcceptDrops( true );
  ui.dictionaries->setDropIndicatorShown( true );

  connect( ui.buttons, SIGNAL( accepted() ),
            this, SLOT( accept() ) );
  connect( ui.buttons, SIGNAL( rejected() ),
            this, SLOT( reject() ) );

  connect( ui.add, SIGNAL( clicked() ),
            this, SLOT( add() ) );
  connect( ui.remove, SIGNAL( clicked() ),
            this, SLOT( remove() ) );
}

void Sources::add()
{
  QString dir = 
    QFileDialog::getExistingDirectory( this, tr( "Choose a directory" ) );

  if ( !dir.isEmpty() )
  {
    paths.push_back( dir );
    ui.dictionaries->addItem( dir );
  }
}

void Sources::remove()
{
  int row = ui.dictionaries->currentRow();

  if ( row >= 0 && row < (int)paths.size() &&
      QMessageBox::question( this, tr( "Confirm removal" ),
                             tr( "Remove directory <b>%1</b> from the list?" ).arg( paths[ row ] ),
                             QMessageBox::Ok,
                             QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    if( QListWidgetItem * item = ui.dictionaries->takeItem( row ) )
      delete item;
    paths.erase( paths.begin() + row );
  }
}

void Sources::on_addMediaWiki_clicked()
{
  mediawikisModel.addNewWiki();
  QModelIndex result =
    mediawikisModel.index( mediawikisModel.rowCount( QModelIndex() ) - 1,
                           1, QModelIndex() );

  ui.mediaWikis->scrollTo( result );
  //ui.mediaWikis->setCurrentIndex( result );
  ui.mediaWikis->edit( result );
}

void Sources::on_removeMediaWiki_clicked()
{
  QModelIndex current = ui.mediaWikis->currentIndex();

  if ( current.isValid() &&
       QMessageBox::question( this, tr( "Confirm removal" ),
                              tr( "Remove site <b>%1</b> from the list?" ).arg( mediawikisModel.getCurrentWikis()[ current.row() ].name ),
                              QMessageBox::Ok,
                              QMessageBox::Cancel ) == QMessageBox::Ok )
    mediawikisModel.removeWiki( current.row() );
}

////////// MediaWikisModel

MediaWikisModel::MediaWikisModel( QWidget * parent,
                                  Config::MediaWikis const & mediawikis_ ):
  QAbstractItemModel( parent ), mediawikis( mediawikis_ )
{
}
void MediaWikisModel::removeWiki( int index )
{
  beginRemoveRows( QModelIndex(), index, index );
  mediawikis.erase( mediawikis.begin() + index );
  endRemoveRows();
}

void MediaWikisModel::addNewWiki()
{
  Config::MediaWiki w;

  w.enabled = false;

  // That's quite some rng
  w.id = QString(
    QCryptographicHash::hash(
      QDateTime::currentDateTime().toString( "\"MediaWiki\"dd.MM.yyyy hh:mm:ss.zzz" ).toUtf8(),
      QCryptographicHash::Md5 ).toHex() );

  w.url = "http://";

  beginInsertRows( QModelIndex(), mediawikis.size(), mediawikis.size() );
  mediawikis.push_back( w );
  endInsertRows();
}

QModelIndex MediaWikisModel::index( int row, int column, QModelIndex const & /*parent*/ ) const
{
  return createIndex( row, column, 0 );
}

QModelIndex MediaWikisModel::parent( QModelIndex const & /*parent*/ ) const
{
  return QModelIndex();
}

Qt::ItemFlags MediaWikisModel::flags( QModelIndex const & index ) const
{
  Qt::ItemFlags result = QAbstractItemModel::flags( index );

  if ( index.isValid() )
  {
    if ( !index.column() )
      result |= Qt::ItemIsUserCheckable;
    else
      result |= Qt::ItemIsEditable;
  }

  return result;
}

int MediaWikisModel::rowCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return mediawikis.size();
}

int MediaWikisModel::columnCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return 3;
}

QVariant MediaWikisModel::headerData( int section, Qt::Orientation /*orientation*/, int role ) const
{
  if ( role == Qt::DisplayRole )
    switch( section )
    {
      case 0:
        return tr( "Enabled" );
      case 1:
        return tr( "Name" );
      case 2:
        return tr( "Address" );
      default:
        return QVariant();
    }

  return QVariant();
}

QVariant MediaWikisModel::data( QModelIndex const & index, int role ) const
{
  if ( (unsigned)index.row() >= mediawikis.size() )
    return QVariant();

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    switch( index.column() )
    {
      case 1:
        return mediawikis[ index.row() ].name;
      case 2:
        return mediawikis[ index.row() ].url;
      default:
        return QVariant();
    }
  }

  if ( role == Qt::CheckStateRole && !index.column() )
    return mediawikis[ index.row() ].enabled;

  return QVariant();
}

bool MediaWikisModel::setData( QModelIndex const & index, const QVariant & value,
                               int role )
{
  if ( (unsigned)index.row() >= mediawikis.size() )
    return false;

  if ( role == Qt::CheckStateRole && !index.column() )
  {
    //printf( "type = %d\n", (int)value.type() );
    //printf( "value = %d\n", (int)value.toInt() );

    // XXX it seems to be always passing Int( 2 ) as a value, so we just toggle
    mediawikis[ index.row() ].enabled = !mediawikis[ index.row() ].enabled;

    dataChanged( index, index );
    return true;
  }

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
    switch( index.column() )
    {
      case 1:
        mediawikis[ index.row() ].name =  value.toString();
        dataChanged( index, index );
        return true;
      case 2:
        mediawikis[ index.row() ].url =  value.toString();
        dataChanged( index, index );
        return true;
      default:
        return false;
    }

  return false;
}

