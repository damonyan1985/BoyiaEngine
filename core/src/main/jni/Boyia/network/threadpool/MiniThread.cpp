/*
 * MiniThreadAndroid.cpp
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#include "AutoLock.h"
#include "MiniThread.h"
#include "SalLog.h"
#include <time.h>
#include <unistd.h>

#define TAG "MiniThread"

namespace yanbo
{
MiniThread::MiniThread()
    : m_running(false)
{
}

MiniThread::~MiniThread()
{
}

void MiniThread::start()
{
    pthread_create(&m_thread, NULL, startThread, this);
}

int MiniThread::wait()
{
	if (!m_running)
	{
		return 0;
	}

	// pthread_join function means waiting for the
	// thread end
    return pthread_join(m_thread, NULL);
}

void* MiniThread::startThread(void* ptr)
{
    MiniThread* thread = (MiniThread*)ptr;
    thread->m_running = true;
    thread->run();
    thread->m_running = false;
    return ptr;
}

void MiniThread::stop()
{
}

void MiniThread::waitOnNotify()
{
	AutoLock lock(&m_lock);
	pthread_cond_wait(&m_condition, m_lock.getMutex());
}

void MiniThread::notify()
{
	AutoLock lock(&m_lock);
	pthread_cond_signal(&m_condition);
}

// millisecond毫秒级别
void MiniThread::waitTimeOut(long timeout)
{
	AutoLock lock(&m_lock);
	struct timespec outtime;
	struct timeval now;
	gettimeofday(&now, NULL);

    // 换算成纳秒， tv_usec微秒 * 1000（纳秒）
    // timeout毫秒 * 1000 * 1000（纳秒）
    long nsec = now.tv_usec * 1000 + timeout * 1000 * 1000;
	outtime.tv_sec = now.tv_sec + nsec / (1000 * 1000 * 1000);
	//outtime.tv_nsec = now.tv_usec * 1000;
	outtime.tv_nsec = nsec % (1000 * 1000 * 1000);
	pthread_cond_timedwait(&m_condition, m_lock.getMutex(), &outtime);
}

void MiniThread::sleep(long time)
{
//	struct timeval delay;
//	delay.tv_sec = 0;
//	delay.tv_usec = time; // time us
//	select(0, NULL, NULL, NULL, &delay);

	usleep(time);
}

int MiniThread::getId()
{
    return (int)pthread_self();
}

}
