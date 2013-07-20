#ifndef LANGCODER_H
#define LANGCODER_H

#include <QtGui>
#include "wstring.hh"

struct GDLangCode
{
    char code[ 3 ]; // ISO 639-1
    char code3[ 4 ]; // ISO 639-2B ( http://www.loc.gov/standards/iso639-2/ )
    int isRTL; // Right-to-left writing; 0 - no, 1 - yes, -1 - let Qt define
    char const * lang; // Language name in English
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

  /// Return true for RTL languages
  static bool isLanguageRTL(quint32 code);

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
