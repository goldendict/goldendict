/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __MUTEX_HH_INCLUDED__
#define __MUTEX_HH_INCLUDED__

#include <QMutex>

/// This provides a mutex class. As you can see, it's just a Qt one, but it
/// does provide the Lock class which doesn't seem to exist in Qt, and it does
/// provide some abstraction for dictionaries in case they are to be ported
/// away from Qt.
class Mutex : public QMutex
{
public:
  Mutex() : QMutex()
  {}
  ~Mutex()
  {}

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
