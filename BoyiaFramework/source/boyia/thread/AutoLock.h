/*
 * AutoLock.h
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#ifndef AutoLock_h
#define AutoLock_h

#include "Mutex.h"

namespace yanbo {

class AutoLock {
public:
    AutoLock(Mutex* lock)
        : m_lock(lock)
    {
        m_lock->lock();
    }
    ~AutoLock()
    {
        m_lock->unlock();
    }

private:
    Mutex* m_lock;
};
}

#endif
