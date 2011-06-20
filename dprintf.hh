#ifndef __DPRINTF_HH_INCLUDED__
#define __DPRINTF_HH_INCLUDED__

#ifdef NO_CONSOLE
#define DPRINTF(...)
#define FDPRINTF(...)
#else
#define DPRINTF(...) printf(__VA_ARGS__)
#define FDPRINTF(...) fprintf(__VA_ARGS__)
#endif

#endif // __DPRINTF_HH_INCLUDED__
