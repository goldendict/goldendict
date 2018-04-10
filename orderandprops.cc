/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "orderandprops.hh"
#include "instances.hh"
#include "langcoder.hh"
#include "language.hh"
#include "fsencoding.hh"
#include <algorithm>

#include <QMenu>
#include <QPair>

using std::vector;
using std::sort;

namespace {

bool dictNameLessThan( sptr< Dictionary::Class > const & dict1,
                       sptr< Dictionary::Class > const & dict2 )
{
  QString str1 = QString::fromUtf8( dict1->getName().c_str() );
  QString str2 = QString::fromUtf8( dict2->getName().c_str() );
  if( str1.isEmpty() && !str2.isEmpty() )
    return false;
  if( !str1.isEmpty() && str2.isEmpty() )
    return true;

  return str1.localeAwareCompare( str2 ) < 0;
}

bool dictLessThan( sptr< Dictionary::Class > const & dict1,
                   sptr< Dictionary::Class > const & dict2 )
{
  int idFrom1 = dict1->getLangFrom();
  int idTo1 = dict1->getLangTo();
  if( idFrom1 == 0)
  {
    QPair<quint32,quint32> ids = LangCoder::findIdsForName( QString::fromUtf8( dict1->getName().c_str() ) );
    idFrom1 = ids.first;
    idTo1 = ids.second;
  }

  int idFrom2 = dict2->getLangFrom();
  int idTo2 = dict2->getLangTo();
  if( idFrom2 == 0)
  {
    QPair<quint32,quint32> ids = LangCoder::findIdsForName( QString::fromUtf8( dict2->getName().c_str() ) );
    idFrom2 = ids.first;
    idTo2 = ids.second;
  }

  QString str1 = LangCoder::decode( idFrom1 );
  QString str2 = LangCoder::decode( idFrom2 );
  if( str1.isEmpty() && !str2.isEmpty() )
    return false;
  if( !str1.isEmpty() && str2.isEmpty() )
    return true;
  int res = str1.localeAwareCompare( str2 );
  if( res )
    return res < 0;

  str1 = LangCoder::decode( idTo1 );
  str2 = LangCoder::decode( idTo2 );
  if( str1.isEmpty() && !str2.isEmpty() )
    return false;
  if( !str1.isEmpty() && str2.isEmpty() )
    return true;
  res = str1.localeAwareCompare( str2 );
  if( res )
    return res < 0;

  str1 = QString::fromUtf8( dict1->getName().c_str() );
  str2 = QString::fromUtf8( dict2->getName().c_str() );
  if( str1.isEmpty() && !str2.isEmpty() )
    return false;
  if( !str1.isEmpty() && str2.isEmpty() )
    return true;

  return str1.localeAwareCompare( str2 ) < 0;
}

} // namespace

OrderAndProps::OrderAndProps( QWidget * parent,
                              Config::Group const & dictionaryOrder,
                              Config::Group const & inactiveDictionaries,
                              std::vector< sptr< Dictionary::Class > > const &
                              allDictionaries ):
  QWidget( parent )
{
  ui.setupUi( this );

  Instances::Group order( dictionaryOrder, allDictionaries, Config::Group() );
  Instances::Group inactive( inactiveDictionaries, allDictionaries, Config::Group() );

  Instances::complementDictionaryOrder( order, inactive, allDictionaries );

  ui.dictionaryOrder->populate( order.dictionaries, allDictionaries );
  ui.inactiveDictionaries->populate( inactive.dictionaries, allDictionaries );

  ui.searchLine->applyTo( ui.dictionaryOrder );
  addAction( ui.searchLine->getFocusAction() );

  disableDictionaryDescription();

  ui.dictionaryOrder->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( ui.dictionaryOrder, SIGNAL( customContextMenuRequested( QPoint ) ),
           this, SLOT( contextMenuRequested( QPoint ) ) );

  connect( ui.dictionaryOrder, SIGNAL( gotFocus() ),
           this, SLOT( dictListFocused() ) );
  connect( ui.inactiveDictionaries, SIGNAL( gotFocus() ),
           this, SLOT( inactiveDictListFocused() ) );

  connect ( ui.dictionaryOrder->selectionModel(), SIGNAL( selectionChanged ( const QItemSelection & , const QItemSelection & ) ),
      this, SLOT( dictionarySelectionChanged( const QItemSelection & ) ) );
  connect ( ui.inactiveDictionaries->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ),
      this, SLOT( inactiveDictionarySelectionChanged( QItemSelection const & ) ) );

  connect (ui.searchLine, SIGNAL( filterChanged( QString const & ) ),
      this, SLOT( filterChanged( QString const &) ) );

  connect( ui.dictionaryOrder->getModel(), SIGNAL( contentChanged() ),
           this, SLOT( showDictNumbers() ) );
  connect( ui.inactiveDictionaries->getModel(), SIGNAL( contentChanged() ),
           this, SLOT( showDictNumbers() ) );

  showDictNumbers();
}

Config::Group OrderAndProps::getCurrentDictionaryOrder() const
{
  Instances::Group g( "" );

  g.dictionaries = ui.dictionaryOrder->getCurrentDictionaries();

  return g.makeConfigGroup();
}

Config::Group OrderAndProps::getCurrentInactiveDictionaries() const
{
  Instances::Group g( "" );

  g.dictionaries = ui.inactiveDictionaries->getCurrentDictionaries();

  return g.makeConfigGroup();
}

void OrderAndProps::filterChanged( QString const & filterText)
{
  // when the filter is active, disable the possibility
  // to drop dictionaries to this filtered list
  ui.dictionaryOrder->setAcceptDrops(filterText.isEmpty());
}

void OrderAndProps::dictListFocused()
{
  describeDictionary( ui.dictionaryOrder, ui.searchLine->mapToSource( ui.dictionaryOrder->currentIndex() ) );
}

void OrderAndProps::inactiveDictListFocused()
{
  describeDictionary( ui.inactiveDictionaries, ui.inactiveDictionaries->currentIndex() );
}

void OrderAndProps::dictionarySelectionChanged( const QItemSelection & current )
{
  if ( current.empty() )
    return;

  describeDictionary( ui.dictionaryOrder, ui.searchLine->mapToSource( current.front().topLeft() ) );
}

void OrderAndProps::inactiveDictionarySelectionChanged( QItemSelection const & current )
{
  if ( current.empty() )
    return;
  describeDictionary( ui.inactiveDictionaries, current.front().topLeft() );
}

void OrderAndProps::disableDictionaryDescription()
{
  ui.dictionaryInformation->setEnabled( false );

  ui.dictionaryName->clear();
  ui.dictionaryTotalArticles->clear();
  ui.dictionaryTotalWords->clear();
  ui.dictionaryTranslatesFrom->clear();
  ui.dictionaryTranslatesTo->clear();
  ui.dictionaryFileList->clear();

  ui.dictionaryDescription->clear();
  ui.dictionaryDescription->setVisible( false );
  ui.dictionaryDescriptionLabel->setVisible( false );
  ui.infoVerticalSpacer->changeSize( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
  ui.infoVerticalLayout->invalidate();
}

void OrderAndProps::describeDictionary( DictListWidget * lst, QModelIndex const & idx )
{
  if ( !idx.isValid() || (unsigned) idx.row() >= lst->getCurrentDictionaries().size() )
    disableDictionaryDescription();
  else
  {
    sptr< Dictionary::Class > dict = lst->getCurrentDictionaries()[ idx.row() ];

    if ( !dict )
    {
      return;
    }

    ui.dictionaryInformation->setEnabled( true );

    ui.dictionaryName->setText( QString::fromUtf8( dict->getName().c_str() ) );

    ui.dictionaryTotalArticles->setText( QString::number( dict->getArticleCount() ) );
    ui.dictionaryTotalWords->setText( QString::number( dict->getWordCount() ) );
    ui.dictionaryTranslatesFrom->setText( Language::localizedStringForId( dict->getLangFrom() ) );
    ui.dictionaryTranslatesTo->setText( Language::localizedStringForId( dict->getLangTo() ) );

    std::vector< std::string > const & filenames = dict->getDictionaryFilenames();

    QString filenamesText;

    for( unsigned x = 0; x < filenames.size(); x++ )
    {
      filenamesText += FsEncoding::decode( filenames[ x ].c_str() );
      filenamesText += '\n';
    }

    ui.dictionaryFileList->setPlainText( filenamesText );

    QString const& descText = dict->getDescription();
    if( !descText.isEmpty() && descText.compare( "NONE" ) != 0 )
    {
      ui.dictionaryDescription->setPlainText( descText );
      ui.dictionaryDescription->setVisible( true );
      ui.dictionaryDescriptionLabel->setVisible( true );
      ui.infoVerticalSpacer->changeSize( 0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum );
    }
    else
    {
      ui.dictionaryDescription->setVisible( false );
      ui.dictionaryDescriptionLabel->setVisible( false );
      ui.infoVerticalSpacer->changeSize( 20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    }
    ui.infoVerticalLayout->invalidate();
  }
}

void OrderAndProps::contextMenuRequested( const QPoint & pos )
{
  QMenu menu( this );
  QAction * sortNameAction = new QAction( tr( "Sort by name" ), &menu );
  menu.addAction( sortNameAction );
  QAction * sortLangAction = new QAction( tr( "Sort by languages" ), &menu );
  menu.addAction( sortLangAction );

  QAction * showHeadwordsAction = NULL;

  QModelIndex idx = ui.searchLine->mapToSource( ui.dictionaryOrder->indexAt( pos ) );
  sptr< Dictionary::Class > dict;
  if( idx.isValid() && (unsigned)idx.row() < ui.dictionaryOrder->getCurrentDictionaries().size() )
    dict = ui.dictionaryOrder->getCurrentDictionaries()[ idx.row() ];
  if ( dict && dict->getWordCount() > 0 )
  {
    showHeadwordsAction = new QAction( tr( "Dictionary headwords" ), &menu );
    menu.addAction( showHeadwordsAction );
  }

  QAction * result = menu.exec( ui.dictionaryOrder->mapToGlobal( pos ) );

  if( result == sortNameAction || result == sortLangAction )
  {
    vector< sptr< Dictionary::Class > > sortedDicts = ui.dictionaryOrder->getCurrentDictionaries();
    if( result == sortNameAction )
      sort( sortedDicts.begin(), sortedDicts.end(), dictNameLessThan );
    else
      sort( sortedDicts.begin(), sortedDicts.end(), dictLessThan );
    ui.dictionaryOrder->populate( sortedDicts );
  }

  if( result && result == showHeadwordsAction )
  {
    emit showDictionaryHeadwords( QString::fromUtf8( dict->getId().c_str() ) );
  }
}

void OrderAndProps::showDictNumbers()
{
  ui.dictionariesNumber->setText( tr( "Dictionaries active: %1, inactive: %2" )
                                  .arg( QString::number( ui.dictionaryOrder->getModel()->rowCount( QModelIndex() ) ) )
                                  .arg( QString::number( ui.inactiveDictionaries->getModel()->rowCount( QModelIndex() ) ) ) );
}
