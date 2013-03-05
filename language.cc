#include "language.hh"
#include "langcoder.hh"
#include <map>
#include <QCoreApplication>
#ifdef _MSC_VER
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif

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

  map< QString, QString > const & getIso2ToCountry() const
  { return iso2ToCountry; }

private:

  map< QString, QString > iso2ToEnglish, iso2ToLocalized, iso2ToCountry;

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
  addEntry( "jb", "Lojban", QCoreApplication::translate( "Language", "Lojban" ) );

  // Countries

  iso2ToCountry[ "aa" ] = "et";
  iso2ToCountry[ "af" ] = "za";
  iso2ToCountry[ "am" ] = "et";
  iso2ToCountry[ "an" ] = "es";
  iso2ToCountry[ "ar" ] = "ae";
  iso2ToCountry[ "as" ] = "in";
  iso2ToCountry[ "az" ] = "az";
  iso2ToCountry[ "be" ] = "by";
  iso2ToCountry[ "bg" ] = "bg";
  iso2ToCountry[ "bn" ] = "bd";
  iso2ToCountry[ "bo" ] = "cn";
  iso2ToCountry[ "br" ] = "fr";
  iso2ToCountry[ "bs" ] = "ba";
  iso2ToCountry[ "ca" ] = "ad";
  iso2ToCountry[ "cs" ] = "cz";
  iso2ToCountry[ "cy" ] = "gb";
  iso2ToCountry[ "da" ] = "dk";
  iso2ToCountry[ "de" ] = "de";
  iso2ToCountry[ "dz" ] = "bt";
  iso2ToCountry[ "el" ] = "gr";
  iso2ToCountry[ "en" ] = "gb";
  iso2ToCountry[ "es" ] = "es";
  iso2ToCountry[ "et" ] = "ee";
  iso2ToCountry[ "eu" ] = "es";
  iso2ToCountry[ "fa" ] = "ir";
  iso2ToCountry[ "fi" ] = "fi";
  iso2ToCountry[ "fo" ] = "fo";
  iso2ToCountry[ "fr" ] = "fr";
  iso2ToCountry[ "fy" ] = "nl";
  iso2ToCountry[ "ga" ] = "ie";
  iso2ToCountry[ "gd" ] = "gb";
  iso2ToCountry[ "gl" ] = "es";
  iso2ToCountry[ "gu" ] = "in";
  iso2ToCountry[ "gv" ] = "gb";
  iso2ToCountry[ "ha" ] = "ng";
  iso2ToCountry[ "he" ] = "il";
  iso2ToCountry[ "hi" ] = "in";
  iso2ToCountry[ "hr" ] = "hr";
  iso2ToCountry[ "ht" ] = "ht";
  iso2ToCountry[ "hu" ] = "hu";
  iso2ToCountry[ "hy" ] = "am";
  iso2ToCountry[ "id" ] = "id";
  iso2ToCountry[ "ig" ] = "ng";
  iso2ToCountry[ "ik" ] = "ca";
  iso2ToCountry[ "is" ] = "is";
  iso2ToCountry[ "it" ] = "it";
  iso2ToCountry[ "iu" ] = "ca";
  iso2ToCountry[ "iw" ] = "il";
  iso2ToCountry[ "ja" ] = "jp";
  iso2ToCountry[ "ka" ] = "ge";
  iso2ToCountry[ "kk" ] = "kz";
  iso2ToCountry[ "kl" ] = "gl";
  iso2ToCountry[ "km" ] = "kh";
  iso2ToCountry[ "kn" ] = "in";
  iso2ToCountry[ "ko" ] = "kr";
  iso2ToCountry[ "ku" ] = "tr";
  iso2ToCountry[ "kw" ] = "gb";
  iso2ToCountry[ "ky" ] = "kg";
  iso2ToCountry[ "lg" ] = "ug";
  iso2ToCountry[ "li" ] = "be";
  iso2ToCountry[ "lo" ] = "la";
  iso2ToCountry[ "lt" ] = "lt";
  iso2ToCountry[ "lv" ] = "lv";
  iso2ToCountry[ "mg" ] = "mg";
  iso2ToCountry[ "mi" ] = "nz";
  iso2ToCountry[ "mk" ] = "mk";
  iso2ToCountry[ "ml" ] = "in";
  iso2ToCountry[ "mn" ] = "mn";
  iso2ToCountry[ "mr" ] = "in";
  iso2ToCountry[ "ms" ] = "my";
  iso2ToCountry[ "mt" ] = "mt";
  iso2ToCountry[ "nb" ] = "no";
  iso2ToCountry[ "ne" ] = "np";
  iso2ToCountry[ "nl" ] = "nl";
  iso2ToCountry[ "nn" ] = "no";
  iso2ToCountry[ "nr" ] = "za";
  iso2ToCountry[ "oc" ] = "fr";
  iso2ToCountry[ "om" ] = "et";
  iso2ToCountry[ "or" ] = "in";
  iso2ToCountry[ "pa" ] = "pk";
  iso2ToCountry[ "pl" ] = "pl";
  iso2ToCountry[ "pt" ] = "pt";
  iso2ToCountry[ "ro" ] = "ro";
  iso2ToCountry[ "ru" ] = "ru";
  iso2ToCountry[ "rw" ] = "rw";
  iso2ToCountry[ "sa" ] = "in";
  iso2ToCountry[ "sc" ] = "it";
  iso2ToCountry[ "sd" ] = "in";
  iso2ToCountry[ "se" ] = "no";
  iso2ToCountry[ "si" ] = "lk";
  iso2ToCountry[ "sk" ] = "sk";
  iso2ToCountry[ "sl" ] = "si";
  iso2ToCountry[ "so" ] = "so";
  iso2ToCountry[ "sq" ] = "al";
  iso2ToCountry[ "sr" ] = "rs";
  iso2ToCountry[ "ss" ] = "za";
  iso2ToCountry[ "st" ] = "za";
  iso2ToCountry[ "sv" ] = "se";
  iso2ToCountry[ "ta" ] = "in";
  iso2ToCountry[ "te" ] = "in";
  iso2ToCountry[ "tg" ] = "tj";
  iso2ToCountry[ "th" ] = "th";
  iso2ToCountry[ "ti" ] = "er";
  iso2ToCountry[ "tk" ] = "tm";
  iso2ToCountry[ "tl" ] = "ph";
  iso2ToCountry[ "tn" ] = "za";
  iso2ToCountry[ "tr" ] = "tr";
  iso2ToCountry[ "ts" ] = "za";
  iso2ToCountry[ "tt" ] = "ru";
  iso2ToCountry[ "ug" ] = "cn";
  iso2ToCountry[ "uk" ] = "ua";
  iso2ToCountry[ "ur" ] = "pk";
  iso2ToCountry[ "uz" ] = "uz";
  iso2ToCountry[ "ve" ] = "za";
  iso2ToCountry[ "vi" ] = "vn";
  iso2ToCountry[ "wa" ] = "be";
  iso2ToCountry[ "wo" ] = "sn";
  iso2ToCountry[ "xh" ] = "za";
  iso2ToCountry[ "yi" ] = "us";
  iso2ToCountry[ "yo" ] = "ng";
  iso2ToCountry[ "zh" ] = "cn";
  iso2ToCountry[ "zu" ] = "za";
}

}

/// babylon languages
#ifndef blgCode2Int
#define blgCode2Int( index, code0, code1 ) (((uint32_t)index) << 16 ) + (((uint32_t)code1) << 8 ) + (uint32_t)code0
#endif
const BabylonLang BabylonDb[] ={
    { blgCode2Int( 1, 'z', 'h' ), "tw", "Traditional Chinese", QT_TR_NOOP( "Traditional Chinese" ) },
    { blgCode2Int( 2, 'z', 'h' ), "cn", "Simplified Chinese", QT_TR_NOOP( "Simplified Chinese" ) },
    { blgCode2Int( 3, 0, 0 ), "other", "Other", QT_TR_NOOP( "Other" ) },
    { blgCode2Int( 4, 'z', 'h' ), "cn", "Other Simplified Chinese dialects", QT_TR_NOOP( "Other Simplified Chinese dialects" ) },
    { blgCode2Int( 5, 'z', 'h' ), "tw", "Other Traditional Chinese dialects", QT_TR_NOOP( "Other Traditional Chinese dialects" ) },
    { blgCode2Int( 6, 0, 0 ), "other", "Other Eastern-European languages", QT_TR_NOOP( "Other Eastern-European languages" ) },
    { blgCode2Int( 7, 0, 0 ), "other", "Other Western-European languages", QT_TR_NOOP( "Other Western-European languages" )},
    { blgCode2Int( 8, 'r', 'u' ), "ru", "Other Russian languages", QT_TR_NOOP( "Other Russian languages" ) },
    { blgCode2Int( 9, 'j', 'a' ), "jp", "Other Japanese languages", QT_TR_NOOP( "Other Japanese languages" ) },
    { blgCode2Int( 10, 0, 0 ), "other", "Other Baltic languages", QT_TR_NOOP( "Other Baltic languages" )},
    { blgCode2Int( 11, 'e', 'l' ), "gr", "Other Greek languages", QT_TR_NOOP( "Other Greek languages" ) },
    { blgCode2Int( 12, 'k', 'o' ), "kr", "Other Korean dialects", QT_TR_NOOP( "Other Korean dialects" ) },
    { blgCode2Int( 13, 't', 'r' ), "tr", "Other Turkish dialects", QT_TR_NOOP( "Other Turkish dialects" ) },
    { blgCode2Int( 14, 't', 'h' ), "th", "Other Thai dialects", QT_TR_NOOP( "Other Thai dialects" ) },
    { blgCode2Int( 15, 0, 0 ), "dz", "Tamazight", QT_TR_NOOP( "Tamazight" ) }
};

BabylonLang getBabylonLangByIndex( int index )
{
    return BabylonDb[ index ];
}

quint32 findBlgLangIDByEnglishName( gd::wstring const & lang )
{
    QString enName = gd::toQString( lang );
    for( int idx=0;idx < 15 ; ++idx )
    {
        if( QString::compare( BabylonDb[ idx ].englishName, enName, Qt::CaseInsensitive  ) == 0 )
            return BabylonDb[ idx ].id;
    }
    return 0;
}

QString englishNameForId( Id id )
{
    if(  id >= 0x010000 && id <= 0x0fffff ) //babylon
    {
        return BabylonDb[ ( (id >> 16 ) & 0x0f) - 1 ].englishName;
    }
  map< QString, QString >::const_iterator i =
      Db::instance().getIso2ToEnglish().find( LangCoder::intToCode2( id ) );

  if ( i == Db::instance().getIso2ToEnglish().end() )
    return QString();

  return i->second;
}

QString localizedNameForId( Id id )
{
    if(  id >= 0x010000 && id <= 0x0fffff ) //babylon
    {
        return QCoreApplication::translate( "Language", BabylonDb[ ( ( id >> 16 ) & 0x0f ) - 1 ].localizedName );
    }
  map< QString, QString >::const_iterator i =
      Db::instance().getIso2ToLocalized().find( LangCoder::intToCode2( id ) );

  if ( i == Db::instance().getIso2ToLocalized().end() )
    return QString();

  return i->second;
}

QString countryCodeForId( Id id )
{
    if(  id >= 0x010000 && id <= 0x0fffff ) //babylon
    {
        return BabylonDb[ ( ( id >> 16 ) & 0x0f ) - 1 ].contryCode;
    }
  map< QString, QString >::const_iterator i =
      Db::instance().getIso2ToCountry().find( LangCoder::intToCode2( id ) );

  if ( i == Db::instance().getIso2ToCountry().end() )
    return QString();

  return i->second;
}

QString localizedStringForId( Id langId )
{
    QString name = localizedNameForId( langId );

    if ( name.isEmpty() )
      return name;

    QString iconId = countryCodeForId( langId );

    if( iconId.isEmpty() )
      return name;
    else
      return QString( "<img src=\":/flags/%1.png\"> %2" ).arg( iconId ).arg( name );
  }
}
