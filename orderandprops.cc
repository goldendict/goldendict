/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "orderandprops.hh"
#include "instances.hh"
#include "langcoder.hh"
#include "language.hh"
#include "fsencoding.hh"
#include <algorithm>

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

  return str1.compare( str2, Qt::CaseInsensitive ) < 0;
}

bool dictLessThan( sptr< Dictionary::Class > const & dict1,
                   sptr< Dictionary::Class > const & dict2 )
{
  QString str1 = LangCoder::decode( dict1->getLangFrom() );
  QString str2 = LangCoder::decode( dict2->getLangFrom() );
  if( str1.isEmpty() && !str2.isEmpty() )
    return false;
  if( !str1.isEmpty() && str2.isEmpty() )
    return true;
  int res = str1.compare( str2, Qt::CaseInsensitive );
  if( res )
    return res < 0;

  str1 = LangCoder::decode( dict1->getLangTo() );
  str2 = LangCoder::decode( dict2->getLangTo() );
  if( str1.isEmpty() && !str2.isEmpty() )
    return false;
  if( !str1.isEmpty() && str2.isEmpty() )
    return true;
  res = str1.compare( str2, Qt::CaseInsensitive );
  if( res )
    return res < 0;

  str1 = QString::fromUtf8( dict1->getName().c_str() );
  str2 = QString::fromUtf8( dict2->getName().c_str() );
  if( str1.isEmpty() && !str2.isEmpty() )
    return false;
  if( !str1.isEmpty() && str2.isEmpty() )
    return true;

  return str1.compare( str2, Qt::CaseInsensitive ) < 0;
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

  // For now we don't support arrows, so remove them until we get to that
  delete ui.moveActiveUp;
  delete ui.moveActiveDown;

  delete ui.moveToActive;
  delete ui.moveToInactive;

  Instances::Group order( dictionaryOrder, allDictionaries );
  Instances::Group inactive( inactiveDictionaries, allDictionaries );

  Instances::complementDictionaryOrder( order, inactive, allDictionaries );

  ui.dictionaryOrder->populate( order.dictionaries, allDictionaries );
  ui.inactiveDictionaries->populate( inactive.dictionaries, allDictionaries );

  disableDictionaryDescription();

  ui.dictionaryOrder->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( ui.dictionaryOrder, SIGNAL( customContextMenuRequested( QPoint ) ),
           this, SLOT( contextMenuRequested( QPoint ) ) );
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

void OrderAndProps::on_dictionaryOrder_clicked( QModelIndex const & idx )
{
  describeDictionary( ui.dictionaryOrder, idx );
}

void OrderAndProps::on_inactiveDictionaries_clicked( QModelIndex const & idx )
{
  describeDictionary( ui.inactiveDictionaries, idx );
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
}

void OrderAndProps::describeDictionary( DictListWidget * lst, QModelIndex const & idx )
{
  if ( !idx.isValid() || (unsigned) idx.row() >= lst->getCurrentDictionaries().size() )
    disableDictionaryDescription();
  else
  {
    sptr< Dictionary::Class > dict = lst->getCurrentDictionaries()[ idx.row() ];

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
    }
    else
    {
      ui.dictionaryDescription->setVisible( false );
      ui.dictionaryDescriptionLabel->setVisible( false );
    }
  }
}

void OrderAndProps::contextMenuRequested( const QPoint & pos )
{
  QMenu menu( this );
  QAction * sortNameAction = new QAction( tr( "Sort by name" ), &menu );
  menu.addAction( sortNameAction );
  QAction * sortLangAction = new QAction( tr( "Sort by languages" ), &menu );
  menu.addAction( sortLangAction );

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
}
