#ifndef LANGCODER_H
#define LANGCODER_H

#include <QtGui>
#include "wstring.hh"

struct LangCode
{
    char code[ 3 ]; // ISO 639-1
    char code3[ 4 ]; // ISO 639-2B ( http://www.loc.gov/standards/iso639-2/ )
    char const * lang; // Language name in English
};

// Language codes

const LangCode LangCodes[] = {

    { "aa", "aar", "Afar" },
    { "ab", "abk", "Abkhazian" },
    { "ae", "ave", "Avestan" },
    { "af", "afr", "Afrikaans" },
    { "ak", "aka", "Akan" },
    { "am", "amh", "Amharic" },
    { "an", "arg", "Aragonese" },
    { "ar", "ara", "Arabic" },
    { "as", "asm", "Assamese" },
    { "av", "ava", "Avaric" },
    { "ay", "aym", "Aymara" },
    { "az", "aze", "Azerbaijani" },
    { "ba", "bak", "Bashkir" },
    { "be", "bel", "Belarusian" },
    { "bg", "bul", "Bulgarian" },
    { "bh", "bih", "Bihari" },
    { "bi", "bis", "Bislama" },
    { "bm", "bam", "Bambara" },
    { "bn", "ben", "Bengali" },
    { "bo", "tib", "Tibetan" },
    { "br", "bre", "Breton" },
    { "bs", "bos", "Bosnian" },
    { "ca", "cat", "Catalan" },
    { "ce", "che", "Chechen" },
    { "ch", "cha", "Chamorro" },
    { "co", "cos", "Corsican" },
    { "cr", "cre", "Cree" },
    { "cs", "cze", "Czech" },
    { "cu", "chu", "Church Slavic" },
    { "cv", "chv", "Chuvash" },
    { "cy", "wel", "Welsh" },
    { "da", "dan", "Danish" },
    { "de", "ger", "German" },
    { "dv", "div", "Divehi" },
    { "dz", "dzo", "Dzongkha" },
    { "ee", "ewe", "Ewe" },
    { "el", "gre", "Greek" },
    { "en", "eng", "English" },
    { "eo", "epo", "Esperanto" },
    { "es", "spa", "Spanish" },
    { "et", "est", "Estonian" },
    { "eu", "baq", "Basque" },
    { "fa", "per", "Persian" },
    { "ff", "ful", "Fulah" },
    { "fi", "fin", "Finnish" },
    { "fj", "fij", "Fijian" },
    { "fo", "fao", "Faroese" },
    { "fr", "fre", "French" },
    { "fy", "fry", "Western Frisian" },
    { "ga", "gle", "Irish" },
    { "gd", "gla", "Scottish Gaelic" },
    { "gl", "glg", "Galician" },
    { "gn", "grn", "Guarani" },
    { "gu", "guj", "Gujarati" },
    { "gv", "glv", "Manx" },
    { "ha", "hau", "Hausa" },
    { "he", "heb", "Hebrew" },
    { "hi", "hin", "Hindi" },
    { "ho", "hmo", "Hiri Motu" },
    { "hr", "hrv", "Croatian" },
    { "ht", "hat", "Haitian" },
    { "hu", "hun", "Hungarian" },
    { "hy", "arm", "Armenian" },
    { "hz", "her", "Herero" },
    { "ia", "ina", "Interlingua" },
    { "id", "ind", "Indonesian" },
    { "ie", "ile", "Interlingue" },
    { "ig", "ibo", "Igbo" },
    { "ii", "iii", "Sichuan Yi" },
    { "ik", "ipk", "Inupiaq" },
    { "io", "ido", "Ido" },
    { "is", "ice", "Icelandic" },
    { "it", "ita", "Italian" },
    { "iu", "iku", "Inuktitut" },
    { "ja", "jpn", "Japanese" },
    { "jv", "jav", "Javanese" },
    { "ka", "geo", "Georgian" },
    { "kg", "kon", "Kongo" },
    { "ki", "kik", "Kikuyu" },
    { "kj", "kua", "Kwanyama" },
    { "kk", "kaz", "Kazakh" },
    { "kl", "kal", "Kalaallisut" },
    { "km", "khm", "Khmer" },
    { "kn", "kan", "Kannada" },
    { "ko", "kor", "Korean" },
    { "kr", "kau", "Kanuri" },
    { "ks", "kas", "Kashmiri" },
    { "ku", "kur", "Kurdish" },
    { "kv", "kom", "Komi" },
    { "kw", "cor", "Cornish" },
    { "ky", "kir", "Kirghiz" },
    { "la", "lat", "Latin" },
    { "lb", "ltz", "Luxembourgish" },
    { "lg", "lug", "Ganda" },
    { "li", "lim", "Limburgish" },
    { "ln", "lin", "Lingala" },
    { "lo", "lao", "Lao" },
    { "lt", "lit", "Lithuanian" },
    { "lu", "lub", "Luba-Katanga" },
    { "lv", "lav", "Latvian" },
    { "mg", "mlg", "Malagasy" },
    { "mh", "mah", "Marshallese" },
    { "mi", "mao", "Maori" },
    { "mk", "mac", "Macedonian" },
    { "ml", "mal", "Malayalam" },
    { "mn", "mon", "Mongolian" },
    { "mr", "mar", "Marathi" },
    { "ms", "may", "Malay" },
    { "mt", "mlt", "Maltese" },
    { "my", "bur", "Burmese" },
    { "na", "nau", "Nauru" },
    { "nb", "nob", "Norwegian Bokmal" },
    { "nd", "nde", "North Ndebele" },
    { "ne", "nep", "Nepali" },
    { "ng", "ndo", "Ndonga" },
    { "nl", "dut", "Dutch" },
    { "nn", "nno", "Norwegian Nynorsk" },
    { "no", "nor", "Norwegian" },
    { "nr", "nbl", "South Ndebele" },
    { "nv", "nav", "Navajo" },
    { "ny", "nya", "Chichewa" },
    { "oc", "oci", "Occitan" },
    { "oj", "oji", "Ojibwa" },
    { "om", "orm", "Oromo" },
    { "or", "ori", "Oriya" },
    { "os", "oss", "Ossetian" },
    { "pa", "pan", "Panjabi" },
    { "pi", "pli", "Pali" },
    { "pl", "pol", "Polish" },
    { "ps", "pus", "Pashto" },
    { "pt", "por", "Portuguese" },
    { "qu", "que", "Quechua" },
    { "rm", "roh", "Raeto-Romance" },
    { "rn", "run", "Kirundi" },
    { "ro", "rum", "Romanian" },
    { "ru", "rus", "Russian" },
    { "rw", "kin", "Kinyarwanda" },
    { "sa", "san", "Sanskrit" },
    { "sc", "srd", "Sardinian" },
    { "sd", "snd", "Sindhi" },
    { "se", "sme", "Northern Sami" },
    { "sg", "sag", "Sango" },
    { "sh", "shr", "Serbo-Croatian" },
    { "si", "sin", "Sinhala" },
    { "sk", "slo", "Slovak" },
    { "sl", "slv", "Slovenian" },
    { "sm", "smo", "Samoan" },
    { "sn", "sna", "Shona" },
    { "so", "som", "Somali" },
    { "sq", "alb", "Albanian" },
    { "sr", "srp", "Serbian" },
    { "ss", "ssw", "Swati" },
    { "st", "sot", "Southern Sotho" },
    { "su", "sun", "Sundanese" },
    { "sv", "swe", "Swedish" },
    { "sw", "swa", "Swahili" },
    { "ta", "tam", "Tamil" },
    { "te", "tel", "Telugu" },
    { "tg", "tgk", "Tajik" },
    { "th", "tha", "Thai" },
    { "ti", "tir", "Tigrinya" },
    { "tk", "tuk", "Turkmen" },
    { "tl", "tgl", "Tagalog" },
    { "tn", "tsn", "Tswana" },
    { "to", "ton", "Tonga" },
    { "tr", "tur", "Turkish" },
    { "ts", "tso", "Tsonga" },
    { "tt", "tat", "Tatar" },
    { "tw", "twi", "Twi" },
    { "ty", "tah", "Tahitian" },
    { "ug", "uig", "Uighur" },
    { "uk", "ukr", "Ukrainian" },
    { "ur", "urd", "Urdu" },
    { "uz", "uzb", "Uzbek" },
    { "ve", "ven", "Venda" },
    { "vi", "vie", "Vietnamese" },
    { "vo", "vol", "Volapuk" },
    { "wa", "wln", "Walloon" },
    { "wo", "wol", "Wolof" },
    { "xh", "xho", "Xhosa" },
    { "yi", "yid", "Yiddish" },
    { "yo", "yor", "Yoruba" },
    { "za", "zha", "Zhuang" },
    { "zh", "chi", "Chinese" },
    { "zu", "zul", "Zulu" },

    { "", "", "" }

};

template <typename T, int N>
inline int arraySize(T (&)[N])   { return N; }


struct LangStruct
{
  int order;
  quint32 code;
  QIcon icon;
  QString lang;
};

class LangCoder
{
public:
  LangCoder();

  static quint32 code2toInt(const char code[2])
  { return ( ((quint32)code[1]) << 8 ) + (quint32)code[0]; }

  static QString intToCode2( quint32 );

  static quint32 code3toInt(const std::string& code3);

  /// Finds the id for the given language name, written in english. The search
  /// is case- and punctuation insensitive.
  static quint32 findIdForLanguage( gd::wstring const & );

  static quint32 findIdForLanguageCode3( const char * );

  static QPair<quint32,quint32> findIdsForName( QString const & );
  static QPair<quint32,quint32> findIdsForFilename( QString const & );

  static quint32 guessId( const QString & lang );

  /// Returns decoded name of language or empty string if not found.
  static QString decode(quint32 code);
  /// Returns icon for language or empty string if not found.
  static QIcon icon(quint32 code);

  //const QMap<quint32, int>& codes() { return codeMap; }

  LangStruct langStruct(quint32 code);

//	QString CodeToHtml(const QString &code);

//	bool CheckCode(QString &code);

private:
  QMap<quint32, int> codeMap;
//	LangStruct dummyLS;
};

//extern LangCoder langCoder;

///////////////////////////////////////////////////////////////////////////////

#define LangCodeRole	Qt::UserRole

/*
class LangModel : public QAbstractItemModel
{
public:
  LangModel(QObject * parent = 0);

  virtual int columnCount ( const QModelIndex & parent = QModelIndex(}, const;
  virtual int rowCount ( const QModelIndex & parent = QModelIndex(}, const;

  virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

  virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex(}, const;
  virtual QModelIndex parent ( const QModelIndex & index ) const;
};
*/

#endif // LANGCODER_H
