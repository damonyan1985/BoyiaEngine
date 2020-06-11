/*
 * TaskThread.cpp
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#include "TaskThread.h"
#include "AutoLock.h"
#include "SalLog.h"

namespace yanbo {

TaskThread::TaskThread(BlockQueue* queue)
    : m_queue(queue)
    , m_continue(LTrue)
    , m_working(LFalse)
{
}

TaskThread::~TaskThread()
{
}

LBool TaskThread::working()
{
    return m_working;
}

void TaskThread::run()
{
    while (m_continue) {
        if (!m_queue) {
            return;
        }

        BoyiaPtr<TaskBase> task = m_queue->pollTask();
        if (task.get()) {
            m_working = LTrue;
            KFORMATLOG("TaskThread::run listsize=%d", m_queue->size());

            // 智能指针会自动释放内存
            task->execute();
            KFORMATLOG("TaskThread::run listsize=%d end", m_queue->size());
        } else {
            m_working = LFalse;
            BaseThread::waitOnNotify();
        }
    }
}

bool TaskThread::isAlive()
{
    return m_continue;
}

void TaskThread::stop()
{
    m_continue = LFalse;
}
}
