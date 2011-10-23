#ifndef UFILE_HH_INCLUDED
#define UFILE_HH_INCLUDED

#ifdef __WIN32

#ifdef __cplusplus
extern "C"
{
#endif

FILE *gd_fopen( const char *filename, const char *mode );
int gd_open( const char *filename);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#else
#define gd_fopen fopen
#endif

#endif // UFILE_HH
