#include "MessageQueue.h"

namespace yanbo {

const LInt kMessageCapacity = 50;
Message::Message()
    : type(0)
    , obj(kBoyiaNull)
    , inCache(LTrue)
{
}

MessageCache::MessageCache()
    : m_cache(kBoyiaNull)
    , m_useIndex(0)
{
}

LVoid MessageCache::initCache()
{
    m_cache = new Message[kMessageCapacity];
    m_freeList = &m_cache[0];
    {
        m_freeList->next = kBoyiaNull;
    }
}

Message* MessageCache::obtain()
{
    if (!m_cache) {
        initCache();
    }

    Message* msg = m_freeList;
    if (m_freeList && m_freeList->next) {
        m_freeList = m_freeList->next;
    } else {
        // 如果free用完了，而且m_useIndex已经达到最大容量了, 将freelist头部设置为null
        if (m_useIndex >= kMessageCapacity - 1) {
            if (m_freeList) {
                m_freeList = kBoyiaNull;
            }
            if (!msg) {
                msg = new Message();
                msg->inCache = LFalse;
            }
        } else {
            // 如果m_useIndex还没用完, 继续分配
            m_freeList = &m_cache[++m_useIndex];
            m_freeList->next = kBoyiaNull;
        }
        
    }

    return msg;
}

LVoid MessageCache::freeMessage(Message* msg)
{
    if (!msg->inCache) {
        delete msg;
        return;
    }
    
    if (m_freeList) {
        msg->next = m_freeList->next;
    } else {
        msg->next = kBoyiaNull;
    }
    
    m_freeList = msg;
}

LVoid MessageQueue::push(Message* msg)
{
    AutoLock lock(&m_queueLock);
    m_list.push(msg);
}

LInt MessageQueue::size()
{
    return m_list.count();
}

Message* MessageQueue::poll()
{
    AutoLock lock(&m_queueLock);
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
    AutoLock lock(&m_queueLock);
    return MessageCache::obtain();
}

LVoid MessageQueue::freeMessage(Message* msg)
{
    AutoLock lock(&m_queueLock);
    return MessageCache::freeMessage(msg);
}

LBool MessageQueue::hasMessage(LInt type)
{
    AutoLock lock(&m_queueLock);
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
    AutoLock lock(&m_queueLock);
    MessageList::Iterator iter = m_list.begin();
    MessageList::Iterator iterEnd = m_list.end();
    while (iter != iterEnd) {
        Message* msg = *iter;
        if (msg->type == type) {
            MessageList::Iterator tmpIter = iter++;
            m_list.erase(tmpIter);

            freeMessage(msg);
        } else {
            ++iter;
        }
    }
}
}
