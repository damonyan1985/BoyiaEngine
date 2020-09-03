#ifndef WeakPtr_h
#define WeakPtr_h

#include "BoyiaPtr.h"

namespace util{
template<typename T>
class WeakPtr{
public:
    WeakPtr(BoyiaPtr<T> ptr)
    {
        m_ptr = ptr.get();
        m_refCount = static_cast<const BoyiaRef*>(ptr.get())->count();
        m_refCount->attachWeak();
    }

    // if the pointer is expired
    LBool expired()
    {
        return !m_refCount->shareCount();
    }

    T* get() const 
    {
        return m_ptr;
    }

    ~WeakPtr()
    {
        if (m_refCount) {
            m_refCount->detchWeak();
            m_refCount->release();
        }
    }

private:
    RefCount* m_refCount;
    T* m_ptr;
};
}

#endif
