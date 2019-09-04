/*
 * Mutex.cpp
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#include "Mutex.h"

namespace yanbo {

Mutex::Mutex()
{
    pthread_mutex_init(&m_lock, NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_lock);
}

void Mutex::lock()
{
    pthread_mutex_lock(&m_lock);
}

void Mutex::unlock()
{
    pthread_mutex_unlock(&m_lock);
}

void Mutex::tryLock()
{
    pthread_mutex_trylock(&m_lock);
}

pthread_mutex_t* Mutex::getMutex()
{
    return &m_lock;
}
}
