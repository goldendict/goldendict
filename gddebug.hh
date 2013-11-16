#ifndef __GDDEBUG_HH_INCLUDED__
#define __GDDEBUG_HH_INCLUDED__

#include <QFile>

#ifdef NO_CONSOLE
#define DPRINTF(...)
#define FDPRINTF(...)
#else
#define DPRINTF(...) printf(__VA_ARGS__)
#define FDPRINTF(...) fprintf(__VA_ARGS__)
#endif

void gdWarning(const char *, ...) /* print warning message */
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

void gdDebug(const char *, ...)
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

extern QFile logFile;

#endif // __GDDEBUG_HH_INCLUDED__
