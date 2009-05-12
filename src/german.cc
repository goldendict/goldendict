/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "german.hh"
#include "transliteration.hh"
#include <QCoreApplication>

namespace GermanTranslit {

class GermanTable: public Transliteration::Table
{
public:

  GermanTable();
};

GermanTable::GermanTable()
{
  // Utf8

  ins( "ue", "ü" );
  ins( "ae", "ä" );
  ins( "oe", "ö" );
  ins( "ss", "ß" );

  ins( "UE", "Ü" );
  ins( "AE", "Ä" );
  ins( "OE", "Ö" );
  ins( "SS", "ß" );

//  ins( "ü", "ue" );
//  ins( "ä", "ae" );
//  ins( "ö", "oe" );
//  ins( "ß", "ss" );

}

sptr< Dictionary::Class > makeDictionary() throw( std::exception )
{
  static GermanTable t;

  return new Transliteration::TransliterationDictionary( "cf1b74acd98adea9b2bba16af38f1081",
                      QCoreApplication::translate( "GermanTranslit", "German Transliteration" ).toUtf8().data(), t );
}


}

