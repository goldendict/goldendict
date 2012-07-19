#include "wstring_qt.hh"
#include <QVector>

namespace gd
{
  #ifdef __WIN32

  QString toQString( wstring const & in )
  {
    return QString::fromUcs4( in.c_str() );
  }

  wstring toWString( QString const & in )
  {
    QVector< unsigned int > v = in.toUcs4();

    // Fix for CJK Extension B characters
    int n = v.size();
    while( n > 0 && v[ n - 1 ] == 0 ) n--;
    if( n != v.size() )
        v.resize( n );

    return wstring( v.constData(), v.size() );
  }

  #else

  QString toQString( wstring const & in )
  {
    return QString::fromStdWString( in );
  }

  wstring toWString( QString const & in )
  {
    return in.toStdWString();
  }
  #endif
}
