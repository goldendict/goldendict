/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "russiantranslit.hh"
#include "transliteration.hh"
#include <QCoreApplication>

namespace RussianTranslit {

class RussianTable: public Transliteration::Table
{
public:

  RussianTable();
};

RussianTable::RussianTable()
{
  // Utf8

  // Lowercase
  ins( "a", "а" );
  ins( "b", "б" );
  ins( "v", "в" );
  ins( "w", "в" );
  ins( "g", "г" );
  ins( "d", "д" );
  ins( "e", "е" );
  ins( "yo", "ё" );
  ins( "zh", "ж" );
  ins( "z", "з" );
  ins( "i", "и" );
  ins( "j", "й" );
  ins( "k", "к" );
  ins( "l", "л" );
  ins( "m", "м" );
  ins( "n", "н" );
  ins( "o", "о" );
  ins( "p", "п" );
  ins( "r", "р" );
  ins( "s", "с" );
  ins( "t", "т" );
  ins( "u", "у" );
  ins( "f", "ф" );
  ins( "h", "х" );
  ins( "ts", "ц" );
  ins( "c", "ц" );
  ins( "ch", "ч" );
  ins( "sh", "ш" );
  ins( "shch", "щ" );
  ins( "\"", "ъ" );
  ins( "y", "ы" );
  ins( "'", "ь" );
  ins( "'e", "э" );
  ins( "yu", "ю" );
  ins( "ya", "я" );

  // Uppercase
  ins( "A", "А" );
  ins( "B", "Б" );
  ins( "V", "В" );
  ins( "W", "В" );
  ins( "G", "Г" );
  ins( "D", "Д" );
  ins( "E", "Е" );
  ins( "YO", "Ё" );
  ins( "Yo", "Ё" );
  ins( "ZH", "Ж" );
  ins( "Zh", "Ж" );
  ins( "Z", "З" );
  ins( "I", "И" );
  ins( "J", "Й" );
  ins( "K", "К" );
  ins( "L", "Л" );
  ins( "M", "М" );
  ins( "N", "Н" );
  ins( "O", "О" );
  ins( "P", "П" );
  ins( "R", "Р" );
  ins( "S", "С" );
  ins( "T", "Т" );
  ins( "U", "У" );
  ins( "F", "Ф" );
  ins( "H", "Х" );
  ins( "TS", "Ц" );
  ins( "Ts", "Ц" );
  ins( "C", "Ц" );
  ins( "CH", "Ч" );
  ins( "Ch", "Ч" );
  ins( "SH", "Ш" );
  ins( "Sh", "Ш" );
  ins( "SHCH", "Щ" );
  ins( "ShCh", "Щ" );
  ins( "Y", "Ы" );
  ins( "'E", "Э" );
  ins( "YU", "Ю" );
  ins( "Yu", "Ю" );
  ins( "YA", "Я" );
  ins( "Ya", "Я" );
}
  
sptr< Dictionary::Class > makeDictionary() throw( std::exception )
{
  static RussianTable t;

  return new Transliteration::TransliterationDictionary( "cf1b74acd98adea9b2bba16af38f1086",
                      QCoreApplication::translate( "RussianTranslit", "Russian Transliteration" ).toUtf8().data(),
                      QIcon( ":/flags/ru.png" ), t );
}

}

