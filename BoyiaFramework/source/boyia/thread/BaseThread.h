/*
 * BaseThread.h
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#ifndef Thread_h
#define Thread_h

#include "Mutex.h"
#include <pthread.h>

namespace yanbo {

class Condition;
class BaseThread {
public:
    BaseThread();
    virtual ~BaseThread();

    void start();

    int wait();
    void waitOnNotify();
    void notify();
    void waitTimeOut(long time);
    int getId();

    static void sleepMS(long time);
    virtual void stop();

protected:
    virtual void run() = 0;
    static void* startThread(void* ptr);

protected:
    // pthread_t m_thread;
    // pthread_cond_t m_condition;

    Mutex m_lock;
    bool m_running;
    Condition* m_condition;
};
}
#endif
