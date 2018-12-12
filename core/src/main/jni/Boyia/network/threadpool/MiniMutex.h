/*
 * MiniMutex.h
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#ifndef MiniMutex_h
#define MiniMutex_h

#include <pthread.h>

namespace yanbo
{
class MiniMutex
{
public:
	MiniMutex();
	~MiniMutex();

	void lock();
	void unlock();
	void tryLock();

	pthread_mutex_t* getMutex();

private:
	pthread_mutex_t     	m_lock;
};
}

#endif
