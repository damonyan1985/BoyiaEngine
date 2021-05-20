#ifndef BlockQueue_h
#define BlockQueue_h

#include "BoyiaPtr.h"
#include "KList.h"
#include "Lock.h"
#include "TaskBase.h"

namespace yanbo {

class BlockQueue : public BoyiaRef {
public:
    BlockQueue();
    ~BlockQueue();

    void addTask(TaskBase* task);
    BoyiaPtr<TaskBase> pollTask();
    void clear();
    int size();

private:
    KList<BoyiaPtr<TaskBase>> m_list;
    Lock m_queueMutex;
};
}

#endif
