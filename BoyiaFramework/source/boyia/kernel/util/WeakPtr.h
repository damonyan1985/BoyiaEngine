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
        if (m_ptr) {
            m_refCount = static_cast<const BoyiaRef*>(m_ptr)->count();
            m_refCount->attachWeak();
        }
    }

    // if the pointer is expired
    LBool expired()
    {
        return !m_refCount->shareCount();
    }

    // if ptr expired, reture null pointer
    T* get() const 
    {
        return expired() ? kBoyiaNull : m_ptr;
    }

    ~WeakPtr()
    {
        if (m_refCount) {
            // detch weak count
            m_refCount->detchWeak();
            // release reference count
            m_refCount->release();
        }
    }

private:
    RefCount* m_refCount;
    T* m_ptr;
};
}

#endif