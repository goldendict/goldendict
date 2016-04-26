/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "fulltextsearch.hh"
#include "ftshelpers.hh"
#include "gddebug.hh"
#include "mainwindow.hh"
#include "qt4x5.hh"

#include <QThreadPool>
#include <QIntValidator>
#include <QMessageBox>
#include <qalgorithms.h>

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) ) && defined( Q_OS_WIN32 )

#include <QtWidgets/QStyleFactory>
#include <qt_windows.h>
#include <uxtheme.h>

#endif

namespace FTS
{

enum
{
  MinDistanceBetweenWords = 0,
  MaxDistanceBetweenWords = 15,
  MinArticlesPerDictionary = 1,
  MaxArticlesPerDictionary = 10000
};

void Indexing::run()
{
  try
  {
    // First iteration - dictionaries with no more MaxDictionarySizeForFastSearch articles
    for( size_t x = 0; x < dictionaries.size(); x++ )
    {
      if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
        break;

      if( dictionaries.at( x )->canFTS()
          &&!dictionaries.at( x )->haveFTSIndex() )
      {
        emit sendNowIndexingName( QString::fromUtf8( dictionaries.at( x )->getName().c_str() ) );
        dictionaries.at( x )->makeFTSIndex( isCancelled, true );
      }
    }

    // Second iteration - all remaining dictionaries
    for( size_t x = 0; x < dictionaries.size(); x++ )
    {
      if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
        break;

      if( dictionaries.at( x )->canFTS()
          &&!dictionaries.at( x )->haveFTSIndex() )
      {
        emit sendNowIndexingName( QString::fromUtf8( dictionaries.at( x )->getName().c_str() ) );
        dictionaries.at( x )->makeFTSIndex( isCancelled, false );
      }
    }
  }
  catch( std::exception &ex )
  {
    gdWarning( "Exception occured while full-text search: %s", ex.what() );
  }
  emit sendNowIndexingName( QString() );
}


FtsIndexing::FtsIndexing( std::vector< sptr< Dictionary::Class > > const & dicts):
  dictionaries( dicts ),
  started( false )
{
}

void FtsIndexing::doIndexing()
{
  if( started )
    stopIndexing();

  if( !started )
  {
    while( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      isCancelled.deref();

    Indexing *idx = new Indexing( isCancelled, dictionaries, indexingExited );

    connect( idx, SIGNAL( sendNowIndexingName( QString ) ), this, SLOT( setNowIndexedName( QString ) ) );

    QThreadPool::globalInstance()->start( idx );

    started = true;
  }
}

void FtsIndexing::stopIndexing()
{
  if( started )
  {
    if( !Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      isCancelled.ref();

    indexingExited.acquire();
    started = false;

    setNowIndexedName( QString() );
  }
}

void FtsIndexing::setNowIndexedName( QString name )
{
  {
    Mutex::Lock _( nameMutex );
    nowIndexing = name;
  }
  emit newIndexingName( name );
}

QString FtsIndexing::nowIndexingName()
{
  Mutex::Lock _( nameMutex );
  return nowIndexing;
}

FullTextSearchDialog::FullTextSearchDialog( QWidget * parent,
                                            Config::Class & cfg_,
                                            std::vector< sptr< Dictionary::Class > > const & dictionaries_,
                                            std::vector< Instances::Group > const & groups_,
                                            FtsIndexing & ftsidx ) :
  QDialog( parent ),
  cfg( cfg_ ),
  dictionaries( dictionaries_ ),
  groups( groups_ ),
  group( 0 ),
  ftsIdx( ftsidx )
, helpAction( this )
{
  ui.setupUi( this );

  if( cfg.preferences.fts.dialogGeometry.size() > 0 )
    restoreGeometry( cfg.preferences.fts.dialogGeometry );

  setAttribute( Qt::WA_DeleteOnClose, false );
  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

  setWindowTitle( tr( "Full-text search" ) );

  setNewIndexingName( ftsIdx.nowIndexingName() );

  connect( &ftsIdx, SIGNAL( newIndexingName( QString ) ),
           this, SLOT( setNewIndexingName( QString ) ) );

  ui.searchMode->addItem( tr( "Whole words" ), WholeWords );
  ui.searchMode->addItem( tr( "Plain text"), PlainText );
  ui.searchMode->addItem( tr( "Wildcards" ), Wildcards );
  ui.searchMode->addItem( tr( "RegExp" ), RegExp );
  ui.searchMode->setCurrentIndex( cfg.preferences.fts.searchMode );

  ui.searchProgressBar->hide();

  ui.checkBoxDistanceBetweenWords->setText( tr( "Max distance between words (%1-%2):" )
                                            .arg( QString::number( MinDistanceBetweenWords ) )
                                            .arg( QString::number( MaxDistanceBetweenWords ) ) );
  ui.checkBoxDistanceBetweenWords->setChecked( cfg.preferences.fts.useMaxDistanceBetweenWords );

  ui.distanceBetweenWords->setMinimum( MinDistanceBetweenWords );
  ui.distanceBetweenWords->setMaximum( MaxDistanceBetweenWords );
  ui.distanceBetweenWords->setValue( cfg.preferences.fts.maxDistanceBetweenWords );

  ui.checkBoxArticlesPerDictionary->setText( tr( "Max articles per dictionary (%1-%2):" )
                                             .arg( QString::number( MinArticlesPerDictionary ) )
                                             .arg( QString::number( MaxArticlesPerDictionary ) ) );
  ui.checkBoxArticlesPerDictionary->setChecked( cfg.preferences.fts.useMaxArticlesPerDictionary );

  ui.articlesPerDictionary->setMinimum( MinArticlesPerDictionary );
  ui.articlesPerDictionary->setMaximum( MaxArticlesPerDictionary );
  ui.articlesPerDictionary->setValue( cfg.preferences.fts.maxArticlesPerDictionary );

  ui.matchCase->setChecked( cfg.preferences.fts.matchCase );

  setLimitsUsing();

  connect( ui.checkBoxDistanceBetweenWords, SIGNAL( stateChanged( int ) ),
           this, SLOT( setLimitsUsing() ) );
  connect( ui.checkBoxArticlesPerDictionary, SIGNAL( stateChanged( int ) ),
           this, SLOT( setLimitsUsing() ) );
  connect( ui.searchMode, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( setLimitsUsing() ) );

  model = new HeadwordsListModel( this, results, activeDicts );
  ui.headwordsView->setModel( model );

  ui.articlesFoundLabel->setText( tr( "Articles found: " ) + "0" );

  connect( ui.headwordsView, SIGNAL( clicked( QModelIndex ) ),
           this, SLOT( itemClicked( QModelIndex ) ) );

  connect( this, SIGNAL( finished( int ) ), this, SLOT( saveData() ) );

  connect( ui.OKButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( ui.cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );

  connect( ui.helpButton, SIGNAL( clicked() ),
           this, SLOT( helpRequested() ) );

  helpAction.setShortcut( QKeySequence( "F1" ) );
  helpAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );

  connect( &helpAction, SIGNAL( triggered() ),
           this, SLOT( helpRequested() ) );

  addAction( &helpAction );

  ui.headwordsView->installEventFilter( this );

  delegate = new WordListItemDelegate( ui.headwordsView->itemDelegate() );
  if( delegate )
    ui.headwordsView->setItemDelegate( delegate );

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) ) && defined( Q_OS_WIN32 )

  // Style "windowsvista" in Qt5 turn off progress bar animation for classic appearance
  // We use simply "windows" style instead for this case

  barStyle = 0;
  oldBarStyle = 0;

  if( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA
      && ( QSysInfo::windowsVersion() & QSysInfo::WV_NT_based )
      && !IsThemeActive() )
  {
    barStyle = QStyleFactory::create( "windows" );

    if( barStyle )
    {
      oldBarStyle = ui.searchProgressBar->style();
      ui.searchProgressBar->setStyle( barStyle );
    }
  }

#endif

  ui.searchLine->setText( static_cast< MainWindow * >( parent )->getTranslateLineText() );
  ui.searchLine->selectAll();
}

FullTextSearchDialog::~FullTextSearchDialog()
{
  if( delegate )
    delete delegate;

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) ) && defined( Q_OS_WIN32 )

  if( barStyle )
  {
    ui.searchProgressBar->setStyle( oldBarStyle );
    delete barStyle;
  }

#endif
}

void FullTextSearchDialog::stopSearch()
{
  if( !searchReqs.empty() )
  {
    for( std::list< sptr< Dictionary::DataRequest > >::iterator it = searchReqs.begin();
         it != searchReqs.end(); ++it )
      if( !(*it)->isFinished() )
        (*it)->cancel();

    while( searchReqs.size() )
      QApplication::processEvents();
  }
}

void FullTextSearchDialog::showDictNumbers()
{
  ui.totalDicts->setText( QString::number( activeDicts.size() ) );

  unsigned ready = 0, toIndex = 0;
  for( unsigned x = 0; x < activeDicts.size(); x++ )
  {
    if( activeDicts.at( x )->haveFTSIndex() )
      ready++;
    else
      toIndex++;
  }

  ui.readyDicts->setText( QString::number( ready ) );
  ui.toIndexDicts->setText( QString::number( toIndex ) );
}

void FullTextSearchDialog::saveData()
{
  cfg.preferences.fts.searchMode = ui.searchMode->currentIndex();
  cfg.preferences.fts.matchCase = ui.matchCase->isChecked();
  cfg.preferences.fts.maxArticlesPerDictionary = ui.articlesPerDictionary->text().toInt();
  cfg.preferences.fts.maxDistanceBetweenWords = ui.distanceBetweenWords->text().toInt();
  cfg.preferences.fts.useMaxDistanceBetweenWords = ui.checkBoxDistanceBetweenWords->isChecked();
  cfg.preferences.fts.useMaxArticlesPerDictionary = ui.checkBoxArticlesPerDictionary->isChecked();

  cfg.preferences.fts.dialogGeometry = saveGeometry();
}

void FullTextSearchDialog::setNewIndexingName( QString name )
{
  ui.nowIndexingLabel->setText( tr( "Now indexing: " )
                                + ( name.isEmpty() ? tr( "None" ) : name ) );
  showDictNumbers();
}

void FullTextSearchDialog::setLimitsUsing()
{
  int mode = ui.searchMode->itemData( ui.searchMode->currentIndex() ).toInt();
  if( mode == WholeWords || mode == PlainText )
  {
    ui.checkBoxDistanceBetweenWords->setEnabled( true );
    ui.distanceBetweenWords->setEnabled( ui.checkBoxDistanceBetweenWords->isChecked() );
  }
  else
  {
    ui.checkBoxDistanceBetweenWords->setEnabled( false );
    ui.distanceBetweenWords->setEnabled( false );
  }
  ui.articlesPerDictionary->setEnabled( ui.checkBoxArticlesPerDictionary->isChecked() );
}

void FullTextSearchDialog::accept()
{
  QStringList list1, list2;
  int mode = ui.searchMode->itemData( ui.searchMode->currentIndex() ).toInt();

  int maxResultsPerDict = ui.checkBoxArticlesPerDictionary->isChecked() ?
                            ui.articlesPerDictionary->value() : -1;
  int distanceBetweenWords = ui.checkBoxDistanceBetweenWords->isChecked() ?
                               ui.distanceBetweenWords->value() : -1;

  model->clear();
  ui.articlesFoundLabel->setText( tr( "Articles found: " ) + QString::number( results.size() ) );

  bool hasCJK;
  if( !FtsHelpers::parseSearchString( ui.searchLine->text(), list1, list2,
                                      searchRegExp, mode,
                                      ui.matchCase->isChecked(),
                                      distanceBetweenWords,
                                      hasCJK ) )
  {
    if( hasCJK && ( mode == WholeWords || mode == PlainText ) )
    {
      QMessageBox message( QMessageBox::Warning,
                           "GoldenDict",
                           tr( "CJK symbols in search string are not compatible with search modes \"Whole words\" and \"Plain text\"" ),
                           QMessageBox::Ok,
                           this );
      message.exec();
    }
    else
    {
      QMessageBox message( QMessageBox::Warning,
                           "GoldenDict",
                           tr( "The search line must contains at least one word containing " )
                           + QString::number( MinimumWordSize ) + tr( " or more symbols" ),
                           QMessageBox::Ok,
                           this );
      message.exec();
    }
    return;
  }

  if( activeDicts.empty() )
  {
    QMessageBox message( QMessageBox::Warning,
                         "GoldenDict",
                         tr( "No dictionaries for full-text search" ),
                         QMessageBox::Ok,
                         this );
    message.exec();
    return;
  }

  ui.OKButton->setEnabled( false );
  ui.searchProgressBar->show();

  // Make search requests

  for( unsigned x = 0; x < activeDicts.size(); ++x )
  {
    sptr< Dictionary::DataRequest > req = activeDicts[ x ]->getSearchResults(
                                                              ui.searchLine->text(),
                                                              mode,
                                                              ui.matchCase->isChecked(),
                                                              distanceBetweenWords,
                                                              maxResultsPerDict
                                                            );
    connect( req.get(), SIGNAL( finished() ),
             this, SLOT( searchReqFinished() ), Qt::QueuedConnection );

    searchReqs.push_back( req );
  }

  searchReqFinished(); // Handle any ones which have already finished
}

void FullTextSearchDialog::searchReqFinished()
{
  while ( searchReqs.size() )
  {
    std::list< sptr< Dictionary::DataRequest > >::iterator it;
    for( it = searchReqs.begin(); it != searchReqs.end(); ++it )
    {
      if ( (*it)->isFinished() )
      {
        GD_DPRINTF( "one finished.\n" );

        QString errorString = (*it)->getErrorString();

        if ( (*it)->dataSize() >= 0 || errorString.size() )
        {
          QList< FtsHeadword > * headwords;
          if( (unsigned)(*it)->dataSize() >= sizeof( headwords ) )
          {
            try
            {
              (*it)->getDataSlice( 0, sizeof( headwords ), &headwords );
              model->addResults( QModelIndex(), *headwords );
              delete headwords;
              ui.articlesFoundLabel->setText( tr( "Articles found: " )
                                              + QString::number( results.size() ) );
            }
            catch( std::exception & e )
            {
              gdWarning( "getDataSlice error: %s\n", e.what() );
            }
          }

        }
        break;
      }
    }
    if( it != searchReqs.end() )
    {
      GD_DPRINTF( "erasing..\n" );
      searchReqs.erase( it );
      GD_DPRINTF( "erase done..\n" );
      continue;
    }
    else
      break;
  }
  if ( searchReqs.empty() )
  {
    ui.searchProgressBar->hide();
    ui.OKButton->setEnabled( true );
    QApplication::beep();
  }
}

void FullTextSearchDialog::reject()
{
  if( !searchReqs.empty() )
    stopSearch();
  else
  {
    saveData();
    emit closeDialog();
  }
}

void FullTextSearchDialog::itemClicked( const QModelIndex & idx )
{
  if( idx.isValid() && idx.row() < results.size() )
  {
    QString headword = results[ idx.row() ].headword;
    emit showTranslationFor( headword, results[ idx.row() ].dictIDs, searchRegExp );
  }
}

void FullTextSearchDialog::updateDictionaries()
{
  activeDicts.clear();

  // Find the given group

  Instances::Group const * activeGroup = 0;

  for( unsigned x = 0; x < groups.size(); ++x )
    if ( groups[ x ].id == group )
    {
      activeGroup = &groups[ x ];
      break;
    }

  // If we've found a group, use its dictionaries; otherwise, use the global
  // heap.
  std::vector< sptr< Dictionary::Class > > const & groupDicts =
    activeGroup ? activeGroup->dictionaries : dictionaries;

  // Exclude muted dictionaries

  Config::Group const * grp = cfg.getGroup( group );
  Config::MutedDictionaries const * mutedDicts;

  if( group == Instances::Group::AllGroupId )
    mutedDicts = &cfg.mutedDictionaries;
  else
    mutedDicts = grp ? &grp->mutedDictionaries : 0;

  if( mutedDicts && !mutedDicts->isEmpty() )
  {
    activeDicts.reserve( groupDicts.size() );
    for( unsigned x = 0; x < groupDicts.size(); ++x )
      if ( groupDicts[ x ]->canFTS()
           && !mutedDicts->contains( QString::fromStdString( groupDicts[ x ]->getId() ) )
         )
        activeDicts.push_back( groupDicts[ x ] );
  }
  else
  {
    for( unsigned x = 0; x < groupDicts.size(); ++x )
      if ( groupDicts[ x ]->canFTS() )
        activeDicts.push_back( groupDicts[ x ] );
  }

  showDictNumbers();
}

bool FullTextSearchDialog::eventFilter( QObject * obj, QEvent * ev )
{
  if( obj == ui.headwordsView && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent * kev = static_cast< QKeyEvent * >( ev );
    if( kev->key() == Qt::Key_Return || kev->key() == Qt::Key_Enter )
    {
      itemClicked( ui.headwordsView->currentIndex() );
      return true;
    }
  }
  return QDialog::eventFilter( obj, ev );
}

void FullTextSearchDialog::helpRequested()
{
  MainWindow * mainWindow = qobject_cast< MainWindow * >( parentWidget() );
  if( mainWindow )
    mainWindow->showGDHelpForID( "Full-text search" );
}

/// HeadwordsListModel

int HeadwordsListModel::rowCount( QModelIndex const & ) const
{
  return headwords.size();
}

QVariant HeadwordsListModel::data( QModelIndex const & index, int role ) const
{
  if( index.row() < 0 )
    return QVariant();

  FtsHeadword const & head = headwords[ index.row() ];

  if ( head.headword.isEmpty() )
    return QVariant();

  switch ( role )
  {
    case Qt::ToolTipRole:
    {
      QString tt;
      for( int x = 0; x < head.dictIDs.size(); x++ )
      {
        if( x != 0 )
          tt += "<br>";

        int n = getDictIndex( head.dictIDs[ x ] );
        if( n != -1 )
          tt += QString::fromUtf8( dictionaries[ n ]->getName().c_str() ) ;
      }
      return tt;
    }

    case Qt::DisplayRole :
      return head.headword;

    case Qt::EditRole :
      return head.headword;

    default:;
  }

  return QVariant();
}

void HeadwordsListModel::addResults(const QModelIndex & parent, QList< FtsHeadword > const & hws )
{
Q_UNUSED( parent );
  beginResetModel();

  QList< FtsHeadword > temp;

  for( int x = 0; x < hws.length(); x++ )
  {
    QList< FtsHeadword >::iterator it = qBinaryFind( headwords.begin(), headwords.end(), hws.at( x ) );
    if( it != headwords.end() )
      it->dictIDs.push_back( hws.at( x ).dictIDs.front() );
    else
      temp.append( hws.at( x ) );
  }

  headwords.append( temp );
  qSort( headwords );

  endResetModel();
  emit contentChanged();
}

bool HeadwordsListModel::clear()
{
  beginResetModel();

  headwords.clear();

  endResetModel();

  emit contentChanged();

  return true;
}

int HeadwordsListModel::getDictIndex( QString const & id ) const
{
  std::string dictID( id.toUtf8().data() );
  for( unsigned x = 0; x < dictionaries.size(); x++ )
  {
    if( dictionaries[ x ]->getId().compare( dictID ) == 0 )
      return x;
  }
  return -1;
}

} // namespace FTS
