/*
 * MiniMutex.cpp
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#include "MiniThreadPool.h"

namespace yanbo
{
const int KMiniThreadPoolSize = 1;
// 注意线程安全, 使用饿汉模式
MiniThreadPool* MiniThreadPool::s_pool = new MiniThreadPool();
MiniThreadPool::MiniThreadPool()
{
	m_queue = new MiniBlockQueue();
}

MiniThreadPool::~MiniThreadPool()
{
	KList<KRefPtr<MiniTaskThread> >::Iterator iter = m_threadList.begin();
	KList<KRefPtr<MiniTaskThread> >::Iterator iterEnd = m_threadList.end();
	for (; iter != iterEnd; ++iter)
	{
		(*iter)->stop();
	}

	m_threadList.clear();
}

MiniThreadPool* MiniThreadPool::getInstance()
{
    return s_pool;
}

void MiniThreadPool::destroy()
{
	if (s_pool != NULL)
	{
		delete s_pool;
		s_pool = NULL;
	}
}

void MiniThreadPool::sendMiniTask(MiniTaskBase* task)
{
	KFORMATLOG("sendMiniTask MiniTaskThread::run listsize=%d", m_queue->size());
	// 添加task
	{
		m_queue->addTask(task);
	}

	KList<KRefPtr<MiniTaskThread> >::Iterator iter = m_threadList.begin();
	KList<KRefPtr<MiniTaskThread> >::Iterator iterEnd = m_threadList.end();
	for (; iter != iterEnd; ++iter)
	{
        if (!(*iter)->working())
        {
        	(*iter)->notify();
        	return;
        }
	}

	if (m_threadList.count() < KMiniThreadPoolSize)
	{
		KRefPtr<MiniTaskThread> thread = new MiniTaskThread(m_queue.get());
        m_threadList.push(thread);
        thread->start();
	}
}



}
