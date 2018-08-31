/*
 * MiniMutex.cpp
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#include "MiniTaskThread.h"
#include "AutoLock.h"
#include "SalLog.h"

namespace yanbo
{
MiniTaskThread::MiniTaskThread(MiniBlockQueue* queue)
    : m_queue(queue)
    , m_continue(LTrue)
    , m_working(LFalse)
{
}

MiniTaskThread::~MiniTaskThread()
{
}

LBool MiniTaskThread::working()
{
	return m_working;
}

void MiniTaskThread::run()
{
    while (m_continue)
    {
    	if (m_queue == NULL)
    	{
    		return;
    	}


    	KRefPtr<MiniTaskBase> task = m_queue->pollTask();
        if (task.get() != NULL)
        {
        	m_working = LTrue;
        	KFORMATLOG("MiniTaskThread::run listsize=%d", m_queue->size());

        	// 智能指针会自动释放内存
        	task->execute();
        	KFORMATLOG("MiniTaskThread::run listsize=%d end", m_queue->size());
        }
        else
        {
        	m_working = LFalse;
        	MiniThread::waitOnNotify();
        }
    }
}

void MiniTaskThread::stop()
{
	m_continue = LFalse;
}

}
