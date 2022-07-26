/*
 * ThreadPool.cpp
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#include "ThreadPool.h"

namespace yanbo {

const int KThreadPoolSize = 9;
ThreadPool::ThreadPool()
{
    m_queue = new BlockQueue();
}

ThreadPool::~ThreadPool()
{
    KList<TaskThread*>::Iterator iter = m_threadList.begin();
    KList<TaskThread*>::Iterator iterEnd = m_threadList.end();
    for (; iter != iterEnd; ++iter) {
        (*iter)->stop();
    }

    m_threadList.clear();
}

ThreadPool* ThreadPool::getInstance()
{
    static ThreadPool sPool;
    return &sPool;
}

void ThreadPool::destroy()
{
}

void ThreadPool::sendTask(TaskBase* task)
{
    KFORMATLOG("sendTask TaskThread::run listsize=%d", m_queue->size());
    // 添加task
    {
        m_queue->addTask(task);
    }

    KList<TaskThread*>::Iterator iter = m_threadList.begin();
    KList<TaskThread*>::Iterator iterEnd = m_threadList.end();
    for (; iter != iterEnd; ++iter) {
        if (!(*iter)->working()) {
            (*iter)->notify();
            return;
        }
    }

    if (m_threadList.count() < KThreadPoolSize) {
        TaskThread* thread = new TaskThread(m_queue.get());
        m_threadList.push(thread);
        thread->start();
    }
}
}
