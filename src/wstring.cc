#ifdef __WIN32

#include "wstring.hh"
#include "iconv.hh"
#include <wchar.h>

namespace gd
{
  wstring __nativeToWs( wchar_t const * str )
  {
    return Iconv::toWstring( "WCHAR_T", str, wcslen( str ) * sizeof( wchar_t ) );
  }
}

#endif

