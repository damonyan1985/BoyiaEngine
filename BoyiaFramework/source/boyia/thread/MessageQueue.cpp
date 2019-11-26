#include "MessageQueue.h"

namespace yanbo {

#define MAX_MESSAGE_SIZE 50
Message::Message()
    : type(0)
    , obj(kBoyiaNull)
    , recycle(LTrue)
    , inCache(LFalse)
{
}

LVoid Message::msgRecycle()
{
    if (inCache) {
        recycle = LTrue;
    } else {
        delete this;
    }
}

MessageCache::MessageCache()
    : m_cache(kBoyiaNull)
{
}

LVoid MessageCache::initCache()
{
    m_cache = new Message[MAX_MESSAGE_SIZE];
    LInt size = MAX_MESSAGE_SIZE;
    while (size--) {
        m_cache[size].inCache = LTrue;
    }
}

Message* MessageCache::obtain()
{
    if (!m_cache) {
        initCache();
    }

    LInt size = MAX_MESSAGE_SIZE;
    while (size--) {
        if (m_cache[size].recycle) {
            m_cache[size].recycle = LFalse;
            return m_cache + size;
        }
    }

    return new Message();
}

void MessageQueue::push(Message* msg)
{
    AutoLock lock(&m_queueMutex);
    m_list.push(msg);
}

LInt MessageQueue::size()
{
    return m_list.count();
}

Message* MessageQueue::poll()
{
    AutoLock lock(&m_queueMutex);
    Message* msg = kBoyiaNull;
    if (!m_list.empty()) {
        MessageList::Iterator iter = m_list.begin();
        msg = *iter;
        m_list.erase(iter);
    }

    return msg;
}

Message* MessageQueue::obtain()
{
    AutoLock lock(&m_queueMutex);
    return MessageCache::obtain();
}

LBool MessageQueue::hasMessage(LInt type)
{
    AutoLock lock(&m_queueMutex);
    MessageList::Iterator iter = m_list.begin();
    MessageList::Iterator iterEnd = m_list.end();
    while (iter != iterEnd) {
        Message* msg = *iter;
        if (msg->type == type) {
            return LTrue;
        }

        ++iter;
    }

    return LFalse;
}

// 删除所有type一样的消息
LVoid MessageQueue::removeMessage(LInt type)
{
    AutoLock lock(&m_queueMutex);
    MessageList::Iterator iter = m_list.begin();
    MessageList::Iterator iterEnd = m_list.end();
    while (iter != iterEnd) {
        Message* msg = *iter;
        if (msg->type == type) {
            MessageList::Iterator tmpIter = iter++;
            m_list.erase(tmpIter);

            msg->msgRecycle();
        } else {
            ++iter;
        }
    }
}
}
