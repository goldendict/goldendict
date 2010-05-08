/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mutex.hh"

Mutex::Lock::Lock( Mutex & m_ ): m( m_ )
{
  m.lock();
}

Mutex::Lock::~Lock()
{
  m.unlock();
}
