/*
 * MiniThread.h
 *
 *  Created on: 2015-7-29
 *      Author: yanbo
 */
#ifndef MiniThread_h
#define MiniThread_h

#include "MiniMutex.h"
#include <pthread.h>

namespace yanbo
{

class MiniThread
{
public:
	MiniThread();
	virtual ~MiniThread();

	void						start();

    int				            wait();
    void 			            waitOnNotify();
    void				        notify();
    void                        waitTimeOut(long time);
    int                         getId();

    static  void                sleep(long time);
    virtual void				stop();

protected:
    virtual void                run() = 0;
    static void*                startThread(void* ptr);

protected:
    pthread_t                   m_thread;
    pthread_cond_t		        m_condition;
    MiniMutex     	            m_lock;
    bool                        m_running;
};
}
#endif
