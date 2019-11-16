/*
 * Mutex.h
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#ifndef Mutex_h
#define Mutex_h

#include "PlatformLib.h"

#if ENABLE(BOYIA_ANDROID)
#include <pthread.h>
typedef pthread_mutex_t Lock;
#elif ENABLE(BOYIA_WINDOWS)
#include <windows.h>
typedef CRITICAL_SECTION Lock;
#endif

namespace yanbo {

class Mutex {
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();
    void tryLock();

    Lock* getMutex();

private:
    Lock m_lock;
};
}

#endif
