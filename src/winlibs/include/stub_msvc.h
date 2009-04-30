#include <string>

#if !defined(strcasecmp)
#  define strcasecmp  _strcmpi
#endif
#if !defined(strncasecmp)
#  define strncasecmp  _strnicmp
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
#define ssize_t long
#endif

