#ifndef MessageThread_h
#define MessageThread_h

#include "BaseThread.h"
#include "MessageQueue.h"

namespace yanbo {

class MessageLoop {
public:
    virtual ~MessageLoop() {};
    virtual LVoid loop() = 0;
    virtual LVoid postMessage(Message* msg) = 0;
    virtual Message* obtain() = 0;
    virtual LVoid quit() = 0;
    virtual MessageQueue* queue() = 0;
    virtual LBool isAlive() = 0;
};

class MessageThread : public BaseThread {
public:
    MessageThread();
    virtual ~MessageThread();

    virtual LVoid handleMessage(Message* msg) = 0;
    LVoid postMessage(Message* msg);
    Message* obtain();
    LVoid quit();
    virtual bool isAlive();

protected:
    virtual LVoid run() LFinal;
    
    //MessageQueue* m_queue;
    //LBool m_continue;
    MessageLoop* m_loop;
};
}

#endif
