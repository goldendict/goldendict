/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "dictlock.hh"
#include <QMutex>

namespace
{
  QMutex & mutexInstance()
  {
    static QMutex mutex;
  
    return mutex;
  }
}

DictLock::DictLock()
{
  mutexInstance().lock();
}

DictLock::~DictLock()
{
  mutexInstance().unlock();
}
