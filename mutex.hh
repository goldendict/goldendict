/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __MUTEX_HH_INCLUDED__
#define __MUTEX_HH_INCLUDED__
#include <QSemaphore>

#if defined(__cplusplus) && (__cplusplus >= 201103L)
#include <mutex>
#include <atomic>

class Mutex:
        #ifdef MDX_LOCALVIDEO_CACHED
        public std::recursive_mutex
        #else
        public std::mutex
        #endif
{
public:
    ~Mutex(){}
    bool tryLock();
    /// Locks the given mutex on construction and unlocks on destruction
    using Lock = std::lock_guard<Mutex>;
};

class AtomicInt32 : public std::atomic<int> {
public:
    AtomicInt32(int v = 0) : atomic<int>(v)
    {}
    ~AtomicInt32(){}

    inline int loadAcquire() const { return load(); }
    inline bool ref()  { return ++(*this); }
    inline bool deref()  { return --(*this); }
    inline void storeRelease(int newValue)  { store(newValue, std::memory_order_release); }
    inline bool testAndSetRelease(int expectedValue, int newValue)
    { return compare_exchange_strong(expectedValue, newValue, std::memory_order_release); }

};

#else
#include <QMutex>
#include <QAtomicInt>

/// This provides a mutex class. As you can see, it's just a Qt one, but it
/// does provide the Lock class which doesn't seem to exist in Qt, and it does
/// provide some abstraction for dictionaries in case they are to be ported
/// away from Qt.
class Mutex: public QMutex
{
public:
#ifdef MDX_LOCALVIDEO_CACHED
    Mutex():QMutex(Recursive) {}
#endif
    ~Mutex() {}
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

using AtomicInt32 = QAtomicInt;

#endif

#endif
