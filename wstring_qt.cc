#include "wstring_qt.hh"
#include <QVector>

namespace gd
{
  QString toQString( wstring const & in )
  {
    return QString::fromStdU32String( in );
  }

  wstring toWString( QString const & in )
  {
    return in.toStdU32String();
  }

  wstring normalize( const wstring & str )
  {
    return gd::toWString( gd::toQString( str ).normalized( QString::NormalizationForm_C ) );
  }

  std::string toStdString(const QString& str)
  {
      return str.toStdString();
  }

}
