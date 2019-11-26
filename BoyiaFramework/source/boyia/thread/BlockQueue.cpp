/*
 * BlockQueue.cpp
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#include "BlockQueue.h"
#include "AutoLock.h"

namespace yanbo {

BlockQueue::BlockQueue()
{
}

BlockQueue::~BlockQueue()
{
    clear();
}

void BlockQueue::addTask(TaskBase* task)
{
    AutoLock lock(&m_queueMutex);
    m_list.push(task);
}

BoyiaPtr<TaskBase> BlockQueue::pollTask()
{
    AutoLock lock(&m_queueMutex);
    BoyiaPtr<TaskBase> task = kBoyiaNull;
    if (!m_list.empty()) {
        KList<BoyiaPtr<TaskBase>>::Iterator iter = m_list.begin();
        task = *iter;
        m_list.erase(iter);
    }

    return task;
}

void BlockQueue::clear()
{
    AutoLock lock(&m_queueMutex);
    m_list.clear();
}

int BlockQueue::size()
{
    return m_list.count();
}
}
