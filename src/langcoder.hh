#ifndef LANGCODER_H
#define LANGCODER_H

#include <QtGui>
#include "wstring.hh"

struct LangCode
{
    char code[ 3 ]; // ISO 639-1
    char *lang; // Language name in English
};

// Language codes

const LangCode LangCodes[] = {

    { "aa",  "Afar" },
    { "ab",  "Abkhazian" },
    { "ae",  "Avestan" },
    { "af",  "Afrikaans" },
    { "ak",  "Akan" },
    { "am",  "Amharic" },
    { "an",  "Aragonese" },
    { "ar",  "Arabic" },
    { "as",  "Assamese" },
    { "av",  "Avaric" },
    { "ay",  "Aymara" },
    { "az",  "Azerbaijani" },
    { "ba",  "Bashkir" },
    { "be",  "Belarusian" },
    { "bg",  "Bulgarian" },
    { "bh",  "Bihari" },
    { "bi",  "Bislama" },
    { "bm",  "Bambara" },
    { "bn",  "Bengali" },
    { "bo",  "Tibetan" },
    { "br",  "Breton" },
    { "bs",  "Bosnian" },
    { "ca",  "Catalan" },
    { "ce",  "Chechen" },
    { "ch",  "Chamorro" },
    { "co",  "Corsican" },
    { "cr",  "Cree" },
    { "cs",  "Czech" },
    { "cu",  "Church Slavic" },
    { "cv",  "Chuvash" },
    { "cy",  "Welsh" },
    { "da",  "Danish" },
    { "de",  "German" },
    { "dv",  "Divehi" },
    { "dz",  "Dzongkha" },
    { "ee",  "Ewe" },
    { "el",  "Greek" },
    { "en",  "English" },
    { "eo",  "Esperanto" },
    { "es",  "Spanish" },
    { "et",  "Estonian" },
    { "eu",  "Basque" },
    { "fa",  "Persian" },
    { "ff",  "Fulah" },
    { "fi",  "Finnish" },
    { "fj",  "Fijian" },
    { "fo",  "Faroese" },
    { "fr",  "French" },
    { "fy",  "Western Frisian" },
    { "ga",  "Irish" },
    { "gd",  "Scottish Gaelic" },
    { "gl",  "Galician" },
    { "gn",  "Guarani" },
    { "gu",  "Gujarati" },
    { "gv",  "Manx" },
    { "ha",  "Hausa" },
    { "he",  "Hebrew" },
    { "hi",  "Hindi" },
    { "ho",  "Hiri Motu" },
    { "hr",  "Croatian" },
    { "ht",  "Haitian" },
    { "hu",  "Hungarian" },
    { "hy",  "Armenian" },
    { "hz",  "Herero" },
    { "ia",  "Interlingua" },
    { "id",  "Indonesian" },
    { "ie",  "Interlingue" },
    { "ig",  "Igbo" },
    { "ii",  "Sichuan Yi" },
    { "ik",  "Inupiaq" },
    { "io",  "Ido" },
    { "is",  "Icelandic" },
    { "it",  "Italian" },
    { "iu",  "Inuktitut" },
    { "ja",  "Japanese" },
    { "jv",  "Javanese" },
    { "ka",  "Georgian" },
    { "kg",  "Kongo" },
    { "ki",  "Kikuyu" },
    { "kj",  "Kwanyama" },
    { "kk",  "Kazakh" },
    { "kl",  "Kalaallisut" },
    { "km",  "Khmer" },
    { "kn",  "Kannada" },
    { "ko",  "Korean" },
    { "kr",  "Kanuri" },
    { "ks",  "Kashmiri" },
    { "ku",  "Kurdish" },
    { "kv",  "Komi" },
    { "kw",  "Cornish" },
    { "ky",  "Kirghiz" },
    { "la",  "Latin" },
    { "lb",  "Luxembourgish" },
    { "lg",  "Ganda" },
    { "li",  "Limburgish" },
    { "ln",  "Lingala" },
    { "lo",  "Lao" },
    { "lt",  "Lithuanian" },
    { "lu",  "Luba-Katanga" },
    { "lv",  "Latvian" },
    { "mg",  "Malagasy" },
    { "mh",  "Marshallese" },
    { "mi",  "Maori" },
    { "mk",  "Macedonian" },
    { "ml",  "Malayalam" },
    { "mn",  "Mongolian" },
    { "mr",  "Marathi" },
    { "ms",  "Malay" },
    { "mt",  "Maltese" },
    { "my",  "Burmese" },
    { "na",  "Nauru" },
    { "nb",  "Norwegian Bokmal" },
    { "nd",  "North Ndebele" },
    { "ne",  "Nepali" },
    { "ng",  "Ndonga" },
    { "nl",  "Dutch" },
    { "nn",  "Norwegian Nynorsk" },
    { "no",  "Norwegian" },
    { "nr",  "South Ndebele" },
    { "nv",  "Navajo" },
    { "ny",  "Chichewa" },
    { "oc",  "Occitan" },
    { "oj",  "Ojibwa" },
    { "om",  "Oromo" },
    { "or",  "Oriya" },
    { "os",  "Ossetian" },
    { "pa",  "Panjabi" },
    { "pi",  "Pali" },
    { "pl",  "Polish" },
    { "ps",  "Pashto" },
    { "pt",  "Portuguese" },
    { "qu",  "Quechua" },
    { "rm",  "Raeto-Romance" },
    { "rn",  "Kirundi" },
    { "ro",  "Romanian" },
    { "ru",  "Russian" },
    { "rw",  "Kinyarwanda" },
    { "sa",  "Sanskrit" },
    { "sc",  "Sardinian" },
    { "sd",  "Sindhi" },
    { "se",  "Northern Sami" },
    { "sg",  "Sango" },
    { "sh",  "Serbo-Croatian" },
    { "si",  "Sinhala" },
    { "sk",  "Slovak" },
    { "sl",  "Slovenian" },
    { "sm",  "Samoan" },
    { "sn",  "Shona" },
    { "so",  "Somali" },
    { "sq",  "Albanian" },
    { "sr",  "Serbian" },
    { "ss",  "Swati" },
    { "st",  "Southern Sotho" },
    { "su",  "Sundanese" },
    { "sv",  "Swedish" },
    { "sw",  "Swahili" },
    { "ta",  "Tamil" },
    { "te",  "Telugu" },
    { "tg",  "Tajik" },
    { "th",  "Thai" },
    { "ti",  "Tigrinya" },
    { "tk",  "Turkmen" },
    { "tl",  "Tagalog" },
    { "tn",  "Tswana" },
    { "to",  "Tonga" },
    { "tr",  "Turkish" },
    { "ts",  "Tsonga" },
    { "tt",  "Tatar" },
    { "tw",  "Twi" },
    { "ty",  "Tahitian" },
    { "ug",  "Uighur" },
    { "uk",  "Ukrainian" },
    { "ur",  "Urdu" },
    { "uz",  "Uzbek" },
    { "ve",  "Venda" },
    { "vi",  "Vietnamese" },
    { "vo",  "Volapuk" },
    { "wa",  "Walloon" },
    { "wo",  "Wolof" },
    { "xh",  "Xhosa" },
    { "yi",  "Yiddish" },
    { "yo",  "Yoruba" },
    { "za",  "Zhuang" },
    { "zh",  "Chinese" },
    { "zu",  "Zulu" },

    { "", "" }

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

  static quint32 code3toInt(const std::string& code3);

  /// Finds the id for the given language name, written in english. The search
  /// is case- and punctuation insensitive.
  static quint32 findIdForLanguage( gd::wstring const & );


  static QPair<quint32,quint32> findIdsForFilename( QString const & );

  static quint32 guessId( const QString & lang );

  /// Returns decoded name of language or empty string if not found.
  static QString decode(quint32 code);

  //const QMap<quint32, int>& codes() { return codeMap; }

  LangStruct langStruct(quint32 code);

//	QString CodeToHtml(const QString &code);

//	bool CheckCode(QString &code);

protected:
  QMap<quint32, int> codeMap;
//	LangStruct dummyLS;
};

extern LangCoder langCoder;

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
