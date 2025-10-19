/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __MUTEX_HH_INCLUDED__
#define __MUTEX_HH_INCLUDED__

#include <QMutex>

/// This provides a mutex class. As you can see, it's just a Qt one, but it
/// does provide the Lock class which doesn't seem to exist in Qt, and it does
/// provide some abstraction for dictionaries in case they are to be ported
/// away from Qt.
class Mutex: public QMutex
{
public:

// Only the MdxDictionary::getCachedFileName() workaround, which is restricted to the Qt WebKit version, relies on
// the recursiveness of Mutex. Keep Mutex nonrecursive in the Qt WebEngine version to avoid performance penalty.
#ifdef USE_QTWEBKIT
   // `#pragma GCC diagnostic push` and `#pragma GCC diagnostic pop` were introduced in GCC 4.6.
#  if __GNUC__ > 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 6 )
#    pragma GCC diagnostic push
     // QMutex::Recursive is deprecated in Qt 5.15 and absent from Qt 6.0+. It is used only in the Qt WebKit version.
     // Qt WebKit has not been properly ported to Qt 6. So this deprecation warning is not a problem and can be ignored.
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#  endif

  Mutex() : QMutex( Recursive )
  {}
  ~Mutex()
  {}

#  if __GNUC__ > 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 6 )
#    pragma GCC diagnostic pop
#  endif
#endif // USE_QTWEBKIT

  /// Locks the given mutex on construction and unlocks on destruction
  class Lock
  {
    Mutex & m;
    
  public:
    
    Lock( Mutex & );
    ~Lock();
    
  private:
    Lock( Lock const & );
  };
};

#endif
