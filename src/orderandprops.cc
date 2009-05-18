/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "orderandprops.hh"
#include "instances.hh"

OrderAndProps::OrderAndProps( QWidget * parent,
                              Config::Group const & dictionaryOrder,
                              Config::Group const & inactiveDictionaries,
                              std::vector< sptr< Dictionary::Class > > const &
                              allDictionaries ):
  QWidget( parent )
{
  ui.setupUi( this );
  ui.dictionaryOrder->populate(
    Instances::Group( dictionaryOrder, allDictionaries ).dictionaries,
    allDictionaries );
  ui.inactiveDictionaries->populate(
    Instances::Group( inactiveDictionaries, allDictionaries ).dictionaries,
    allDictionaries );
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
