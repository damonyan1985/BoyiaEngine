#ifndef ThreadPool_h
#define ThreadPool_h

#include "KList.h"
#include "TaskBase.h"
#include "TaskThread.h"

namespace yanbo {

class ThreadPool {
public:
    static ThreadPool* getInstance();
    static void destroy();

    void sendTask(TaskBase* task);

private:
    ~ThreadPool();
    ThreadPool();

    KList<TaskThread*> m_threadList;
    BoyiaPtr<BlockQueue> m_queue;
};
}

#endif
