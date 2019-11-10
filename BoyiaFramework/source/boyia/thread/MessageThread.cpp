#include "MessageThread.h"

namespace yanbo {

MessageThread::MessageThread()
    : m_queue(new MessageQueue())
    , m_continue(LTrue)
{
}

MessageThread::~MessageThread()
{
    delete m_queue;
}

LVoid MessageThread::postMessage(Message* msg)
{
    m_queue->push(msg);
    notify();
}

Message* MessageThread::obtain()
{
    return m_queue->obtain();
}

void MessageThread::run()
{
    while (m_continue) {
        if (!m_queue) {
            return;
        }

        Message* msg = m_queue->poll();
        if (msg) {
            handleMessage(msg);
            msg->msgRecycle();
        } else {
            BaseThread::waitOnNotify();
        }
    }
}
}
