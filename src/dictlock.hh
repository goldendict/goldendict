/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __DICTLOCK_HH_INCLUDED__
#define __DICTLOCK_HH_INCLUDED__

/// This is a one large coarse-grained lock class used to lock all the
/// dictionaries and all their shared containers (usually vectors) in order
/// to work with them concurrently in different threads. Since we do matching
/// in a different thread, we need this, and since matching doesn't happen
/// too much and doesn't typically take a lot of time, there's no need for any
/// more fine-grained locking approaches.
/// Create this object before working with any dictionary objects or their
/// containers which are known to be shared with the other objects. Destory
/// this object when you're done (i.e,, leave the scope where it was declared).
class DictLock
{
public:

  DictLock();
  ~DictLock();
};

#endif
