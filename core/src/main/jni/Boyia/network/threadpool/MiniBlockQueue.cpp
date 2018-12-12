/*
 * MiniMutex.cpp
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#include "MiniBlockQueue.h"
#include "AutoLock.h"

namespace yanbo
{
MiniBlockQueue::MiniBlockQueue()
{
}

MiniBlockQueue::~MiniBlockQueue()
{
	clear();
}

void MiniBlockQueue::addTask(MiniTaskBase* task)
{
	AutoLock lock(&m_queueMutex);
	m_list.push(task);
}

KRefPtr<MiniTaskBase> MiniBlockQueue::pollTask()
{
	AutoLock lock(&m_queueMutex);
	KRefPtr<MiniTaskBase> task = NULL;
	if (!m_list.empty())
	{
		KList<KRefPtr<MiniTaskBase> >::Iterator iter = m_list.begin();
		task = *iter;
		m_list.erase(iter);
	}

	return task;
}

void MiniBlockQueue::clear()
{
	AutoLock lock(&m_queueMutex);
	m_list.clear();
}

int MiniBlockQueue::size()
{
	return m_list.count();
}
}
