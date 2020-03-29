/*
 * BaseThread.cpp
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#include "AutoLock.h"
#include "BaseThread.h"
#include "SalLog.h"
#if ENABLE(BOYIA_ANDROID)
#include <time.h>
#include <unistd.h>

#define TAG "BaseThread"

const LInt kThreadStackSize = 256 * 1024;

namespace yanbo {

class Condition {
public:
    Condition() {}
    pthread_t thread;
    pthread_cond_t condition;
};

BaseThread::BaseThread()
    : m_running(false)
    , m_condition(new Condition())
{
}

BaseThread::~BaseThread()
{
    delete m_condition;
}

void BaseThread::start()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, kThreadStackSize);
    pthread_create(&m_condition->thread, NULL, startThread, this);
    pthread_attr_destroy(&attr);
}

int BaseThread::wait()
{
    if (!m_running) {
        return 0;
    }

    // pthread_join function means waiting for the
    // thread end
    return pthread_join(m_condition->thread, NULL);
}

void* BaseThread::startThread(void* ptr)
{
    BaseThread* thread = (BaseThread*)ptr;
    thread->m_running = true;
    thread->run();
    thread->m_running = false;
    return ptr;
}

void BaseThread::stop()
{
}

void BaseThread::waitOnNotify()
{
    AutoLock lock(&m_lock);
    pthread_cond_wait(&m_condition->condition, m_lock.getMutex());
}

void BaseThread::notify()
{
    AutoLock lock(&m_lock);
    pthread_cond_signal(&m_condition->condition);
}

// millisecond毫秒级别
void BaseThread::waitTimeOut(long timeout)
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
    pthread_cond_timedwait(&m_condition->condition, m_lock.getMutex(), &outtime);
}

// milliseconds
void BaseThread::sleepMS(long time)
{
    //	struct timeval delay;
    //	delay.tv_sec = 0;
    //	delay.tv_usec = time; // time us
    //	select(0, NULL, NULL, NULL, &delay);

    //	usleep(time);
    struct timespec ts;
    ts.tv_sec = time / 1000;
    ts.tv_nsec = (time % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int BaseThread::getId()
{
    return (int)pthread_self();
}
}

#endif
