#include "language.hh"
#include "langcoder.hh"
#include <map>
#include <QCoreApplication>

namespace Language {

namespace {

using std::map;

struct Db
{
  static Db const & instance();

  map< QString, QString > const & getIso2ToEnglish() const
  { return iso2ToEnglish; }
  map< QString, QString > const & getIso2ToLocalized() const
  { return iso2ToLocalized; }

private:

  map< QString, QString > iso2ToEnglish, iso2ToLocalized;

  Db();

  void addEntry( QString const & iso2, QString const & english,
                 QString const & localized );
};

Db const & Db::instance()
{
  static Db v;

  return v;
}

void Db::addEntry( QString const & iso2, QString const & english,
                   QString const & localized )
{
  iso2ToEnglish[ iso2 ] = english;
  iso2ToLocalized[ iso2 ] = localized;
}

Db::Db()
{
  addEntry( "aa", "Afar", QCoreApplication::translate( "Language", "Afar" ) );
  addEntry( "ab", "Abkhazian", QCoreApplication::translate( "Language", "Abkhazian" ) );
  addEntry( "ae", "Avestan", QCoreApplication::translate( "Language", "Avestan" ) );
  addEntry( "af", "Afrikaans", QCoreApplication::translate( "Language", "Afrikaans" ) );
  addEntry( "ak", "Akan", QCoreApplication::translate( "Language", "Akan" ) );
  addEntry( "am", "Amharic", QCoreApplication::translate( "Language", "Amharic" ) );
  addEntry( "an", "Aragonese", QCoreApplication::translate( "Language", "Aragonese" ) );
  addEntry( "ar", "Arabic", QCoreApplication::translate( "Language", "Arabic" ) );
  addEntry( "as", "Assamese", QCoreApplication::translate( "Language", "Assamese" ) );
  addEntry( "av", "Avaric", QCoreApplication::translate( "Language", "Avaric" ) );
  addEntry( "ay", "Aymara", QCoreApplication::translate( "Language", "Aymara" ) );
  addEntry( "az", "Azerbaijani", QCoreApplication::translate( "Language", "Azerbaijani" ) );
  addEntry( "ba", "Bashkir", QCoreApplication::translate( "Language", "Bashkir" ) );
  addEntry( "be", "Belarusian", QCoreApplication::translate( "Language", "Belarusian" ) );
  addEntry( "bg", "Bulgarian", QCoreApplication::translate( "Language", "Bulgarian" ) );
  addEntry( "bh", "Bihari", QCoreApplication::translate( "Language", "Bihari" ) );
  addEntry( "bi", "Bislama", QCoreApplication::translate( "Language", "Bislama" ) );
  addEntry( "bm", "Bambara", QCoreApplication::translate( "Language", "Bambara" ) );
  addEntry( "bn", "Bengali", QCoreApplication::translate( "Language", "Bengali" ) );
  addEntry( "bo", "Tibetan", QCoreApplication::translate( "Language", "Tibetan" ) );
  addEntry( "br", "Breton", QCoreApplication::translate( "Language", "Breton" ) );
  addEntry( "bs", "Bosnian", QCoreApplication::translate( "Language", "Bosnian" ) );
  addEntry( "ca", "Catalan", QCoreApplication::translate( "Language", "Catalan" ) );
  addEntry( "ce", "Chechen", QCoreApplication::translate( "Language", "Chechen" ) );
  addEntry( "ch", "Chamorro", QCoreApplication::translate( "Language", "Chamorro" ) );
  addEntry( "co", "Corsican", QCoreApplication::translate( "Language", "Corsican" ) );
  addEntry( "cr", "Cree", QCoreApplication::translate( "Language", "Cree" ) );
  addEntry( "cs", "Czech", QCoreApplication::translate( "Language", "Czech" ) );
  addEntry( "cu", "Church Slavic", QCoreApplication::translate( "Language", "Church Slavic" ) );
  addEntry( "cv", "Chuvash", QCoreApplication::translate( "Language", "Chuvash" ) );
  addEntry( "cy", "Welsh", QCoreApplication::translate( "Language", "Welsh" ) );
  addEntry( "da", "Danish", QCoreApplication::translate( "Language", "Danish" ) );
  addEntry( "de", "German", QCoreApplication::translate( "Language", "German" ) );
  addEntry( "dv", "Divehi", QCoreApplication::translate( "Language", "Divehi" ) );
  addEntry( "dz", "Dzongkha", QCoreApplication::translate( "Language", "Dzongkha" ) );
  addEntry( "ee", "Ewe", QCoreApplication::translate( "Language", "Ewe" ) );
  addEntry( "el", "Greek", QCoreApplication::translate( "Language", "Greek" ) );
  addEntry( "en", "English", QCoreApplication::translate( "Language", "English" ) );
  addEntry( "eo", "Esperanto", QCoreApplication::translate( "Language", "Esperanto" ) );
  addEntry( "es", "Spanish", QCoreApplication::translate( "Language", "Spanish" ) );
  addEntry( "et", "Estonian", QCoreApplication::translate( "Language", "Estonian" ) );
  addEntry( "eu", "Basque", QCoreApplication::translate( "Language", "Basque" ) );
  addEntry( "fa", "Persian", QCoreApplication::translate( "Language", "Persian" ) );
  addEntry( "ff", "Fulah", QCoreApplication::translate( "Language", "Fulah" ) );
  addEntry( "fi", "Finnish", QCoreApplication::translate( "Language", "Finnish" ) );
  addEntry( "fj", "Fijian", QCoreApplication::translate( "Language", "Fijian" ) );
  addEntry( "fo", "Faroese", QCoreApplication::translate( "Language", "Faroese" ) );
  addEntry( "fr", "French", QCoreApplication::translate( "Language", "French" ) );
  addEntry( "fy", "Western Frisian", QCoreApplication::translate( "Language", "Western Frisian" ) );
  addEntry( "ga", "Irish", QCoreApplication::translate( "Language", "Irish" ) );
  addEntry( "gd", "Scottish Gaelic", QCoreApplication::translate( "Language", "Scottish Gaelic" ) );
  addEntry( "gl", "Galician", QCoreApplication::translate( "Language", "Galician" ) );
  addEntry( "gn", "Guarani", QCoreApplication::translate( "Language", "Guarani" ) );
  addEntry( "gu", "Gujarati", QCoreApplication::translate( "Language", "Gujarati" ) );
  addEntry( "gv", "Manx", QCoreApplication::translate( "Language", "Manx" ) );
  addEntry( "ha", "Hausa", QCoreApplication::translate( "Language", "Hausa" ) );
  addEntry( "he", "Hebrew", QCoreApplication::translate( "Language", "Hebrew" ) );
  addEntry( "hi", "Hindi", QCoreApplication::translate( "Language", "Hindi" ) );
  addEntry( "ho", "Hiri Motu", QCoreApplication::translate( "Language", "Hiri Motu" ) );
  addEntry( "hr", "Croatian", QCoreApplication::translate( "Language", "Croatian" ) );
  addEntry( "ht", "Haitian", QCoreApplication::translate( "Language", "Haitian" ) );
  addEntry( "hu", "Hungarian", QCoreApplication::translate( "Language", "Hungarian" ) );
  addEntry( "hy", "Armenian", QCoreApplication::translate( "Language", "Armenian" ) );
  addEntry( "hz", "Herero", QCoreApplication::translate( "Language", "Herero" ) );
  addEntry( "ia", "Interlingua", QCoreApplication::translate( "Language", "Interlingua" ) );
  addEntry( "id", "Indonesian", QCoreApplication::translate( "Language", "Indonesian" ) );
  addEntry( "ie", "Interlingue", QCoreApplication::translate( "Language", "Interlingue" ) );
  addEntry( "ig", "Igbo", QCoreApplication::translate( "Language", "Igbo" ) );
  addEntry( "ii", "Sichuan Yi", QCoreApplication::translate( "Language", "Sichuan Yi" ) );
  addEntry( "ik", "Inupiaq", QCoreApplication::translate( "Language", "Inupiaq" ) );
  addEntry( "io", "Ido", QCoreApplication::translate( "Language", "Ido" ) );
  addEntry( "is", "Icelandic", QCoreApplication::translate( "Language", "Icelandic" ) );
  addEntry( "it", "Italian", QCoreApplication::translate( "Language", "Italian" ) );
  addEntry( "iu", "Inuktitut", QCoreApplication::translate( "Language", "Inuktitut" ) );
  addEntry( "ja", "Japanese", QCoreApplication::translate( "Language", "Japanese" ) );
  addEntry( "jv", "Javanese", QCoreApplication::translate( "Language", "Javanese" ) );
  addEntry( "ka", "Georgian", QCoreApplication::translate( "Language", "Georgian" ) );
  addEntry( "kg", "Kongo", QCoreApplication::translate( "Language", "Kongo" ) );
  addEntry( "ki", "Kikuyu", QCoreApplication::translate( "Language", "Kikuyu" ) );
  addEntry( "kj", "Kwanyama", QCoreApplication::translate( "Language", "Kwanyama" ) );
  addEntry( "kk", "Kazakh", QCoreApplication::translate( "Language", "Kazakh" ) );
  addEntry( "kl", "Kalaallisut", QCoreApplication::translate( "Language", "Kalaallisut" ) );
  addEntry( "km", "Khmer", QCoreApplication::translate( "Language", "Khmer" ) );
  addEntry( "kn", "Kannada", QCoreApplication::translate( "Language", "Kannada" ) );
  addEntry( "ko", "Korean", QCoreApplication::translate( "Language", "Korean" ) );
  addEntry( "kr", "Kanuri", QCoreApplication::translate( "Language", "Kanuri" ) );
  addEntry( "ks", "Kashmiri", QCoreApplication::translate( "Language", "Kashmiri" ) );
  addEntry( "ku", "Kurdish", QCoreApplication::translate( "Language", "Kurdish" ) );
  addEntry( "kv", "Komi", QCoreApplication::translate( "Language", "Komi" ) );
  addEntry( "kw", "Cornish", QCoreApplication::translate( "Language", "Cornish" ) );
  addEntry( "ky", "Kirghiz", QCoreApplication::translate( "Language", "Kirghiz" ) );
  addEntry( "la", "Latin", QCoreApplication::translate( "Language", "Latin" ) );
  addEntry( "lb", "Luxembourgish", QCoreApplication::translate( "Language", "Luxembourgish" ) );
  addEntry( "lg", "Ganda", QCoreApplication::translate( "Language", "Ganda" ) );
  addEntry( "li", "Limburgish", QCoreApplication::translate( "Language", "Limburgish" ) );
  addEntry( "ln", "Lingala", QCoreApplication::translate( "Language", "Lingala" ) );
  addEntry( "lo", "Lao", QCoreApplication::translate( "Language", "Lao" ) );
  addEntry( "lt", "Lithuanian", QCoreApplication::translate( "Language", "Lithuanian" ) );
  addEntry( "lu", "Luba-Katanga", QCoreApplication::translate( "Language", "Luba-Katanga" ) );
  addEntry( "lv", "Latvian", QCoreApplication::translate( "Language", "Latvian" ) );
  addEntry( "mg", "Malagasy", QCoreApplication::translate( "Language", "Malagasy" ) );
  addEntry( "mh", "Marshallese", QCoreApplication::translate( "Language", "Marshallese" ) );
  addEntry( "mi", "Maori", QCoreApplication::translate( "Language", "Maori" ) );
  addEntry( "mk", "Macedonian", QCoreApplication::translate( "Language", "Macedonian" ) );
  addEntry( "ml", "Malayalam", QCoreApplication::translate( "Language", "Malayalam" ) );
  addEntry( "mn", "Mongolian", QCoreApplication::translate( "Language", "Mongolian" ) );
  addEntry( "mr", "Marathi", QCoreApplication::translate( "Language", "Marathi" ) );
  addEntry( "ms", "Malay", QCoreApplication::translate( "Language", "Malay" ) );
  addEntry( "mt", "Maltese", QCoreApplication::translate( "Language", "Maltese" ) );
  addEntry( "my", "Burmese", QCoreApplication::translate( "Language", "Burmese" ) );
  addEntry( "na", "Nauru", QCoreApplication::translate( "Language", "Nauru" ) );
  addEntry( "nb", "Norwegian Bokmal", QCoreApplication::translate( "Language", "Norwegian Bokmal" ) );
  addEntry( "nd", "North Ndebele", QCoreApplication::translate( "Language", "North Ndebele" ) );
  addEntry( "ne", "Nepali", QCoreApplication::translate( "Language", "Nepali" ) );
  addEntry( "ng", "Ndonga", QCoreApplication::translate( "Language", "Ndonga" ) );
  addEntry( "nl", "Dutch", QCoreApplication::translate( "Language", "Dutch" ) );
  addEntry( "nn", "Norwegian Nynorsk", QCoreApplication::translate( "Language", "Norwegian Nynorsk" ) );
  addEntry( "no", "Norwegian", QCoreApplication::translate( "Language", "Norwegian" ) );
  addEntry( "nr", "South Ndebele", QCoreApplication::translate( "Language", "South Ndebele" ) );
  addEntry( "nv", "Navajo", QCoreApplication::translate( "Language", "Navajo" ) );
  addEntry( "ny", "Chichewa", QCoreApplication::translate( "Language", "Chichewa" ) );
  addEntry( "oc", "Occitan", QCoreApplication::translate( "Language", "Occitan" ) );
  addEntry( "oj", "Ojibwa", QCoreApplication::translate( "Language", "Ojibwa" ) );
  addEntry( "om", "Oromo", QCoreApplication::translate( "Language", "Oromo" ) );
  addEntry( "or", "Oriya", QCoreApplication::translate( "Language", "Oriya" ) );
  addEntry( "os", "Ossetian", QCoreApplication::translate( "Language", "Ossetian" ) );
  addEntry( "pa", "Panjabi", QCoreApplication::translate( "Language", "Panjabi" ) );
  addEntry( "pi", "Pali", QCoreApplication::translate( "Language", "Pali" ) );
  addEntry( "pl", "Polish", QCoreApplication::translate( "Language", "Polish" ) );
  addEntry( "ps", "Pashto", QCoreApplication::translate( "Language", "Pashto" ) );
  addEntry( "pt", "Portuguese", QCoreApplication::translate( "Language", "Portuguese" ) );
  addEntry( "qu", "Quechua", QCoreApplication::translate( "Language", "Quechua" ) );
  addEntry( "rm", "Raeto-Romance", QCoreApplication::translate( "Language", "Raeto-Romance" ) );
  addEntry( "rn", "Kirundi", QCoreApplication::translate( "Language", "Kirundi" ) );
  addEntry( "ro", "Romanian", QCoreApplication::translate( "Language", "Romanian" ) );
  addEntry( "ru", "Russian", QCoreApplication::translate( "Language", "Russian" ) );
  addEntry( "rw", "Kinyarwanda", QCoreApplication::translate( "Language", "Kinyarwanda" ) );
  addEntry( "sa", "Sanskrit", QCoreApplication::translate( "Language", "Sanskrit" ) );
  addEntry( "sc", "Sardinian", QCoreApplication::translate( "Language", "Sardinian" ) );
  addEntry( "sd", "Sindhi", QCoreApplication::translate( "Language", "Sindhi" ) );
  addEntry( "se", "Northern Sami", QCoreApplication::translate( "Language", "Northern Sami" ) );
  addEntry( "sg", "Sango", QCoreApplication::translate( "Language", "Sango" ) );
  addEntry( "sh", "Serbo-Croatian", QCoreApplication::translate( "Language", "Serbo-Croatian" ) );
  addEntry( "si", "Sinhala", QCoreApplication::translate( "Language", "Sinhala" ) );
  addEntry( "sk", "Slovak", QCoreApplication::translate( "Language", "Slovak" ) );
  addEntry( "sl", "Slovenian", QCoreApplication::translate( "Language", "Slovenian" ) );
  addEntry( "sm", "Samoan", QCoreApplication::translate( "Language", "Samoan" ) );
  addEntry( "sn", "Shona", QCoreApplication::translate( "Language", "Shona" ) );
  addEntry( "so", "Somali", QCoreApplication::translate( "Language", "Somali" ) );
  addEntry( "sq", "Albanian", QCoreApplication::translate( "Language", "Albanian" ) );
  addEntry( "sr", "Serbian", QCoreApplication::translate( "Language", "Serbian" ) );
  addEntry( "ss", "Swati", QCoreApplication::translate( "Language", "Swati" ) );
  addEntry( "st", "Southern Sotho", QCoreApplication::translate( "Language", "Southern Sotho" ) );
  addEntry( "su", "Sundanese", QCoreApplication::translate( "Language", "Sundanese" ) );
  addEntry( "sv", "Swedish", QCoreApplication::translate( "Language", "Swedish" ) );
  addEntry( "sw", "Swahili", QCoreApplication::translate( "Language", "Swahili" ) );
  addEntry( "ta", "Tamil", QCoreApplication::translate( "Language", "Tamil" ) );
  addEntry( "te", "Telugu", QCoreApplication::translate( "Language", "Telugu" ) );
  addEntry( "tg", "Tajik", QCoreApplication::translate( "Language", "Tajik" ) );
  addEntry( "th", "Thai", QCoreApplication::translate( "Language", "Thai" ) );
  addEntry( "ti", "Tigrinya", QCoreApplication::translate( "Language", "Tigrinya" ) );
  addEntry( "tk", "Turkmen", QCoreApplication::translate( "Language", "Turkmen" ) );
  addEntry( "tl", "Tagalog", QCoreApplication::translate( "Language", "Tagalog" ) );
  addEntry( "tn", "Tswana", QCoreApplication::translate( "Language", "Tswana" ) );
  addEntry( "to", "Tonga", QCoreApplication::translate( "Language", "Tonga" ) );
  addEntry( "tr", "Turkish", QCoreApplication::translate( "Language", "Turkish" ) );
  addEntry( "ts", "Tsonga", QCoreApplication::translate( "Language", "Tsonga" ) );
  addEntry( "tt", "Tatar", QCoreApplication::translate( "Language", "Tatar" ) );
  addEntry( "tw", "Twi", QCoreApplication::translate( "Language", "Twi" ) );
  addEntry( "ty", "Tahitian", QCoreApplication::translate( "Language", "Tahitian" ) );
  addEntry( "ug", "Uighur", QCoreApplication::translate( "Language", "Uighur" ) );
  addEntry( "uk", "Ukrainian", QCoreApplication::translate( "Language", "Ukrainian" ) );
  addEntry( "ur", "Urdu", QCoreApplication::translate( "Language", "Urdu" ) );
  addEntry( "uz", "Uzbek", QCoreApplication::translate( "Language", "Uzbek" ) );
  addEntry( "ve", "Venda", QCoreApplication::translate( "Language", "Venda" ) );
  addEntry( "vi", "Vietnamese", QCoreApplication::translate( "Language", "Vietnamese" ) );
  addEntry( "vo", "Volapuk", QCoreApplication::translate( "Language", "Volapuk" ) );
  addEntry( "wa", "Walloon", QCoreApplication::translate( "Language", "Walloon" ) );
  addEntry( "wo", "Wolof", QCoreApplication::translate( "Language", "Wolof" ) );
  addEntry( "xh", "Xhosa", QCoreApplication::translate( "Language", "Xhosa" ) );
  addEntry( "yi", "Yiddish", QCoreApplication::translate( "Language", "Yiddish" ) );
  addEntry( "yo", "Yoruba", QCoreApplication::translate( "Language", "Yoruba" ) );
  addEntry( "za", "Zhuang", QCoreApplication::translate( "Language", "Zhuang" ) );
  addEntry( "zh", "Chinese", QCoreApplication::translate( "Language", "Chinese" ) );
  addEntry( "zu", "Zulu", QCoreApplication::translate( "Language", "Zulu" ) );
}

}

QString englishNameForId( Id id )
{
  map< QString, QString >::const_iterator i =
      Db::instance().getIso2ToEnglish().find( LangCoder::intToCode2( id ) );

  if ( i == Db::instance().getIso2ToEnglish().end() )
    return QString();

  return i->second;
}

QString localizedNameForId( Id id )
{
  map< QString, QString >::const_iterator i =
      Db::instance().getIso2ToLocalized().find( LangCoder::intToCode2( id ) );

  if ( i == Db::instance().getIso2ToLocalized().end() )
    return QString();

  return i->second;
}

}
