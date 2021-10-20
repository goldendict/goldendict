#ifdef __WIN32

#include "wstring.hh"
#include "iconv.hh"
#include <wchar.h>
#include <QString>

namespace gd
{
  wstring __nativeToWs( wchar_t const * str )
  {
      QString qStr=QString::fromWCharArray(str);
      return qStr.toStdU32String();
    //return Iconv::toWstring( "WCHAR_T", str, wcslen( str ) * sizeof( wchar_t ) );
  }
}

#endif

