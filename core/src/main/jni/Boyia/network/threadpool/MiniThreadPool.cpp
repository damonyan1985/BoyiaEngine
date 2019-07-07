/*
 * MiniMutex.cpp
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#include "MiniThreadPool.h"

namespace yanbo {

const int KMiniThreadPoolSize = 9;
MiniThreadPool::MiniThreadPool()
{
    m_queue = new MiniBlockQueue();
}

MiniThreadPool::~MiniThreadPool()
{
    KList<BoyiaPtr<MiniTaskThread>>::Iterator iter = m_threadList.begin();
    KList<BoyiaPtr<MiniTaskThread>>::Iterator iterEnd = m_threadList.end();
    for (; iter != iterEnd; ++iter) {
        (*iter)->stop();
    }

    m_threadList.clear();
}

MiniThreadPool* MiniThreadPool::getInstance()
{
    static MiniThreadPool sPool;
    return &sPool;
}

void MiniThreadPool::destroy()
{
}

void MiniThreadPool::sendMiniTask(MiniTaskBase* task)
{
    KFORMATLOG("sendMiniTask MiniTaskThread::run listsize=%d", m_queue->size());
    // 添加task
    {
        m_queue->addTask(task);
    }

    KList<BoyiaPtr<MiniTaskThread>>::Iterator iter = m_threadList.begin();
    KList<BoyiaPtr<MiniTaskThread>>::Iterator iterEnd = m_threadList.end();
    for (; iter != iterEnd; ++iter) {
        if (!(*iter)->working()) {
            (*iter)->notify();
            return;
        }
    }

    if (m_threadList.count() < KMiniThreadPoolSize) {
        BoyiaPtr<MiniTaskThread> thread = new MiniTaskThread(m_queue.get());
        m_threadList.push(thread);
        thread->start();
    }
}
}
