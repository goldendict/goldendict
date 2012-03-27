#include "langcoder.hh"
#include "folding.hh"
#include "wstring_qt.hh"
#include "language.hh"

#include <cctype>

LangCoder langCoder;


LangCoder::LangCoder()
{
  LangStruct ls;
  for (int i = 0; true; i++) {
    const LangCode &lc = LangCodes[i];
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
    const LangCode &lc = LangCodes[ langCoder.codeMap[ code ] ];
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
    const LangCode &lc = LangCodes[order];
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

  return QString::fromAscii( code, 2 );
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

  for( LangCode const * lc = LangCodes; lc->code[ 0 ]; ++lc )
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
  for( LangCode const * lc = LangCodes; lc->code[ 0 ]; ++lc )
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
    for( LangCode const * lc = LangCodes; lc->code[ 0 ]; ++lc )
    {
      if ( lstr == QString( lc->lang ) )
      {
        // We've got a match
        return code2toInt( lc->code );
      }
    }
  }

  // still not found - try to match by 2-symbol code
  return code2toInt( lstr.left(2).toAscii().data() );
}

QPair<quint32,quint32> LangCoder::findIdsForFilename( QString const & name )
{
  QString nameFolded = "|" + QFileInfo( name ).fileName().toCaseFolded() + "|";

//  qDebug() << nameFolded;

  QRegExp reg( "[^a-z]([a-z]{2,3})-([a-z]{2,3})[^a-z]" ); reg.setMinimal(true);
  int off = 0;
  while ( reg.indexIn( nameFolded, off ) >= 0 )
  {
//    qDebug() << reg.cap(1);
//    qDebug() << reg.cap(2);

    quint32 from = guessId( reg.cap(1) );
    quint32 to = guessId( reg.cap(2) );
    if (from && to)
      return QPair<quint32,quint32>(from, to);

    off += reg.matchedLength();
  }

  return QPair<quint32,quint32>(0, 0);
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
