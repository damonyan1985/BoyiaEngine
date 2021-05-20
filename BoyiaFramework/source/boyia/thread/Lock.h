/*
 * Mutex.h
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#ifndef Lock_h
#define Lock_h

#include "PlatformLib.h"

#if ENABLE(BOYIA_ANDROID)
#include <pthread.h>
typedef pthread_mutex_t PlatformLock;
#elif ENABLE(BOYIA_WINDOWS)
#include <windows.h>
typedef CRITICAL_SECTION PlatformLock;
#endif

namespace yanbo {

class Lock {
public:
    Lock();
    ~Lock();

    void lock();
    void unlock();
    void tryLock();

    PlatformLock* getLock();

private:
    PlatformLock m_lock;
};
}

#endif
