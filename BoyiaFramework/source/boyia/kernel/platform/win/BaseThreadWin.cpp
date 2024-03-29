#include "AutoLock.h"
#include "BaseThread.h"

#if ENABLE(BOYIA_WINDOWS)
namespace yanbo {
class Condition {
public:
    Condition()
        : thread(0)
        , winEvent(0)
        , threadId(0)
    {
        winEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    ~Condition() 
    {
        if (winEvent) {
            ::CloseHandle(winEvent);
        }

        if (thread) {
            ::CloseHandle(thread);
        }
    }
    DWORD threadId;
    HANDLE thread;
    HANDLE winEvent;
};

BaseThread::BaseThread()
    : m_condition(new Condition())
{
}

BaseThread::~BaseThread()
{
    delete m_condition;
}

void BaseThread::start()
{
    m_condition->thread = CreateThread(kBoyiaNull, 0, (LPTHREAD_START_ROUTINE)startThread, this, 0, &m_condition->threadId);
}

int BaseThread::wait()
{
    if (!isAlive()) {
        return 0;
    }

    return WaitForSingleObject(m_condition->thread, INFINITE);
}

void* BaseThread::startThread(void* ptr)
{
    BaseThread* thread = (BaseThread*)ptr;
    thread->run();
    return ptr;
}

void BaseThread::waitOnNotify()
{
    WaitForSingleObject(m_condition->winEvent, INFINITE);
    ::ResetEvent(m_condition->winEvent);
    //SuspendThread(m_condition->thread);
}

void BaseThread::notify()
{
    //if (m_condition->thread) {
    //    ResumeThread(m_condition->thread);
    //}
    ::SetEvent(m_condition->winEvent);
}

// millisecond
void BaseThread::waitTimeOut(long timeout)
{
    ::ResetEvent(m_condition->winEvent);
    WaitForSingleObject(m_condition->winEvent, timeout);
    ::ResetEvent(m_condition->winEvent);
}

// milliseconds
void BaseThread::sleepMS(long time)
{
    ::Sleep(time);
}

LUint64 BaseThread::getId()
{
    return (LUint64)m_condition->threadId;
}

void BaseThread::stop()
{
    ::TerminateThread(m_condition->thread, NULL);
}

}
#endif