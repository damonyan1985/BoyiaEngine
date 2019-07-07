#ifndef MiniTaskThread_h
#define MiniTaskThread_h

#include "MiniBlockQueue.h"
#include "MiniThread.h"

namespace yanbo {

class MiniTaskThread : public MiniThread, public BoyiaRef {
public:
    MiniTaskThread(MiniBlockQueue* queue);
    ~MiniTaskThread();

    virtual void stop();
    LBool working();

protected:
    virtual void run();

private:
    MiniBlockQueue* m_queue;
    LBool m_continue;
    LBool m_working;
};
}
#endif
