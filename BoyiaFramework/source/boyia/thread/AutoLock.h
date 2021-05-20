/*
 * AutoLock.h
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#ifndef AutoLock_h
#define AutoLock_h

#include "Lock.h"

namespace yanbo {

class AutoLock {
public:
    AutoLock(Lock* lock)
        : m_lock(lock)
    {
        m_lock->lock();
    }
    ~AutoLock()
    {
        m_lock->unlock();
    }

private:
    Lock* m_lock;
};
}

#endif
