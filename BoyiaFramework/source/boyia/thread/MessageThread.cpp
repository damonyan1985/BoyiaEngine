#include "MessageThread.h"

namespace yanbo {
class CommonMessageLoop : public MessageLoop {
public:
    CommonMessageLoop(MessageThread* thread)
        : m_thread(thread)
        , m_continue(LTrue)
        , m_queue(new MessageQueue()){}
    
    virtual LVoid loop() LOverride
    {
        while (m_continue) {
            if (!m_queue) {
                return;
            }

            Message* msg = m_queue->poll();
            if (msg) {
                m_thread->handleMessage(msg);
                msg->msgRecycle();
            } else {
                m_thread->waitOnNotify();
            }
        }
    }
    
    Message* obtain() LOverride
    {
        return m_queue->obtain();
    }
    
    LVoid postMessage(Message* msg) LOverride
    {
        m_queue->push(msg);
        m_thread->notify();
    }
    
    LVoid quit() LOverride
    {
        m_continue = LFalse;
    }
    
    MessageQueue* queue() LOverride
    {
        return m_queue;
    }
    
    ~CommonMessageLoop()
    {
        quit();
        delete m_queue;
    }
    
    LBool isAlive() LOverride
    {
        return m_continue;
    }
    
private:
    MessageQueue* m_queue;
    MessageThread* m_thread;
    LBool m_continue;
};

MessageThread::MessageThread()
    : m_loop(new CommonMessageLoop(this))
{
}

MessageThread::~MessageThread()
{
    delete m_loop;
}

LVoid MessageThread::postMessage(Message* msg)
{
    m_loop->postMessage(msg);
}

Message* MessageThread::obtain()
{
    return m_loop->obtain();
}

LVoid MessageThread::run()
{
    m_loop->loop();
}

LVoid MessageThread::quit()
{
    m_loop->quit();
}

bool MessageThread::isAlive()
{
    return m_loop->isAlive();
}
}
