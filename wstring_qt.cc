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
    QVector< unsigned int > v = in.toUcs4();

    // Fix for QString instance which contains non-BMP characters
    // Qt will created unexpected null characters may confuse btree indexer.
    // Related: https://bugreports.qt-project.org/browse/QTBUG-25536
    int n = v.size();
    while ( n > 0 && v[ n - 1 ] == 0 ) n--;
    if ( n != v.size() )
      v.resize( n );

    return wstring( ( const wchar * ) v.constData(), v.size() );
  }

  wstring normalize( const wstring & str )
  {
    return gd::toWString( gd::toQString( str ).normalized( QString::NormalizationForm_C ) );
  }

}
