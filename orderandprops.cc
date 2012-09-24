/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "orderandprops.hh"
#include "instances.hh"
#include "langcoder.hh"
#include "language.hh"
#include "fsencoding.hh"

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
