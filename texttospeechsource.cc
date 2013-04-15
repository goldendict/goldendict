/* This file is (c) 2013 Timon Wong <timon86.wang@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "texttospeechsource.hh"

#include <QMessageBox>

TextToSpeechSource::TextToSpeechSource( QWidget * parent,
                                        Config::VoiceEngines voiceEngines ):
  QWidget( parent ),
  voiceEnginesModel( this, voiceEngines )
{
  ui.setupUi( this );

  ui.selectedVoiceEngines->setTabKeyNavigation( true );
  ui.selectedVoiceEngines->setModel( &voiceEnginesModel );

  SpeechClient::Engines engines = SpeechClient::availableEngines();
  foreach ( SpeechClient::Engine engine, engines )
  {
    ui.availableVoiceEngines->addItem( engine.name, engine.id );
  }
}

void TextToSpeechSource::on_addVoiceEngine_clicked()
{
  int idx = ui.availableVoiceEngines->currentIndex();
  if ( idx < 0 )
    return;

  QString name = ui.availableVoiceEngines->itemText( idx );
  QString id = ui.availableVoiceEngines->itemData( idx ).toString();
  voiceEnginesModel.addNewVoiceEngine( id, name );
  fitSelectedVoiceEnginesColumns();
}

void TextToSpeechSource::on_removeVoiceEngine_clicked()
{
  QModelIndex current = ui.selectedVoiceEngines->currentIndex();

  if ( current.isValid() &&
       QMessageBox::question( this, tr( "Confirm removal" ),
                              tr( "Remove voice engine <b>%1</b> from the list?" ).arg(
                                voiceEnginesModel.getCurrentVoiceEngines()[ current.row() ].name ),
                              QMessageBox::Ok,
                              QMessageBox::Cancel ) == QMessageBox::Ok )

  voiceEnginesModel.removeVoiceEngine( current.row() );
}

void TextToSpeechSource::on_previewVoice_clicked()
{
  int idx = ui.availableVoiceEngines->currentIndex();
  if ( idx < 0 )
    return;

  QString engineId = ui.availableVoiceEngines->itemData( idx ).toString();
  QString text = ui.previewText->text();
  SpeechClient *speechClient = new SpeechClient( engineId, this );

  ui.previewVoice->setEnabled( false );
  if ( !speechClient->tell( text, this, SLOT( previewVoiceFinished( SpeechClient * ) ) ) ) {
    ui.previewVoice->setEnabled( true );
    delete speechClient;
  }
}

void TextToSpeechSource::previewVoiceFinished( SpeechClient * speechClient )
{
  ui.previewVoice->setEnabled( true );

  if (speechClient) {
    delete speechClient;
  }
}

void TextToSpeechSource::fitSelectedVoiceEnginesColumns()
{
  ui.selectedVoiceEngines->resizeColumnToContents( 0 );
  ui.selectedVoiceEngines->resizeColumnToContents( 1 );
}

VoiceEnginesModel::VoiceEnginesModel( QWidget * parent,
                                      Config::VoiceEngines const & voiceEngines ):
  QAbstractItemModel( parent ), voiceEngines( voiceEngines )
{
}

void VoiceEnginesModel::removeVoiceEngine( int index )
{
	beginRemoveRows( QModelIndex(), index, index );
  voiceEngines.erase( voiceEngines.begin() + index );
  endRemoveRows();
}

void VoiceEnginesModel::addNewVoiceEngine( QString const & id, QString const & name )
{
  if ( id.isEmpty() || name.isEmpty() )
    return;

	Config::VoiceEngine v;
	v.enabled = true;
	v.id = id;
	v.name = name;

  // Check dupliates
  foreach ( Config::VoiceEngine engine, voiceEngines )
  {
    if ( v.id == engine.id )
      return;
  }

  beginInsertRows( QModelIndex(), voiceEngines.size(), voiceEngines.size() );
  voiceEngines.push_back( v );
  endInsertRows();
}

QModelIndex VoiceEnginesModel::index( int row, int column, QModelIndex const & /*parent*/ ) const
{
  return createIndex( row, column, 0 );
}

QModelIndex VoiceEnginesModel::parent( QModelIndex const & /*parent*/ ) const
{
  return QModelIndex();
}

Qt::ItemFlags VoiceEnginesModel::flags( QModelIndex const & index ) const
{
  Qt::ItemFlags result = QAbstractItemModel::flags( index );

  if ( index.isValid() )
  {
    switch ( index.column() ) {
    case 0:
      result |= Qt::ItemIsUserCheckable;
      break;
    case 2:
      result |= Qt::ItemIsEditable;
      break;
    }
  }

  return result;
}

int VoiceEnginesModel::rowCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return voiceEngines.size();
}

int VoiceEnginesModel::columnCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return 3;
}

QVariant VoiceEnginesModel::headerData( int section, Qt::Orientation /*orientation*/, int role ) const
{
  if ( role == Qt::DisplayRole )
    switch( section )
    {
      case 0:
        return tr( "Enabled" );
      case 1:
        return tr( "Name" );
      case 2:
        return tr( "Icon" );
      default:
        return QVariant();
    }

  return QVariant();
}

QVariant VoiceEnginesModel::data( QModelIndex const & index, int role ) const
{
  if ( index.row() >= voiceEngines.size() )
    return QVariant();

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    switch ( index.column() )
    {
      case 1:
        return voiceEngines[ index.row() ].name;
      case 2:
        return voiceEngines[ index.row() ].iconFilename;
      default:
        return QVariant();
    }
  }

  if ( role == Qt::CheckStateRole && !index.column() )
    return voiceEngines[ index.row() ].enabled;

  return QVariant();
}

bool VoiceEnginesModel::setData( QModelIndex const & index, const QVariant & value,
                              int role )
{
  if ( index.row() >= voiceEngines.size() )
    return false;

  if ( role == Qt::CheckStateRole && !index.column() )
  {
    voiceEngines[ index.row() ].enabled = !voiceEngines[ index.row() ].enabled;
    dataChanged( index, index );
    return true;
  }

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    switch ( index.column() )
    {
      case 2:
        voiceEngines[ index.row() ].iconFilename = value.toString();
        dataChanged( index, index );
        return true;
      default:
        return false;
    }
  }

  return false;
}
