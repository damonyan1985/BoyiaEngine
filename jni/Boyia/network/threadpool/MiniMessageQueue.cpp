#include "MiniMessageQueue.h"

namespace yanbo
{
#define MAX_MESSAGE_SIZE 50
MiniMessage::MiniMessage()
    : type(0)
    , obj(NULL)
    , recycle(LTrue)
    , inCache(LFalse)
{
}

LVoid MiniMessage::msgRecycle()
{
	if (inCache)
	{
		recycle = LTrue;
	}
	else
	{
		delete this;
	}
}

MiniMessageCache::MiniMessageCache()
    : m_cache(NULL)
{
}

LVoid MiniMessageCache::initCache()
{
	m_cache = new MiniMessage[MAX_MESSAGE_SIZE];
	LInt size = MAX_MESSAGE_SIZE;
	while (size--)
	{
		m_cache[size].inCache = LTrue;
	}
}

MiniMessage* MiniMessageCache::obtain()
{
	if (m_cache == NULL)
	{
		initCache();
	}

    LInt size = MAX_MESSAGE_SIZE;
    while (size--)
    {
        if (m_cache[size].recycle)
        {
        	m_cache[size].recycle = LFalse;
        	return m_cache + size;
        }
    }

    return new MiniMessage();
}

void MiniMessageQueue::push(MiniMessage* msg)
{
	AutoLock lock(&m_queueMutex);
	m_list.push(msg);
}

LInt MiniMessageQueue::size()
{
	return m_list.count();
}

MiniMessage* MiniMessageQueue::poll()
{
	AutoLock lock(&m_queueMutex);
	MiniMessage* msg = NULL;
	if (!m_list.empty())
	{
		MiniMessageList::Iterator iter = m_list.begin();
		msg = *iter;
		m_list.erase(iter);
	}

	return msg;
}

MiniMessage* MiniMessageQueue::obtain()
{
	AutoLock lock(&m_queueMutex);
	return MiniMessageCache::obtain();
}

// 删除所有type一样的消息
LVoid MiniMessageQueue::removeMessage(LInt type)
{
	AutoLock lock(&m_queueMutex);
	MiniMessageList::Iterator iter = m_list.begin();
	MiniMessageList::Iterator iterEnd = m_list.end();
	while (iter != iterEnd)
	{
		MiniMessage* msg = *iter;
		if (msg->type == type)
		{
			MiniMessageList::Iterator tmpIter = iter++;
			m_list.erase(tmpIter);

			msg->msgRecycle();
		}
		else
		{
			++iter;
		}
	}
}

}
