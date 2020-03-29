/*
* FileName: MutexWin.cpp
* Author: yanbo
*/

#include "Mutex.h"

#if ENABLE(BOYIA_WINDOWS)
namespace yanbo {
Mutex::Mutex()
{
    InitializeCriticalSection(&m_lock);
}

Mutex::~Mutex()
{
    DeleteCriticalSection(&m_lock);
}

void Mutex::lock()
{
    EnterCriticalSection(&m_lock);
}

void Mutex::unlock()
{
    LeaveCriticalSection(&m_lock);
}

void Mutex::tryLock()
{
    TryEnterCriticalSection(&m_lock);
}

Lock* Mutex::getMutex()
{
    return &m_lock;
}
}

#endif