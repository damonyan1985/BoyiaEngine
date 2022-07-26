#ifndef TaskThread_h
#define TaskThread_h

#include "BaseThread.h"
#include "BlockQueue.h"

namespace yanbo {

class TaskThread : public BaseThread, public BoyiaRef {
public:
    TaskThread(BlockQueue* queue, LBool isOwner = LFalse);

    virtual void stop();
    LBool working();
    virtual bool isAlive();
    LVoid sendTask(TaskBase* task);

protected:
    virtual void run();

private:
    ~TaskThread();
    
    BlockQueue* m_queue;
    LBool m_continue;
    LBool m_working;
    LBool m_isOwner;
};
}
#endif
