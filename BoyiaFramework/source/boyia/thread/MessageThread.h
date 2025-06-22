#ifndef MessageThread_h
#define MessageThread_h

#include "BaseThread.h"
#include "MessageQueue.h"

namespace yanbo {
class MessageThread;
class MessageLoop {
public:
    MessageLoop(MessageThread* thread);

    virtual ~MessageLoop() {};
    virtual LVoid loop() = 0;
    virtual LVoid postMessage(Message* msg) = 0;
    virtual Message* obtain() = 0;
    virtual LVoid quit() = 0;
    virtual MessageQueue* queue() = 0;
    virtual LBool isAlive() = 0;

protected:
    LVoid handleMessage(Message* msg);
    MessageThread* m_thread;
};

class MessageThread : public BaseThread {
public:
    MessageThread();
    virtual ~MessageThread();

    LVoid postMessage(Message* msg);
    Message* obtain();
    LVoid quit();
    virtual bool isAlive();

protected:
    virtual LVoid handleMessage(Message* msg) = 0;
    virtual LVoid run() LFinal;
    
    MessageLoop* m_loop;

    friend class MessageLoop;
};
}

#endif
