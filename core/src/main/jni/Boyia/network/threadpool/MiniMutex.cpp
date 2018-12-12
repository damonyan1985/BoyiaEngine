/*
 * MiniMutex.cpp
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#include "MiniMutex.h"

namespace yanbo
{
MiniMutex::MiniMutex()
{
	pthread_mutex_init(&m_lock, NULL);
}

MiniMutex::~MiniMutex()
{
    pthread_mutex_destroy(&m_lock);
}

void MiniMutex::lock()
{
    pthread_mutex_lock(&m_lock);
}

void MiniMutex::unlock()
{
    pthread_mutex_unlock(&m_lock);
}

void MiniMutex::tryLock()
{
    pthread_mutex_trylock(&m_lock);
}

pthread_mutex_t* MiniMutex::getMutex()
{
	return &m_lock;
}

}
