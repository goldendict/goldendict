#ifdef __WIN32

#include <stdio.h>
#include <fcntl.h>
#include <windows.h>

#include "ufile.hh"

FILE *gd_fopen( const char *filename, const char *mode)
{
    wchar_t wname[16384], wmode[32];

    if( MultiByteToWideChar( CP_UTF8, 0, filename, -1, wname, 16384 ) == 0 )
        return NULL;
    if( MultiByteToWideChar( CP_UTF8, 0, mode, -1, wmode, 32 ) == 0 )
        return NULL;
    return _wfopen( wname, wmode );
}

int gd_open( const char *filename)
{
    wchar_t wname[16384];

    if( MultiByteToWideChar( CP_UTF8, 0, filename, -1, wname, 16384 ) == 0 )
        return -1;
    return _wopen( wname, _O_RDONLY | _O_BINARY );
}

#endif
