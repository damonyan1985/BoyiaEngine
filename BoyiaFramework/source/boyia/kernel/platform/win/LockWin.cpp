/*
* FileName: MutexWin.cpp
* Author: yanbo
*/

#include "Lock.h"

#if ENABLE(BOYIA_WINDOWS)
namespace yanbo {
Lock::Lock()
{
    InitializeCriticalSection(&m_lock);
}

Lock::~Lock()
{
    DeleteCriticalSection(&m_lock);
}

void Lock::lock()
{
    EnterCriticalSection(&m_lock);
}

void Lock::unlock()
{
    LeaveCriticalSection(&m_lock);
}

void Lock::tryLock()
{
    TryEnterCriticalSection(&m_lock);
}

PlatformLock* Lock::getLock()
{
    return &m_lock;
}
}

#endif