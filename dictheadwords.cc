/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "dictheadwords.hh"
#include "gddebug.hh"
#include "mainwindow.hh"

#include <QRegExp>
#include <QDir>
#include <QFileDialog>
#include <QTimer>
#include <QProgressDialog>

#define AUTO_APPLY_LIMIT 150000

DictHeadwords::DictHeadwords( QWidget *parent, Config::Class & cfg_,
                              Dictionary::Class * dict_ ) :
  QDialog(parent)
, cfg( cfg_ )
, dict( dict_ )
, helpAction( this )
{
  ui.setupUi( this );

  if( cfg.headwordsDialog.headwordsDialogGeometry.size() > 0 )
    restoreGeometry( cfg.headwordsDialog.headwordsDialogGeometry );

  bool fromMainWindow = parent->objectName() == "MainWindow";

  if( fromMainWindow )
    setAttribute( Qt::WA_DeleteOnClose, false );

  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

  ui.searchModeCombo->addItem( tr( "Text" ), QRegExp::FixedString );
  ui.searchModeCombo->addItem( tr( "Wildcards" ), QRegExp::WildcardUnix );
  ui.searchModeCombo->addItem( tr( "RegExp" ), QRegExp::RegExp );
  ui.searchModeCombo->setCurrentIndex( cfg.headwordsDialog.searchMode );

  ui.exportButton->setAutoDefault( false );
  ui.OKButton->setAutoDefault( false);
  ui.applyButton->setAutoDefault( true );
  ui.applyButton->setDefault( true );

  ui.matchCase->setChecked( cfg.headwordsDialog.matchCase );

  model = new QStringListModel( this );
  model->setStringList( headers );

  proxy = new QSortFilterProxyModel( this );

  proxy->setSourceModel( model );

  proxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  proxy->setSortLocaleAware( true );
  proxy->setDynamicSortFilter( false );

  ui.headersListView->setModel( proxy );
  ui.headersListView->setEditTriggers( QAbstractItemView::NoEditTriggers );

  // very important call, for performance reasons:
  ui.headersListView->setUniformItemSizes( true );

  delegate = new WordListItemDelegate( ui.headersListView->itemDelegate() );
  if( delegate )
    ui.headersListView->setItemDelegate( delegate );

  ui.autoApply->setChecked( cfg.headwordsDialog.autoApply );

  connect( this, SIGNAL( finished( int ) ), this, SLOT( savePos() ) );

  if( !fromMainWindow )
  {
    ui.helpButton->hide();
    connect( this, SIGNAL( closeDialog() ), this, SLOT( accept() ) );
  }
  else
  {
    connect( ui.helpButton, SIGNAL( clicked() ),
             this, SLOT( helpRequested() ) );

    helpAction.setShortcut( QKeySequence( "F1" ) );
    helpAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );

    connect( &helpAction, SIGNAL( triggered() ),
             this, SLOT( helpRequested() ) );

    addAction( &helpAction );
  }

  connect( ui.OKButton, SIGNAL( clicked( bool ) ), this, SLOT( okButtonClicked() ) );
  connect( ui.exportButton, SIGNAL( clicked( bool ) ), this, SLOT( exportButtonClicked() ) );
  connect( ui.applyButton, SIGNAL( clicked( bool ) ), this, SLOT( filterChanged() ) );

  connect( ui.autoApply, SIGNAL( stateChanged( int ) ),
           this, SLOT( autoApplyStateChanged( int ) ) );

  connect( ui.filterLine, SIGNAL( textChanged( QString ) ),
           this, SLOT( filterChangedInternal() ) );
  connect( ui.searchModeCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( filterChangedInternal() ) );
  connect( ui.matchCase, SIGNAL( stateChanged( int ) ),
           this, SLOT( filterChangedInternal() ) );

  connect( ui.headersListView, SIGNAL( clicked( QModelIndex ) ),
           this, SLOT( itemClicked( QModelIndex ) ) );

  connect( proxy, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
           this, SLOT( showHeadwordsNumber() ) );

  ui.headersListView->installEventFilter( this );

  setup( dict_ );
}

DictHeadwords::~DictHeadwords()
{
  if( delegate )
    delete delegate;
}

void DictHeadwords::setup( Dictionary::Class *dict_ )
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  dict = dict_;

  setWindowTitle( QString::fromUtf8( dict->getName().c_str() ) );

  headers.clear();
  model->setStringList( headers );

  dict->getHeadwords( headers );
  model->setStringList( headers );

  proxy->sort( 0 );
  filterChanged();

  if( headers.size() > AUTO_APPLY_LIMIT )
  {
    cfg.headwordsDialog.autoApply = ui.autoApply->isChecked();
    ui.autoApply->setChecked( false );
    ui.autoApply->setEnabled( false );
  }
  else
  {
    ui.autoApply->setEnabled( true );
    ui.autoApply->setChecked( cfg.headwordsDialog.autoApply );
  }

  ui.applyButton->setEnabled( !ui.autoApply->isChecked() );

  setWindowIcon( dict->getIcon() );

  dictId = QString( dict->getId().c_str() );

  QApplication::restoreOverrideCursor();
}

void DictHeadwords::savePos()
{
  cfg.headwordsDialog.searchMode = ui.searchModeCombo->currentIndex();
  cfg.headwordsDialog.matchCase = ui.matchCase->isChecked();

  if( headers.size() <= AUTO_APPLY_LIMIT )
    cfg.headwordsDialog.autoApply = ui.autoApply->isChecked();

  cfg.headwordsDialog.headwordsDialogGeometry = saveGeometry();
}

bool DictHeadwords::eventFilter( QObject * obj, QEvent * ev )
{
  if( obj == ui.headersListView && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent * kev = static_cast< QKeyEvent * >( ev );
    if( kev->key() == Qt::Key_Return || kev->key() == Qt::Key_Enter )
    {
      itemClicked( ui.headersListView->currentIndex() );
      return true;
    }
  }
  return QDialog::eventFilter( obj, ev );
}

void DictHeadwords::okButtonClicked()
{
  savePos();
  closeDialog();
}

void DictHeadwords::reject()
{
  savePos();
  closeDialog();
}

void DictHeadwords::exportButtonClicked()
{
  saveHeadersToFile();
}

void DictHeadwords::filterChangedInternal()
{
  // emit signal in async manner, to avoid UI slowdown
  if( ui.autoApply->isChecked() )
    QTimer::singleShot( 100, this, SLOT( filterChanged() ) );
}

void DictHeadwords::filterChanged()
{
  QRegExp::PatternSyntax syntax =
          QRegExp::PatternSyntax( ui.searchModeCombo->itemData(
                  ui.searchModeCombo->currentIndex()).toInt() );

  Qt::CaseSensitivity caseSensitivity = ui.matchCase->isChecked() ? Qt::CaseSensitive
                                                                    : Qt::CaseInsensitive;

  QRegExp regExp( ui.filterLine->text(), caseSensitivity, syntax );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  proxy->setFilterRegExp( regExp );
  proxy->sort( 0 );

  QApplication::restoreOverrideCursor();

  showHeadwordsNumber();
}

void DictHeadwords::itemClicked( const QModelIndex & index )
{
  QVariant value = proxy->data( index, Qt::DisplayRole );
  if ( value.canConvert< QString >() )
  {
    QString headword = value.toString();
    emit headwordSelected( headword, dictId );
  }
}

void DictHeadwords::autoApplyStateChanged( int state )
{
  ui.applyButton->setEnabled( state == Qt::Unchecked );
}

void DictHeadwords::showHeadwordsNumber()
{
  ui.headersNumber->setText( tr( "Unique headwords total: %1, filtered: %2" )
                             .arg( QString::number( headers.size() ) )
                             .arg( QString::number( proxy->rowCount() ) ) );
}

void DictHeadwords::saveHeadersToFile()
{
  QString exportPath;
  if( cfg.headwordsDialog.headwordsExportPath.isEmpty() )
    exportPath = QDir::homePath();
  else
  {
    exportPath = QDir::fromNativeSeparators( cfg.headwordsDialog.headwordsExportPath );
    if( !QDir( exportPath ).exists() )
      exportPath = QDir::homePath();
  }

  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save headwords to file" ),
                                                   exportPath,
                                                   tr( "Text files (*.txt);;All files (*.*)" ) );
  if( fileName.size() == 0)
      return;

  cfg.headwordsDialog.headwordsExportPath = QDir::toNativeSeparators(
                                              QFileInfo( fileName ).absoluteDir().absolutePath() );
  QFile file( fileName );

  for(;;)
  {
    if ( !file.open( QFile::WriteOnly | QIODevice::Text ) )
      break;

    int headwordsNumber = proxy->rowCount();

    // Setup progress dialog
    int n = headwordsNumber;
    int step = 1;
    while( n > 1000 )
    {
      step *= 10;
      n /= 10;
    }

    QProgressDialog progress( tr( "Export headwords..."), tr( "Cancel" ),
                              0, n, this );
    progress.setWindowModality( Qt::WindowModal );

    // Write UTF-8 BOM
    QByteArray line;
    line.append( 0xEF ).append( 0xBB ).append( 0xBF );
    if ( file.write( line ) != line.size() )
      break;

    // Write headwords

    int i;
    for( i = 0; i < headwordsNumber; ++i )
    {
      if( i % step == 0 )
        progress.setValue( i / step );

      if( progress.wasCanceled() )
        break;

      QVariant value = proxy->data( proxy->index( i, 0 ) );
      if( !value.canConvert< QString >() )
        continue;

      line = value.toString().toUtf8();

      line.replace( '\n', ' ' );
      line.replace( '\r', ' ' );

      line += "\n";

      if ( file.write( line ) != line.size() )
        break;
    }

    if( i < headwordsNumber && !progress.wasCanceled() )
      break;

    file.close();
    return;
  }

  gdWarning( "Headers export error: %s", file.errorString().toUtf8().data() );
  file.close();
}

void DictHeadwords::helpRequested()
{
  MainWindow * mainWindow = qobject_cast< MainWindow * >( parentWidget() );
  if( mainWindow )
    mainWindow->showGDHelpForID( "Dictionary headwords" );
}
