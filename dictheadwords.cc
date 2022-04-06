/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "dictheadwords.hh"
#include "gddebug.hh"
#include "mainwindow.hh"

#include <QRegExp>
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtCore5Compat>
#endif
#include <QDir>
#include <QFileDialog>
#include <QTimer>
#include <QProgressDialog>

#include <QRegularExpression>
#include "wildcard.hh"
#include "gddebug.hh"

#define AUTO_APPLY_LIMIT 150000

DictHeadwords::DictHeadwords( QWidget *parent, Config::Class & cfg_,
                              Dictionary::Class * dict_ ) :
  QDialog(parent)
, cfg( cfg_ )
, dict( dict_ )
, helpAction( this )
{
  ui.setupUi( this );

  bool fromMainWindow = parent->objectName() == "MainWindow";

  if( fromMainWindow )
    setAttribute( Qt::WA_DeleteOnClose, false );

  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

  if( cfg.headwordsDialog.headwordsDialogGeometry.size() > 0 )
    restoreGeometry( cfg.headwordsDialog.headwordsDialogGeometry );

  ui.searchModeCombo->addItem( tr( "Text" ), QRegExp::FixedString );
  ui.searchModeCombo->addItem( tr( "Wildcards" ), QRegExp::WildcardUnix );
  ui.searchModeCombo->addItem( tr( "RegExp" ), QRegExp::RegExp );
  ui.searchModeCombo->setCurrentIndex( cfg.headwordsDialog.searchMode );

  ui.exportButton->setAutoDefault( false );
  ui.OKButton->setAutoDefault( false);
  ui.applyButton->setAutoDefault( true );
  ui.applyButton->setDefault( true );

  ui.matchCase->setChecked( cfg.headwordsDialog.matchCase );

  model = new HeadwordListModel( this );

  connect(model,&HeadwordListModel::finished,this,[this](){
    ui.exportButton->setEnabled(true);
  });
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
    delegate->deleteLater();
}

void DictHeadwords::setup( Dictionary::Class *dict_ )
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  dict = dict_;

  setWindowTitle( QString::fromUtf8( dict->getName().c_str() ) );

  auto size = dict->getWordCount();
  model->setDict(dict);
  proxy->sort( 0 );
  filterChanged();

  if( size > AUTO_APPLY_LIMIT )
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

  ui.exportButton->setEnabled(false);
  ui.applyButton->setEnabled( !ui.autoApply->isChecked() );

  setWindowIcon( dict->getIcon() );

  dictId = QString( dict->getId().c_str() );

  QApplication::restoreOverrideCursor();
}

void DictHeadwords::savePos()
{
  cfg.headwordsDialog.searchMode = ui.searchModeCombo->currentIndex();
  cfg.headwordsDialog.matchCase = ui.matchCase->isChecked();

  if( model->totalCount() <= AUTO_APPLY_LIMIT )
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

  QRegularExpression::PatternOptions options = QRegularExpression::UseUnicodePropertiesOption;
  if( !ui.matchCase->isChecked() )
    options |= QRegularExpression::CaseInsensitiveOption;

  QString pattern;
  switch( syntax )
  {
  case QRegExp::FixedString:
    pattern = QRegularExpression::escape( ui.filterLine->text() );
    break;
  case QRegExp::WildcardUnix:
    pattern = wildcardsToRegexp( ui.filterLine->text() );
    break;
  default:
    pattern = ui.filterLine->text();
    break;
  }

  QRegularExpression regExp( pattern, options );

  if( !regExp.isValid() )
  {
    gdWarning( "Invalid regexp pattern: %s\n", pattern.toUtf8().data() );
    regExp.setPattern( QString::fromLatin1( "\1" ) );
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  proxy->setFilterRegularExpression( regExp );
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
                             .arg( QString::number( model->totalCount() ), QString::number( proxy->rowCount() ) ) );
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

    int headwordsNumber = model->totalCount();

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

      QVariant value = model->getRow(i);
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
