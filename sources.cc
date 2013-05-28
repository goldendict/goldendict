/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "sources.hh"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include "dprintf.hh"


Sources::Sources( QWidget * parent, Config::Class const & cfg):
  QWidget( parent ),
#if defined( Q_OS_WIN32 ) || defined( Q_OS_MACX )
  textToSpeechSource( NULL ),
#endif
  itemDelegate( new QItemDelegate( this ) ),
  itemEditorFactory( new QItemEditorFactory() ),
  mediawikisModel( this, cfg.mediawikis ),
  webSitesModel( this, cfg.webSites ),
  programsModel( this, cfg.programs ),
  pathsModel( this, cfg.paths ),
  soundDirsModel( this, cfg.soundDirs ),
  hunspellDictsModel( this, cfg.hunspell )
{
  ui.setupUi( this );

  Config::Hunspell const & hunspell = cfg.hunspell;
  Config::Transliteration const & trs = cfg.transliteration;
  Config::Forvo const & forvo = cfg.forvo;

  // TODO: will programTypeEditorCreator and itemEditorFactory be destoryed by
  // anyone?
  QItemEditorCreatorBase * programTypeEditorCreator =
         new QStandardItemEditorCreator< ProgramTypeEditor >();

  itemEditorFactory->registerEditor( QVariant::Int, programTypeEditorCreator );

  itemDelegate->setItemEditorFactory( itemEditorFactory );

  ui.mediaWikis->setTabKeyNavigation( true );
  ui.mediaWikis->setModel( &mediawikisModel );
  ui.mediaWikis->resizeColumnToContents( 0 );
  ui.mediaWikis->resizeColumnToContents( 1 );
  ui.mediaWikis->resizeColumnToContents( 2 );
  ui.mediaWikis->resizeColumnToContents( 3 );

  ui.webSites->setTabKeyNavigation( true );
  ui.webSites->setModel( &webSitesModel );
  ui.webSites->resizeColumnToContents( 0 );
  ui.webSites->resizeColumnToContents( 1 );
  ui.webSites->resizeColumnToContents( 2 );
  ui.webSites->resizeColumnToContents( 3 );

  ui.programs->setTabKeyNavigation( true );
  ui.programs->setModel( &programsModel );
  ui.programs->resizeColumnToContents( 0 );
  // Make sure this thing will be large enough
  ui.programs->setColumnWidth( 1,
    QFontMetrics( QFont() ).width(
      ProgramTypeEditor::getNameForType( Config::Program::PrefixMatch ) ) + 16 );
  ui.programs->resizeColumnToContents( 2 );
  ui.programs->resizeColumnToContents( 3 );
  ui.programs->resizeColumnToContents( 4 );
  ui.programs->setItemDelegate( itemDelegate );

  ui.paths->setTabKeyNavigation( true );
  ui.paths->setModel( &pathsModel );

  fitPathsColumns();

  ui.soundDirs->setTabKeyNavigation( true );
  ui.soundDirs->setModel( &soundDirsModel );

  fitSoundDirsColumns();

  ui.hunspellPath->setText( hunspell.dictionariesPath );
  ui.hunspellDictionaries->setTabKeyNavigation( true );
  ui.hunspellDictionaries->setModel( &hunspellDictsModel );

  fitHunspellDictsColumns();

  ui.enableRussianTransliteration->setChecked( trs.enableRussianTransliteration );
  ui.enableGermanTransliteration->setChecked( trs.enableGermanTransliteration );
  ui.enableGreekTransliteration->setChecked( trs.enableGreekTransliteration );
  ui.enableBelarusianTransliteration->setChecked( trs.enableBelarusianTransliteration );
  ui.enableRomaji->setChecked( trs.romaji.enable );
  ui.enableHepburn->setChecked( trs.romaji.enableHepburn );
  ui.enableNihonShiki->setChecked( trs.romaji.enableNihonShiki );
  ui.enableKunreiShiki->setChecked( trs.romaji.enableKunreiShiki );
  ui.enableHiragana->setChecked( trs.romaji.enableHiragana );
  ui.enableKatakana->setChecked( trs.romaji.enableKatakana );

  ui.forvoEnabled->setChecked( forvo.enable );
  ui.forvoApiKey->setText( forvo.apiKey );
  ui.forvoLanguageCodes->setText( forvo.languageCodes );

  // Text to speech
#if defined( Q_OS_WIN32 ) || defined( Q_OS_MACX )
  textToSpeechSource = new TextToSpeechSource( this, cfg.voiceEngines );
  ui.tabWidget->addTab( textToSpeechSource, tr( "Text to Speech" ) );
#endif

  if ( Config::isPortableVersion() )
  {
    // Paths

    ui.paths->setEnabled( false );
    ui.addPath->setEnabled( false );
    ui.removePath->setEnabled( false );

    // Sound dirs

    {
      QStandardItemModel * model =  new QStandardItemModel( this );
      model->setHorizontalHeaderLabels( QStringList() << " " );
      model->invisibleRootItem()->appendRow( new QStandardItem( tr( "(not available in portable version)" ) ) );
      ui.soundDirs->setModel( model );
      ui.soundDirs->setEnabled( false );

      ui.addSoundDir->setEnabled( false );
      ui.removeSoundDir->setEnabled( false );
    }

    // Morpho

    ui.hunspellPath->setEnabled( false );
    ui.changeHunspellPath->setEnabled( false );
  }
}

void Sources::fitPathsColumns()
{
  ui.paths->resizeColumnToContents( 0 );
  ui.paths->resizeColumnToContents( 1 );
}

void Sources::fitSoundDirsColumns()
{
  ui.soundDirs->resizeColumnToContents( 0 );
  ui.soundDirs->resizeColumnToContents( 1 );
  ui.soundDirs->resizeColumnToContents( 2 );
}

void Sources::fitHunspellDictsColumns()
{
  ui.hunspellDictionaries->resizeColumnToContents( 0 );
  ui.hunspellDictionaries->resizeColumnToContents( 1 );
}

void Sources::on_addPath_clicked()
{
  QString dir =
    QFileDialog::getExistingDirectory( this, tr( "Choose a directory" ) );

  if ( !dir.isEmpty() )
  {
    pathsModel.addNewPath( dir );
    fitPathsColumns();
  }
}

void Sources::on_removePath_clicked()
{
  QModelIndex current = ui.paths->currentIndex();

  if ( current.isValid() &&
      QMessageBox::question( this, tr( "Confirm removal" ),
                             tr( "Remove directory <b>%1</b> from the list?" ).arg( pathsModel.getCurrentPaths()[ current.row() ].path ),
                             QMessageBox::Ok,
                             QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    pathsModel.removePath( current.row() );
    fitPathsColumns();
  }
}

void Sources::on_addSoundDir_clicked()
{
  QString dir =
    QFileDialog::getExistingDirectory( this, tr( "Choose a directory" ) );

  if ( !dir.isEmpty() )
  {
    soundDirsModel.addNewSoundDir( dir, QDir( dir ).dirName() );
    fitSoundDirsColumns();
  }
}

void Sources::on_removeSoundDir_clicked()
{
  QModelIndex current = ui.soundDirs->currentIndex();

  if ( current.isValid() &&
      QMessageBox::question( this, tr( "Confirm removal" ),
                             tr( "Remove directory <b>%1</b> from the list?" ).arg( soundDirsModel.getCurrentSoundDirs()[ current.row() ].path ),
                             QMessageBox::Ok,
                             QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    soundDirsModel.removeSoundDir( current.row() );
    fitSoundDirsColumns();
  }
}

void Sources::on_changeHunspellPath_clicked()
{
  QString dir =
    QFileDialog::getExistingDirectory( this, tr( "Choose a directory" ) );

  if ( !dir.isEmpty() )
  {
    ui.hunspellPath->setText( dir );
    hunspellDictsModel.changePath( dir );
    fitHunspellDictsColumns();
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

void Sources::on_addWebSite_clicked()
{
  webSitesModel.addNewSite();

  QModelIndex result =
    webSitesModel.index( webSitesModel.rowCount( QModelIndex() ) - 1,
                         1, QModelIndex() );

  ui.webSites->scrollTo( result );
  ui.webSites->edit( result );
}

void Sources::on_removeWebSite_clicked()
{
  QModelIndex current = ui.webSites->currentIndex();

  if ( current.isValid() &&
       QMessageBox::question( this, tr( "Confirm removal" ),
                              tr( "Remove site <b>%1</b> from the list?" ).arg( webSitesModel.getCurrentWebSites()[ current.row() ].name ),
                              QMessageBox::Ok,
                              QMessageBox::Cancel ) == QMessageBox::Ok )
    webSitesModel.removeSite( current.row() );
}

void Sources::on_addProgram_clicked()
{
  programsModel.addNewProgram();

  QModelIndex result =
    programsModel.index( programsModel.rowCount( QModelIndex() ) - 1,
                         1, QModelIndex() );

  ui.programs->scrollTo( result );
  ui.programs->edit( result );
}

void Sources::on_removeProgram_clicked()
{
  QModelIndex current = ui.programs->currentIndex();

  if ( current.isValid() &&
       QMessageBox::question( this, tr( "Confirm removal" ),
                              tr( "Remove program <b>%1</b> from the list?" ).arg( programsModel.getCurrentPrograms()[ current.row() ].name ),
                              QMessageBox::Ok,
                              QMessageBox::Cancel ) == QMessageBox::Ok )
    programsModel.removeProgram( current.row() );
}

Config::VoiceEngines Sources::getVoiceEngines() const
{
#if defined( Q_OS_WIN32 ) || defined( Q_OS_MACX )
  if ( !textToSpeechSource )
    return Config::VoiceEngines();
  return textToSpeechSource->getVoiceEnginesModel().getCurrentVoiceEngines();
#else
  return Config::VoiceEngines();
#endif
}

Config::Hunspell Sources::getHunspell() const
{
  Config::Hunspell h;

  h.dictionariesPath = ui.hunspellPath->text();
  h.enabledDictionaries = hunspellDictsModel.getEnabledDictionaries();

  return h;
}

Config::Transliteration Sources::getTransliteration() const
{
  Config::Transliteration tr;

  tr.enableRussianTransliteration = ui.enableRussianTransliteration->isChecked();
  tr.enableGermanTransliteration = ui.enableGermanTransliteration->isChecked();
  tr.enableGreekTransliteration = ui.enableGreekTransliteration->isChecked();
  tr.enableBelarusianTransliteration = ui.enableBelarusianTransliteration->isChecked();
  tr.romaji.enable = ui.enableRomaji->isChecked();
  tr.romaji.enableHepburn = ui.enableHepburn->isChecked();
  tr.romaji.enableNihonShiki = ui.enableNihonShiki->isChecked();
  tr.romaji.enableKunreiShiki = ui.enableKunreiShiki->isChecked();
  tr.romaji.enableHiragana = ui.enableHiragana->isChecked();
  tr.romaji.enableKatakana = ui.enableKatakana->isChecked();

  return tr;
}

Config::Forvo Sources::getForvo() const
{
  Config::Forvo forvo;

  forvo.enable = ui.forvoEnabled->isChecked();
  forvo.apiKey = ui.forvoApiKey->text();
  forvo.languageCodes = ui.forvoLanguageCodes->text();

  return forvo;
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

  w.id = Dictionary::generateRandomDictionaryId();

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
    return 4;
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
      case 3:
        return tr( "Icon" );
      default:
        return QVariant();
    }

  return QVariant();
}

QVariant MediaWikisModel::data( QModelIndex const & index, int role ) const
{
  if ( index.row() >= mediawikis.size() )
    return QVariant();

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    switch( index.column() )
    {
      case 1:
        return mediawikis[ index.row() ].name;
      case 2:
        return mediawikis[ index.row() ].url;
      case 3:
        return mediawikis[ index.row() ].icon;
      default:
        return QVariant();
    }
  }

  if ( role == Qt::CheckStateRole && !index.column() )
    return mediawikis[ index.row() ].enabled ? Qt::Checked : Qt::Unchecked;

  return QVariant();
}

bool MediaWikisModel::setData( QModelIndex const & index, const QVariant & value,
                               int role )
{
  if ( index.row() >= mediawikis.size() )
    return false;

  if ( role == Qt::CheckStateRole && !index.column() )
  {
    //DPRINTF( "type = %d\n", (int)value.type() );
    //DPRINTF( "value = %d\n", (int)value.toInt() );

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
      case 3:
        mediawikis[ index.row() ].icon =  value.toString();
        dataChanged( index, index );
        return true;
      default:
        return false;
    }

  return false;
}


////////// WebSitesModel

WebSitesModel::WebSitesModel( QWidget * parent,
                              Config::WebSites const & webSites_ ):
  QAbstractItemModel( parent ), webSites( webSites_ )
{
}
void WebSitesModel::removeSite( int index )
{
  beginRemoveRows( QModelIndex(), index, index );
  webSites.erase( webSites.begin() + index );
  endRemoveRows();
}

void WebSitesModel::addNewSite()
{
  Config::WebSite w;

  w.enabled = false;

  w.id = Dictionary::generateRandomDictionaryId();

  w.url = "http://";

  beginInsertRows( QModelIndex(), webSites.size(), webSites.size() );
  webSites.push_back( w );
  endInsertRows();
}

QModelIndex WebSitesModel::index( int row, int column, QModelIndex const & /*parent*/ ) const
{
  return createIndex( row, column, 0 );
}

QModelIndex WebSitesModel::parent( QModelIndex const & /*parent*/ ) const
{
  return QModelIndex();
}

Qt::ItemFlags WebSitesModel::flags( QModelIndex const & index ) const
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

int WebSitesModel::rowCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return webSites.size();
}

int WebSitesModel::columnCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return 4;
}

QVariant WebSitesModel::headerData( int section, Qt::Orientation /*orientation*/, int role ) const
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
      case 3:
        return tr( "Icon" );
      default:
        return QVariant();
    }

  return QVariant();
}

QVariant WebSitesModel::data( QModelIndex const & index, int role ) const
{
  if ( index.row() >= webSites.size() )
    return QVariant();

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    switch( index.column() )
    {
      case 1:
        return webSites[ index.row() ].name;
      case 2:
        return webSites[ index.row() ].url;
      case 3:
        return webSites[ index.row() ].iconFilename;
      default:
        return QVariant();
    }
  }

  if ( role == Qt::CheckStateRole && !index.column() )
    return webSites[ index.row() ].enabled ? Qt::Checked : Qt::Unchecked;

  return QVariant();
}

bool WebSitesModel::setData( QModelIndex const & index, const QVariant & value,
                               int role )
{
  if ( index.row() >= webSites.size() )
    return false;

  if ( role == Qt::CheckStateRole && !index.column() )
  {
    //DPRINTF( "type = %d\n", (int)value.type() );
    //DPRINTF( "value = %d\n", (int)value.toInt() );

    // XXX it seems to be always passing Int( 2 ) as a value, so we just toggle
    webSites[ index.row() ].enabled = !webSites[ index.row() ].enabled;

    dataChanged( index, index );
    return true;
  }

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
    switch( index.column() )
    {
      case 1:
        webSites[ index.row() ].name =  value.toString();
        dataChanged( index, index );
        return true;
      case 2:
        webSites[ index.row() ].url =  value.toString();
        dataChanged( index, index );
        return true;
      case 3:
        webSites[ index.row() ].iconFilename =  value.toString();
        dataChanged( index, index );
        return true;
      default:
        return false;
    }

  return false;
}


////////// ProgramsModel

ProgramsModel::ProgramsModel( QWidget * parent,
                              Config::Programs const & programs_ ):
  QAbstractItemModel( parent ), programs( programs_ )
{
}

void ProgramsModel::removeProgram( int index )
{
  beginRemoveRows( QModelIndex(), index, index );
  programs.erase( programs.begin() + index );
  endRemoveRows();
}

void ProgramsModel::addNewProgram()
{
  Config::Program p;

  p.enabled = false;
  p.type = Config::Program::Audio;

  p.id = Dictionary::generateRandomDictionaryId();

  beginInsertRows( QModelIndex(), programs.size(), programs.size() );
  programs.push_back( p );
  endInsertRows();
}

QModelIndex ProgramsModel::index( int row, int column, QModelIndex const & /*parent*/ ) const
{
  return createIndex( row, column, 0 );
}

QModelIndex ProgramsModel::parent( QModelIndex const & /*parent*/ ) const
{
  return QModelIndex();
}

Qt::ItemFlags ProgramsModel::flags( QModelIndex const & index ) const
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

int ProgramsModel::rowCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return programs.size();
}

int ProgramsModel::columnCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return 5;
}

QVariant ProgramsModel::headerData( int section, Qt::Orientation /*orientation*/, int role ) const
{
  if ( role == Qt::DisplayRole )
    switch( section )
    {
      case 0:
        return tr( "Enabled" );
      case 1:
        return tr( "Type" );
      case 2:
        return tr( "Name" );
      case 3:
        return tr( "Command Line" );
      case 4:
        return tr( "Icon" );
      default:
        return QVariant();
    }

  return QVariant();
}

QVariant ProgramsModel::data( QModelIndex const & index, int role ) const
{
  if ( index.row() >= programs.size() )
    return QVariant();

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    switch( index.column() )
    {
      case 1:
        if ( role == Qt::DisplayRole )
          return ProgramTypeEditor::getNameForType( programs[ index.row() ].type );
        else
          return QVariant( ( int ) programs[ index.row() ].type );
      case 2:
        return programs[ index.row() ].name;
      case 3:
        return programs[ index.row() ].commandLine;
      case 4:
        return programs[ index.row() ].iconFilename;
      default:
        return QVariant();
    }
  }

  if ( role == Qt::CheckStateRole && !index.column() )
    return programs[ index.row() ].enabled ? Qt::Checked : Qt::Unchecked;

  return QVariant();
}

bool ProgramsModel::setData( QModelIndex const & index, const QVariant & value,
                             int role )
{
  if ( index.row() >= programs.size() )
    return false;

  if ( role == Qt::CheckStateRole && !index.column() )
  {
    programs[ index.row() ].enabled = !programs[ index.row() ].enabled;

    dataChanged( index, index );
    return true;
  }

  if ( role == Qt::DisplayRole || role == Qt::EditRole )
    switch( index.column() )
    {
      case 1:
        programs[ index.row() ].type = Config::Program::Type( value.toInt() );
        dataChanged( index, index );
        return true;
      case 2:
        programs[ index.row() ].name = value.toString();
        dataChanged( index, index );
        return true;
      case 3:
        programs[ index.row() ].commandLine = value.toString();
        dataChanged( index, index );
        return true;
      case 4:
        programs[ index.row() ].iconFilename = value.toString();
        dataChanged( index, index );
        return true;
      default:
        return false;
    }

  return false;
}

QString ProgramTypeEditor::getNameForType( int v )
{
  switch( v )
  {
    case Config::Program::Audio:
      return tr( "Audio" );
    case Config::Program::PlainText:
      return tr( "Plain Text" );
    case Config::Program::Html:
      return tr( "Html" );
    case Config::Program::PrefixMatch:
      return tr( "Prefix Match" );
    default:
      return tr( "Unknown" );
  }
}

ProgramTypeEditor::ProgramTypeEditor( QWidget * widget ): QComboBox( widget )
{
  for( int x = 0; x < Config::Program::MaxTypeValue; ++x )
    addItem( getNameForType( x ) );
}

int ProgramTypeEditor::getType() const
{
  return currentIndex();
}

void ProgramTypeEditor::setType( int t )
{
  setCurrentIndex( t );
}

////////// PathsModel

PathsModel::PathsModel( QWidget * parent,
                        Config::Paths const & paths_ ):
  QAbstractItemModel( parent ), paths( paths_ )
{
}

void PathsModel::removePath( int index )
{
  beginRemoveRows( QModelIndex(), index, index );
  paths.erase( paths.begin() + index );
  endRemoveRows();
}

void PathsModel::addNewPath( QString const & path )
{
  beginInsertRows( QModelIndex(), paths.size(), paths.size() );
  paths.push_back( Config::Path( path, false ) );
  endInsertRows();
}

QModelIndex PathsModel::index( int row, int column, QModelIndex const & /*parent*/ ) const
{
  return createIndex( row, column, 0 );
}

QModelIndex PathsModel::parent( QModelIndex const & /*parent*/ ) const
{
  return QModelIndex();
}

Qt::ItemFlags PathsModel::flags( QModelIndex const & index ) const
{
  Qt::ItemFlags result = QAbstractItemModel::flags( index );

  if ( index.isValid() && index.column() == 1 )
    result |= Qt::ItemIsUserCheckable;

  return result;
}

int PathsModel::rowCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return paths.size();
}

int PathsModel::columnCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return 2;
}

QVariant PathsModel::headerData( int section, Qt::Orientation /*orientation*/, int role ) const
{
  if ( role == Qt::DisplayRole )
    switch( section )
    {
      case 0:
        return tr( "Path" );
      case 1:
        return tr( "Recursive" );
      default:
        return QVariant();
    }

  return QVariant();
}

QVariant PathsModel::data( QModelIndex const & index, int role ) const
{
  if ( index.row() >= paths.size() )
    return QVariant();

  if ( ( role == Qt::DisplayRole || role == Qt::EditRole ) && !index.column() )
    return paths[ index.row() ].path;

  if ( role == Qt::CheckStateRole && index.column() == 1 )
    return paths[ index.row() ].recursive ? Qt::Checked : Qt::Unchecked;

  return QVariant();
}

bool PathsModel::setData( QModelIndex const & index, const QVariant & /*value*/,
                          int role )
{
  if ( index.row() >= paths.size() )
    return false;

  if ( role == Qt::CheckStateRole && index.column() == 1 )
  {
    paths[ index.row() ].recursive = !paths[ index.row() ].recursive;

    dataChanged( index, index );
    return true;
  }

  return false;
}


////////// SoundDirsModel

SoundDirsModel::SoundDirsModel( QWidget * parent,
                                Config::SoundDirs const & soundDirs_ ):
  QAbstractItemModel( parent ), soundDirs( soundDirs_ )
{
}

void SoundDirsModel::removeSoundDir( int index )
{
  beginRemoveRows( QModelIndex(), index, index );
  soundDirs.erase( soundDirs.begin() + index );
  endRemoveRows();
}

void SoundDirsModel::addNewSoundDir( QString const & path, QString const & name )
{
  beginInsertRows( QModelIndex(), soundDirs.size(), soundDirs.size() );
  soundDirs.push_back( Config::SoundDir( path, name ) );
  endInsertRows();
}

QModelIndex SoundDirsModel::index( int row, int column, QModelIndex const & /*parent*/ ) const
{
  return createIndex( row, column, 0 );
}

QModelIndex SoundDirsModel::parent( QModelIndex const & /*parent*/ ) const
{
  return QModelIndex();
}

Qt::ItemFlags SoundDirsModel::flags( QModelIndex const & index ) const
{
  Qt::ItemFlags result = QAbstractItemModel::flags( index );

  if ( index.isValid() && index.column() < 3 )
    result |= Qt::ItemIsEditable;

  return result;
}

int SoundDirsModel::rowCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return soundDirs.size();
}

int SoundDirsModel::columnCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return 3;
}

QVariant SoundDirsModel::headerData( int section, Qt::Orientation /*orientation*/, int role ) const
{
  if ( role == Qt::DisplayRole )
    switch( section )
    {
      case 0:
        return tr( "Path" );
      case 1:
        return tr( "Name" );
      case 2:
        return tr( "Icon" );
      default:
        return QVariant();
    }

  return QVariant();
}

QVariant SoundDirsModel::data( QModelIndex const & index, int role ) const
{
  if ( index.row() >= soundDirs.size() )
    return QVariant();

  if ( ( role == Qt::DisplayRole || role == Qt::EditRole ) && !index.column() )
    return soundDirs[ index.row() ].path;

  if ( ( role == Qt::DisplayRole || role == Qt::EditRole ) && index.column() == 1 )
    return soundDirs[ index.row() ].name;

  if ( ( role == Qt::DisplayRole || role == Qt::EditRole ) && index.column() == 2 )
    return soundDirs[ index.row() ].iconFilename;

  return QVariant();
}

bool SoundDirsModel::setData( QModelIndex const & index, const QVariant & value,
                              int role )
{
  if ( index.row() >= soundDirs.size() )
    return false;

  if ( ( role == Qt::DisplayRole || role == Qt::EditRole ) && index.column() < 3 )
  {
    if ( !index.column() )
      soundDirs[ index.row() ].path = value.toString();
    else if ( index.column() == 1 )
      soundDirs[ index.row() ].name = value.toString();
    else
      soundDirs[ index.row() ].iconFilename = value.toString();

    dataChanged( index, index );
    return true;
  }

  return false;
}


////////// HunspellDictsModel

HunspellDictsModel::HunspellDictsModel( QWidget * parent,
                                        Config::Hunspell const & hunspell ):
  QAbstractItemModel( parent ), enabledDictionaries( hunspell.enabledDictionaries )
{
  changePath( hunspell.dictionariesPath );
}

void HunspellDictsModel::changePath( QString const & newPath )
{
  dataFiles = HunspellMorpho::findDataFiles( newPath );
  reset();
}

QModelIndex HunspellDictsModel::index( int row, int column, QModelIndex const & /*parent*/ ) const
{
  return createIndex( row, column, 0 );
}

QModelIndex HunspellDictsModel::parent( QModelIndex const & /*parent*/ ) const
{
  return QModelIndex();
}

Qt::ItemFlags HunspellDictsModel::flags( QModelIndex const & index ) const
{
  Qt::ItemFlags result = QAbstractItemModel::flags( index );

  if ( index.isValid() )
  {
    if ( !index.column() )
      result |= Qt::ItemIsUserCheckable;
  }

  return result;
}

int HunspellDictsModel::rowCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return dataFiles.size();
}

int HunspellDictsModel::columnCount( QModelIndex const & parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return 2;
}

QVariant HunspellDictsModel::headerData( int section, Qt::Orientation /*orientation*/, int role ) const
{
  if ( role == Qt::DisplayRole )
    switch( section )
    {
      case 0:
        return tr( "Enabled" );
      case 1:
        return tr( "Name" );
      default:
        return QVariant();
    }

  return QVariant();
}

QVariant HunspellDictsModel::data( QModelIndex const & index, int role ) const
{
  if ( (unsigned)index.row() >= dataFiles.size() )
    return QVariant();

  if ( role == Qt::DisplayRole && index.column() == 1 )
    return dataFiles[ index.row() ].dictName;

  if ( role == Qt::CheckStateRole && !index.column() )
  {
    for( unsigned x = enabledDictionaries.size(); x--; )
    {
      if ( enabledDictionaries[ x ] == dataFiles[ index.row() ].dictId )
        return Qt::Checked;
    }

    return Qt::Unchecked;
  }

  return QVariant();
}

bool HunspellDictsModel::setData( QModelIndex const & index, const QVariant & /*value*/,
                               int role )
{
  if ( (unsigned)index.row() >= dataFiles.size() )
    return false;

  if ( role == Qt::CheckStateRole && !index.column() )
  {
    for( unsigned x = enabledDictionaries.size(); x--; )
    {
      if ( enabledDictionaries[ x ] == dataFiles[ index.row() ].dictId )
      {
        // Remove it now
        enabledDictionaries.erase( enabledDictionaries.begin() + x );
        dataChanged( index, index );
        return true;
      }
    }

    // Add it

    enabledDictionaries.push_back( dataFiles[ index.row() ].dictId );

    dataChanged( index, index );
    return true;
  }

  return false;
}

void Sources::on_rescan_clicked()
{
  emit rescan();
}

