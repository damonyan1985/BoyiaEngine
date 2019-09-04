#ifndef TaskThread_h
#define TaskThread_h

#include "BaseThread.h"
#include "BlockQueue.h"

namespace yanbo {

class TaskThread : public BaseThread, public BoyiaRef {
public:
    TaskThread(BlockQueue* queue);
    ~TaskThread();

    virtual void stop();
    LBool working();

protected:
    virtual void run();

private:
    BlockQueue* m_queue;
    LBool m_continue;
    LBool m_working;
};
}
#endif
