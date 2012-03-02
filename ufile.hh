#ifndef UFILE_HH_INCLUDED
#define UFILE_HH_INCLUDED

#ifdef __WIN32

#include "zlib.h"

#ifdef __cplusplus
extern "C"
{
#endif

FILE *gd_fopen( const char *filename, const char *mode );
int gd_open( const char *filename );
gzFile gd_gzopen( const char *filename );

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#else
#define gd_fopen fopen
#define gd_gzopen( filename )  gzopen( filename, "rb" )
#endif

#endif // UFILE_HH
