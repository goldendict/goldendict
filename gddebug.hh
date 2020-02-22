#ifndef __GDDEBUG_HH_INCLUDED__
#define __GDDEBUG_HH_INCLUDED__

#include <QFile>

#ifdef NO_CONSOLE
#define GD_DPRINTF(...) do {} while( 0 )
#define GD_FDPRINTF(...) do {} while( 0 )
#else
#define GD_DPRINTF(...) printf(__VA_ARGS__)
#define GD_FDPRINTF(...) fprintf(__VA_ARGS__)
#endif

#ifdef GD_LOG_MSGOUT
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
extern QFile * logFilePtr;
#else
#define gdWarning(...) do {} while( 0 )
#define gdDebug(...) do {} while( 0 )
#endif

#endif // __GDDEBUG_HH_INCLUDED__
