#ifndef MessageThread_h
#define MessageThread_h

#include "BaseThread.h"
#include "MessageQueue.h"

namespace yanbo {

class MessageThread : public BaseThread {
public:
    MessageThread();
    virtual ~MessageThread();

    virtual LVoid handleMessage(Message* msg) = 0;
    LVoid postMessage(Message* msg);
    Message* obtain();

protected:
    virtual void run() LFinal;
    MessageQueue* m_queue;
    LBool m_continue;
};
}

#endif
