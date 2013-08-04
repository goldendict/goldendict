/* This file is (c) 2008-2013 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "langcoder.hh"
#include "folding.hh"
#include "wstring_qt.hh"
#include "language.hh"

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <cctype>
#include <QLocale>

LangCoder langCoder;

// Language codes

static GDLangCode LangCodes[] = {

    { "aa", "aar", -1, "Afar" },
    { "ab", "abk", -1, "Abkhazian" },
    { "ae", "ave", -1, "Avestan" },
    { "af", "afr", -1, "Afrikaans" },
    { "ak", "aka", -1, "Akan" },
    { "am", "amh", -1, "Amharic" },
    { "an", "arg", -1, "Aragonese" },
    { "ar", "ara", 1, "Arabic" },
    { "as", "asm", -1, "Assamese" },
    { "av", "ava", -1, "Avaric" },
    { "ay", "aym", -1, "Aymara" },
    { "az", "aze", 0, "Azerbaijani" },
    { "ba", "bak", 0, "Bashkir" },
    { "be", "bel", 0, "Belarusian" },
    { "bg", "bul", 0, "Bulgarian" },
    { "bh", "bih", -1, "Bihari" },
    { "bi", "bis", -1, "Bislama" },
    { "bm", "bam", -1, "Bambara" },
    { "bn", "ben", -1, "Bengali" },
    { "bo", "tib", -1, "Tibetan" },
    { "br", "bre", -1, "Breton" },
    { "bs", "bos", 0, "Bosnian" },
    { "ca", "cat", -1, "Catalan" },
    { "ce", "che", -1, "Chechen" },
    { "ch", "cha", -1, "Chamorro" },
    { "co", "cos", -1, "Corsican" },
    { "cr", "cre", -1, "Cree" },
    { "cs", "cze", 0, "Czech" },
    { "cu", "chu", 0, "Church Slavic" },
    { "cv", "chv", 0, "Chuvash" },
    { "cy", "wel", 0, "Welsh" },
    { "da", "dan", 0, "Danish" },
    { "de", "ger", 0, "German" },
    { "dv", "div", -1, "Divehi" },
    { "dz", "dzo", -1, "Dzongkha" },
    { "ee", "ewe", -1, "Ewe" },
    { "el", "gre", 0, "Greek" },
    { "en", "eng", 0, "English" },
    { "eo", "epo", 0, "Esperanto" },
    { "es", "spa", 0, "Spanish" },
    { "et", "est", 0, "Estonian" },
    { "eu", "baq", 0, "Basque" },
    { "fa", "per", -1, "Persian" },
    { "ff", "ful", -1, "Fulah" },
    { "fi", "fin", 0, "Finnish" },
    { "fj", "fij", -1, "Fijian" },
    { "fo", "fao", -1, "Faroese" },
    { "fr", "fre", 0, "French" },
    { "fy", "fry", -1, "Western Frisian" },
    { "ga", "gle", 0, "Irish" },
    { "gd", "gla", 0, "Scottish Gaelic" },
    { "gl", "glg", -1, "Galician" },
    { "gn", "grn", -1, "Guarani" },
    { "gu", "guj", -1, "Gujarati" },
    { "gv", "glv", -1, "Manx" },
    { "ha", "hau", -1, "Hausa" },
    { "he", "heb", 1, "Hebrew" },
    { "hi", "hin", -1, "Hindi" },
    { "ho", "hmo", -1, "Hiri Motu" },
    { "hr", "hrv", 0, "Croatian" },
    { "ht", "hat", -1, "Haitian" },
    { "hu", "hun", 0, "Hungarian" },
    { "hy", "arm", 0, "Armenian" },
    { "hz", "her", -1, "Herero" },
    { "ia", "ina", -1, "Interlingua" },
    { "id", "ind", -1, "Indonesian" },
    { "ie", "ile", -1, "Interlingue" },
    { "ig", "ibo", -1, "Igbo" },
    { "ii", "iii", -1, "Sichuan Yi" },
    { "ik", "ipk", -1, "Inupiaq" },
    { "io", "ido", -1, "Ido" },
    { "is", "ice", -1, "Icelandic" },
    { "it", "ita", 0, "Italian" },
    { "iu", "iku", -1, "Inuktitut" },
    { "ja", "jpn", 0, "Japanese" },
    { "jv", "jav", -1, "Javanese" },
    { "ka", "geo", 0, "Georgian" },
    { "kg", "kon", -1, "Kongo" },
    { "ki", "kik", -1, "Kikuyu" },
    { "kj", "kua", -1, "Kwanyama" },
    { "kk", "kaz", 0, "Kazakh" },
    { "kl", "kal", -1, "Kalaallisut" },
    { "km", "khm", -1, "Khmer" },
    { "kn", "kan", -1, "Kannada" },
    { "ko", "kor", 0, "Korean" },
    { "kr", "kau", -1, "Kanuri" },
    { "ks", "kas", -1, "Kashmiri" },
    { "ku", "kur", -1, "Kurdish" },
    { "kv", "kom", 0, "Komi" },
    { "kw", "cor", -1, "Cornish" },
    { "ky", "kir", -1, "Kirghiz" },
    { "la", "lat", 0, "Latin" },
    { "lb", "ltz", 0, "Luxembourgish" },
    { "lg", "lug", -1, "Ganda" },
    { "li", "lim", -1, "Limburgish" },
    { "ln", "lin", -1, "Lingala" },
    { "lo", "lao", -1, "Lao" },
    { "lt", "lit", 0, "Lithuanian" },
    { "lu", "lub", -1, "Luba-Katanga" },
    { "lv", "lav", 0, "Latvian" },
    { "mg", "mlg", -1, "Malagasy" },
    { "mh", "mah", -1, "Marshallese" },
    { "mi", "mao", -1, "Maori" },
    { "mk", "mac", 0, "Macedonian" },
    { "ml", "mal", -1, "Malayalam" },
    { "mn", "mon", -1, "Mongolian" },
    { "mr", "mar", -1, "Marathi" },
    { "ms", "may", -1, "Malay" },
    { "mt", "mlt", -1, "Maltese" },
    { "my", "bur", -1, "Burmese" },
    { "na", "nau", -1, "Nauru" },
    { "nb", "nob", 0, "Norwegian Bokmal" },
    { "nd", "nde", -1, "North Ndebele" },
    { "ne", "nep", -1, "Nepali" },
    { "ng", "ndo", -1, "Ndonga" },
    { "nl", "dut", -1, "Dutch" },
    { "nn", "nno", -1, "Norwegian Nynorsk" },
    { "no", "nor", 0, "Norwegian" },
    { "nr", "nbl", -1, "South Ndebele" },
    { "nv", "nav", -1, "Navajo" },
    { "ny", "nya", -1, "Chichewa" },
    { "oc", "oci", -1, "Occitan" },
    { "oj", "oji", -1, "Ojibwa" },
    { "om", "orm", -1, "Oromo" },
    { "or", "ori", -1, "Oriya" },
    { "os", "oss", -1, "Ossetian" },
    { "pa", "pan", -1, "Panjabi" },
    { "pi", "pli", -1, "Pali" },
    { "pl", "pol", 0, "Polish" },
    { "ps", "pus", -1, "Pashto" },
    { "pt", "por", 0, "Portuguese" },
    { "qu", "que", -1, "Quechua" },
    { "rm", "roh", -1, "Raeto-Romance" },
    { "rn", "run", -1, "Kirundi" },
    { "ro", "rum", 0, "Romanian" },
    { "ru", "rus", 0, "Russian" },
    { "rw", "kin", -1, "Kinyarwanda" },
    { "sa", "san", -1, "Sanskrit" },
    { "sc", "srd", -1, "Sardinian" },
    { "sd", "snd", -1, "Sindhi" },
    { "se", "sme", -1, "Northern Sami" },
    { "sg", "sag", -1, "Sango" },
    { "sh", "shr", 0, "Serbo-Croatian" },
    { "si", "sin", -1, "Sinhala" },
    { "sk", "slo", 0, "Slovak" },
    { "sl", "slv", 0, "Slovenian" },
    { "sm", "smo", -1, "Samoan" },
    { "sn", "sna", -1, "Shona" },
    { "so", "som", -1, "Somali" },
    { "sq", "alb", 0, "Albanian" },
    { "sr", "srp", 0, "Serbian" },
    { "ss", "ssw", -1, "Swati" },
    { "st", "sot", -1, "Southern Sotho" },
    { "su", "sun", -1, "Sundanese" },
    { "sv", "swe", 0, "Swedish" },
    { "sw", "swa", -1, "Swahili" },
    { "ta", "tam", -1, "Tamil" },
    { "te", "tel", -1, "Telugu" },
    { "tg", "tgk", 0, "Tajik" },
    { "th", "tha", -1, "Thai" },
    { "ti", "tir", -1, "Tigrinya" },
    { "tk", "tuk", 0, "Turkmen" },
    { "tl", "tgl", -1, "Tagalog" },
    { "tn", "tsn", -1, "Tswana" },
    { "to", "ton", -1, "Tonga" },
    { "tr", "tur", 0, "Turkish" },
    { "ts", "tso", -1, "Tsonga" },
    { "tt", "tat", -1, "Tatar" },
    { "tw", "twi", -1, "Twi" },
    { "ty", "tah", -1, "Tahitian" },
    { "ug", "uig", -1, "Uighur" },
    { "uk", "ukr", -1, "Ukrainian" },
    { "ur", "urd", -1, "Urdu" },
    { "uz", "uzb", 0, "Uzbek" },
    { "ve", "ven", -1, "Venda" },
    { "vi", "vie", -1, "Vietnamese" },
    { "vo", "vol", 0, "Volapuk" },
    { "wa", "wln", -1, "Walloon" },
    { "wo", "wol", -1, "Wolof" },
    { "xh", "xho", -1, "Xhosa" },
    { "yi", "yid", -1, "Yiddish" },
    { "yo", "yor", -1, "Yoruba" },
    { "za", "zha", -1, "Zhuang" },
    { "zh", "chi", 0, "Chinese" },
    { "zu", "zul", -1, "Zulu" },
    { "jb", "jbo", 0, "Lojban" },

    { "", "", 0, "" }
};

LangCoder::LangCoder()
{
  for (int i = 0; true; i++) {
    const GDLangCode &lc = LangCodes[i];
    if (lc.lang[0] == 0)
      break;
    codeMap[code2toInt(lc.code)] = i;
  }
}

QString LangCoder::decode(quint32 code)
{
  if (langCoder.codeMap.contains(code))
    return LangCodes[langCoder.codeMap[code]].lang;

  return QString();
}

QIcon LangCoder::icon(quint32 code)
{
  if (langCoder.codeMap.contains(code))
  {
    const GDLangCode &lc = LangCodes[ langCoder.codeMap[ code ] ];
    return QIcon( ":/flags/" + QString(lc.code) + ".png" );
  }

  return QIcon();
}

LangStruct LangCoder::langStruct(quint32 code)
{
  LangStruct ls;
  ls.code = code;
  ls.order = -1;
  if (codeMap.contains(code)) {
    int order = codeMap[code];
    const GDLangCode &lc = LangCodes[order];
    ls.order = order;
    ls.lang = lc.lang;
    ls.icon = QIcon(":/flags/" + QString(lc.code) + ".png");
  }
  return ls;
}

QString LangCoder::intToCode2( quint32 val )
{
  if ( !val || val == 0xFFffFFff )
    return QString();

  char code[ 2 ];

  code[ 0 ] = val & 0xFF;
  code[ 1 ] = ( val >> 8 ) & 0xFF;

  return QString::fromLatin1( code, 2 );
}

quint32 LangCoder::code3toInt(const std::string& code3)
{
  if (code3.length() < 2)
    return 0;

  // this is temporary
  char code1 = tolower( code3.at(1) );
  char code0 = tolower( code3.at(0) );

  return ( ((quint32)code1) << 8 ) + (quint32)code0;
}

quint32 LangCoder::findIdForLanguage( gd::wstring const & lang )
{
  gd::wstring langFolded = Folding::apply( lang );

  for( GDLangCode const * lc = LangCodes; lc->code[ 0 ]; ++lc )
  {
    if ( langFolded == Folding::apply( gd::toWString( lc->lang ) ) )
    {
      // We've got a match
      return code2toInt( lc->code );
    }
  }

  return Language::findBlgLangIDByEnglishName( lang );
  //return 0;
}

quint32 LangCoder::findIdForLanguageCode3( const char * code3 )
{
  for( GDLangCode const * lc = LangCodes; lc->code[ 0 ]; ++lc )
  {
    if ( strcasecmp( code3, lc->code3 ) == 0 )
    {
      // We've got a match
      return code2toInt( lc->code );
    }
  }

  return 0;
}

quint32 LangCoder::guessId( const QString & lang )
{
  QString lstr = lang.simplified().toLower();

  // too small to guess
  if (lstr.size() < 2)
    return 0;

  // check if it could be the whole language name
  if (lstr.size() >= 3)
  {
    for( GDLangCode const * lc = LangCodes; lc->code[ 0 ]; ++lc )
    {
      if ( lstr == QString( lc->lang ) )
      {
        // We've got a match
        return code2toInt( lc->code );
      }
    }
  }

  // still not found - try to match by 2-symbol code
  return code2toInt( lstr.left(2).toLatin1().data() );
}

QPair<quint32,quint32> LangCoder::findIdsForName( QString const & name )
{
  QString nameFolded = "|" + name.toCaseFolded() + "|";
  QRegExp reg( "[^a-z]([a-z]{2,3})-([a-z]{2,3})[^a-z]" ); reg.setMinimal(true);
  int off = 0;

  while ( reg.indexIn( nameFolded, off ) >= 0 )
  {
    quint32 from = guessId( reg.cap(1) );
    quint32 to = guessId( reg.cap(2) );
    if (from && to)
      return QPair<quint32,quint32>(from, to);

    off += reg.matchedLength();
  }

  return QPair<quint32,quint32>(0, 0);
}

QPair<quint32,quint32> LangCoder::findIdsForFilename( QString const & name )
{
  return findIdsForName( QFileInfo( name ).fileName() );
}

bool LangCoder::isLanguageRTL( quint32 code )
{
  if ( langCoder.codeMap.contains( code ) )
  {
    GDLangCode &lc = LangCodes[ langCoder.codeMap[ code ] ];
    if( lc.isRTL < 0 )
    {
#if QT_VERSION >= 0x040700
      lc.isRTL = ( int )( QLocale( lc.code ).textDirection() == Qt::RightToLeft );
#else
      lc.isRTL = 0;
#endif
    }
    return lc.isRTL != 0;
  }

  return false;
}

/*
LangStruct& LangCoder::CodeToLangStruct(const QString &code)
{
  if (codeMap.contains(code)) {
    LangStruct &ls = codeMap[code];
    if (ls.icon.isNull() && *ls.icon_code) {
      ls.icon = QIcon(":/Resources/flags/" + QString(ls.icon_code) + ".png");
    }
        return ls;
  }

    return dummyLS;
}

QString LangCoder::CodeToHtml(const QString &code)
{
  if (codeMap.contains(code)) {
    LangStruct &ls = codeMap[code];
    if (*ls.icon_code) {
      return "<img src=':/Resources/flags/" + QString(ls.icon_code) + ".png'>&nbsp;" + ls.lang;
    }
        return ls.lang;
  }

    return "";
}

bool LangCoder::CheckCode(QString &code)
{
  code = code.toUpper();

  if (codeMap.contains(code))
    return true;

  if (code == "DEU") {
    code = "GER";
    return true;
  }

  return false;
}
*/

/*
LangModel::LangModel(QObject * parent) : QAbstractItemModel(parent)
{
}

int LangModel::columnCount ( const QModelIndex & parent ) const
{
  return 2;
}

int LangModel::rowCount ( const QModelIndex & parent ) const
{
  return arraySize(LangCodes);
}

QVariant LangModel::data ( const QModelIndex & index, int role ) const
{
  switch (role) {
    case Qt::DisplayRole:
      return LangCodes[index.row()].lang;

    case LangCodeRole:
      return LangCodes[index.row()].code;

    default:;
  }

  return QVariant();
}

QModelIndex LangModel::index ( int row, int column, const QModelIndex & parent ) const
{
  return createIndex(row, column);
}

QModelIndex LangModel::parent ( const QModelIndex & index ) const
{
  return QModelIndex();
}
*/
