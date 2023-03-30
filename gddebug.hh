#ifndef __GDDEBUG_HH_INCLUDED__
#define __GDDEBUG_HH_INCLUDED__

#include <QFile>

class QTextCodec;

#ifdef NO_CONSOLE
  #define GD_DPRINTF(...) do {} while( 0 )
  #define GD_FDPRINTF(...) do {} while( 0 )
#else
  #ifdef NO_GD_DPRINTF
    #define GD_DPRINTF(...) do {} while( 0 )
  #else
    #define GD_DPRINTF(...) printf(__VA_ARGS__)
  #endif
  #define GD_FDPRINTF(...) fprintf(__VA_ARGS__)
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

QTextCodec * gdCodecForLocale();

extern QFile * logFilePtr;

#endif // __GDDEBUG_HH_INCLUDED__
