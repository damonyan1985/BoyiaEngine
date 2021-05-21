/*
 * LockAndroid.cpp
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */

#include "Lock.h"
#if ENABLE(BOYIA_ANDROID)
namespace yanbo {

Lock::Lock()
{
    pthread_mutex_init(&m_lock, NULL);
}

Lock::~Lock()
{
    pthread_mutex_destroy(&m_lock);
}

void Lock::lock()
{
    pthread_mutex_lock(&m_lock);
}

void Lock::unlock()
{
    pthread_mutex_unlock(&m_lock);
}

void Lock::tryLock()
{
    pthread_mutex_trylock(&m_lock);
}

PlatformLock* Lock::getLock()
{
    return &m_lock;
}
}

#endif
