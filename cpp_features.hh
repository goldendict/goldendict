#ifndef __CPP_HH_INCLUDED__
#define __CPP_HH_INCLUDED__

#ifdef _MSC_VER
#if _MSC_VER >= 1600
#define strcasecmp stricmp
#define strncasecmp  strnicmp
#else
#include <stub_msvc.h>
#endif
#endif

#if __cplusplus > 199711L
#define THROW_SPEC(...)
#else
#define THROW_SPEC(...) throw(__VA_ARGS__)
#endif

#endif // CPP_HH
