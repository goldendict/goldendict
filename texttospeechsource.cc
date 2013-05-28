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

  SpeechClient::Engines engines = SpeechClient::availableEngines();

  ui.selectedVoiceEngines->setTabKeyNavigation( true );
  ui.selectedVoiceEngines->setModel( &voiceEnginesModel );
  ui.selectedVoiceEngines->hideColumn( VoiceEnginesModel::kColumnEngineId );
  fitSelectedVoiceEnginesColumns();
  ui.selectedVoiceEngines->setItemDelegateForColumn( VoiceEnginesModel::kColumnEngineName,
                                                     new VoiceEngineItemDelegate( engines, this ) );

  foreach ( SpeechClient::Engine engine, engines )
  {
    ui.availableVoiceEngines->addItem( engine.name, engine.id );
  }

  if( voiceEngines.count() > 0 )
  {
    QModelIndex const &idx = ui.selectedVoiceEngines->model()->index( 0, 0 );
    if( idx.isValid() )
      ui.selectedVoiceEngines->setCurrentIndex( idx );
  }

  adjustSliders();

  connect( ui.volumeSlider, SIGNAL( valueChanged( int ) ),
           this, SLOT( slidersChanged() ) );
  connect( ui.rateSlider, SIGNAL( valueChanged( int ) ),
           this, SLOT( slidersChanged() ) );
  connect( ui.selectedVoiceEngines->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ),
           this, SLOT( selectionChanged() ) );
}

void TextToSpeechSource::slidersChanged()
{
  if( ui.selectedVoiceEngines->selectionModel()->hasSelection() )
    voiceEnginesModel.setEngineParams( ui.selectedVoiceEngines->currentIndex(),
                                       ui.volumeSlider->value(),
                                       ui.rateSlider->value()) ;
}

void TextToSpeechSource::on_addVoiceEngine_clicked()
{
  if ( ui.availableVoiceEngines->count() == 0 )
  {
    QMessageBox::information( this, tr( "No TTS voice available" ),
                              tr( "Cannot find available TTS voice.<br>"
                                  "Please make sure that at least one TTS engine installed on your computer already." ) );
    return;
  }

  int idx = ui.availableVoiceEngines->currentIndex();
  if( idx >= 0 )
  {
    QString name = ui.availableVoiceEngines->itemText( idx );
    QString id = ui.availableVoiceEngines->itemData( idx ).toString();
    voiceEnginesModel.addNewVoiceEngine( id, name, ui.volumeSlider->value(), ui.rateSlider->value() );
    fitSelectedVoiceEnginesColumns();
  }
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
  {
    voiceEnginesModel.removeVoiceEngine( current.row() );
  }
}

void TextToSpeechSource::on_previewVoice_clicked()
{
  int idx = ui.availableVoiceEngines->currentIndex();
  if ( idx < 0 )
    return;

  QString engineId = ui.availableVoiceEngines->itemData( idx ).toString();
  QString name = ui.availableVoiceEngines->itemText( idx );
  QString text = ui.previewText->text();
  int volume = ui.volumeSlider->value();
  int rate = ui.rateSlider->value();

  SpeechClient * speechClient = new SpeechClient( Config::VoiceEngine( engineId, name, volume, rate ), this );

  connect( speechClient, SIGNAL( started( bool ) ), ui.previewVoice, SLOT( setDisabled( bool ) ) );
  connect( speechClient, SIGNAL( finished() ), this, SLOT( previewVoiceFinished() ) );
  connect( speechClient, SIGNAL( finished() ), speechClient, SLOT( deleteLater() ) );
  speechClient->tell( text );
}

void TextToSpeechSource::previewVoiceFinished()
{
  ui.previewVoice->setDisabled( false );
}

void TextToSpeechSource::fitSelectedVoiceEnginesColumns()
{
  ui.selectedVoiceEngines->resizeColumnToContents( VoiceEnginesModel::kColumnEnabled );
  ui.selectedVoiceEngines->resizeColumnToContents( VoiceEnginesModel::kColumnEngineName );
  ui.selectedVoiceEngines->resizeColumnToContents( VoiceEnginesModel::kColumnIcon );
}

void TextToSpeechSource::adjustSliders()
{
  QModelIndex const & index = ui.selectedVoiceEngines->currentIndex();
  if ( index.isValid() )
  {
    Config::VoiceEngines const &engines = voiceEnginesModel.getCurrentVoiceEngines();
    ui.volumeSlider->setValue( engines[ index.row() ].volume );
    ui.rateSlider->setValue( engines[ index.row() ].rate );
    return;
  }
  ui.volumeSlider->setValue( 50 );
  ui.rateSlider->setValue( 50 );
}

void TextToSpeechSource::selectionChanged()
{
  disconnect( ui.volumeSlider, SIGNAL( valueChanged( int ) ),
              this, SLOT( slidersChanged() ) );
  disconnect( ui.rateSlider, SIGNAL( valueChanged( int ) ),
              this, SLOT( slidersChanged() ) );

  adjustSliders();

  connect( ui.volumeSlider, SIGNAL( valueChanged( int ) ),
           this, SLOT( slidersChanged() ) );
  connect( ui.rateSlider, SIGNAL( valueChanged( int ) ),
           this, SLOT( slidersChanged() ) );
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

void VoiceEnginesModel::addNewVoiceEngine( QString const & id, QString const & name,
                                           int volume, int rate )
{
  if ( id.isEmpty() || name.isEmpty() )
    return;

  Config::VoiceEngine v;
  v.enabled = true;
  v.id = id;
  v.name = name;
  v.volume = volume;
  v.rate = rate;

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
    switch ( index.column() )
    {
      case kColumnEnabled:
        result |= Qt::ItemIsUserCheckable;
        break;
      case kColumnEngineName:
      case kColumnIcon:
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
  return voiceEngines.size();
}

int VoiceEnginesModel::columnCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  return kColumnCount;
}

QVariant VoiceEnginesModel::headerData( int section, Qt::Orientation /*orientation*/, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case kColumnEnabled:
        return tr( "Enabled" );
      case kColumnEngineName:
        return tr( "Name" );
      case kColumnEngineId:
        return tr( "Id" );
      case kColumnIcon:
        return tr( "Icon" );
    }
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
      case kColumnEngineId:
        return voiceEngines[ index.row() ].id;
      case kColumnEngineName:
        return voiceEngines[ index.row() ].name;
      case kColumnIcon:
        return voiceEngines[ index.row() ].iconFilename;
      default:
        return QVariant();
    }
  }

  if ( role == Qt::CheckStateRole && index.column() == kColumnEnabled )
    return voiceEngines[ index.row() ].enabled ? Qt::Checked : Qt::Unchecked;

  return QVariant();
}

bool VoiceEnginesModel::setData( QModelIndex const & index, const QVariant & value,
                                 int role )
{
  if ( index.row() >= voiceEngines.size() )
    return false;

  if ( role == Qt::CheckStateRole && index.column() == kColumnEnabled )
  {
    voiceEngines[ index.row() ].enabled = !voiceEngines[ index.row() ].enabled;
    dataChanged( index, index );
    return true;
  }

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    switch ( index.column() )
    {
      case kColumnEngineId:
        voiceEngines[ index.row() ].id = value.toString();
        dataChanged( index, index );
        return true;
      case kColumnEngineName:
        voiceEngines[ index.row() ].name = value.toString();
        dataChanged( index, index );
        return true;
      case kColumnIcon:
        voiceEngines[ index.row() ].iconFilename = value.toString();
        dataChanged( index, index );
        return true;
      default:
        return false;
    }
  }

  return false;
}

void VoiceEnginesModel::setEngineParams( QModelIndex idx, int volume, int rate )
{
  if ( idx.isValid() )
  {
    voiceEngines[ idx.row() ].volume = volume;
    voiceEngines[ idx.row() ].rate = rate;
  }
}

VoiceEngineEditor::VoiceEngineEditor( SpeechClient::Engines const & engines, QWidget * parent ):
  QComboBox( parent )
{
  foreach ( SpeechClient::Engine engine, engines )
  {
    addItem( engine.name, engine.id );
  }
}

QString VoiceEngineEditor::engineName() const
{
  int idx = currentIndex();
  if ( idx < 0 )
    return "";
  return itemText( idx );
}

QString VoiceEngineEditor::engineId() const
{
  int idx = currentIndex();
  if ( idx < 0 )
    return "";
  return itemData( idx ).toString();
}

void VoiceEngineEditor::setEngineId( QString const & engineId )
{
  // Find index for the id
  int idx = -1;
  for ( int i = 0; i < count(); ++i )
  {
    if ( engineId == itemData( i ).toString() )
    {
      idx = i;
      break;
    }
  }
  setCurrentIndex( idx );
}

VoiceEngineItemDelegate::VoiceEngineItemDelegate( SpeechClient::Engines const & engines, QObject * parent ) :
  QStyledItemDelegate( parent ),
  engines( engines )
{
}

QWidget * VoiceEngineItemDelegate::createEditor( QWidget * parent,
                                                 QStyleOptionViewItem const & option,
                                                 QModelIndex const & index ) const
{
  if ( index.column() != VoiceEnginesModel::kColumnEngineName )
    return QStyledItemDelegate::createEditor( parent, option, index );
  return new VoiceEngineEditor( engines, parent );
}

void VoiceEngineItemDelegate::setEditorData( QWidget * uncastedEditor, const QModelIndex & index ) const
{
  VoiceEngineEditor * editor = qobject_cast< VoiceEngineEditor * >( uncastedEditor );
  if ( !editor )
    return;

  int currentRow = index.row();
  QModelIndex engineIdIndex = index.sibling( currentRow, VoiceEnginesModel::kColumnEngineId );
  QString engineId = index.model()->data( engineIdIndex ).toString();
  editor->setEngineId( engineId );
}

void VoiceEngineItemDelegate::setModelData( QWidget * uncastedEditor, QAbstractItemModel * model,
                                            const QModelIndex & index ) const
{
  VoiceEngineEditor * editor = qobject_cast< VoiceEngineEditor * >( uncastedEditor );
  if ( !editor )
    return;

  int currentRow = index.row();
  QModelIndex engineIdIndex = index.sibling( currentRow, VoiceEnginesModel::kColumnEngineId );
  QModelIndex engineNameIndex = index.sibling( currentRow, VoiceEnginesModel::kColumnEngineName );
  model->setData( engineIdIndex, editor->engineId() );
  model->setData( engineNameIndex, editor->engineName() );
}
