/*
 * Mutex.h
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#ifndef Mutex_h
#define Mutex_h

#include <pthread.h>

namespace yanbo {

class Mutex {
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();
    void tryLock();

    pthread_mutex_t* getMutex();

private:
    pthread_mutex_t m_lock;
};
}

#endif
