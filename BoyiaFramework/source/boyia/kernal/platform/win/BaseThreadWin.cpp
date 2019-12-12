#include "AutoLock.h"
#include "BaseThread.h"

#if ENABLE(BOYIA_WINDOWS)
namespace yanbo {
class Condition {
public:
    Condition() {
        winEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    ~Condition() {
    }
    DWORD threadId;
    HANDLE thread;
    HANDLE winEvent;
};

BaseThread::BaseThread()
    : m_running(false)
    , m_condition(new Condition())
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
    if (!m_running) {
        return 0;
    }

    WaitForSingleObject(m_condition->thread, INFINITE);
}

void* BaseThread::startThread(void* ptr)
{
    BaseThread* thread = (BaseThread*)ptr;
    thread->m_running = true;
    thread->run();
    thread->m_running = false;
    return ptr;
}

void BaseThread::stop()
{
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
    WaitForSingleObject(m_condition->winEvent, timeout);
    ::ResetEvent(m_condition->winEvent);
}

// milliseconds
void BaseThread::sleepMS(long time)
{
    ::Sleep(time);
}

int BaseThread::getId()
{
    return m_condition->threadId;
}
}
#endif