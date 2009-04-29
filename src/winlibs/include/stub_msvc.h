#include <string>

#if !defined(strcasecmp)
#  define strcasecmp  _strcmpi
#endif
#if !defined(strncasecmp)
#  define strncasecmp  _strnicmp
#endif
