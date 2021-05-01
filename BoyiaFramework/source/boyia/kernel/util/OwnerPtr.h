/**
 * Author Yanbo
 * All Copyright reserved 
 * 2019-9-8
 */

#ifndef OwnerPtr_h
#define OwnerPtr_h

namespace util {

template <typename T>
class OwnerPtr {
public:
    OwnerPtr(T* ptr = kBoyiaNull)
        : m_ptr(ptr)
    {
    }

    OwnerPtr(const OwnerPtr<T>& ownerPtr)
        : m_ptr(ownerPtr.m_ptr)
    {
        ownerPtr.m_ptr = kBoyiaNull;
    }

    bool operator==(T* ptr) const
    {
        return m_ptr == ptr;
    }

    bool operator!=(T* ptr) const
    {
        return m_ptr != ptr;
    }

    OwnerPtr<T>& operator=(const OwnerPtr<T>& ownerPtr)
    {
        // 释放当前指针
        release();
        // 重设指针
        m_ptr = ownerPtr.m_ptr;
        // 外部指针归0
        ownerPtr.m_ptr = kBoyiaNull;

        return *this;
    }

    OwnerPtr<T>& operator=(T* ptr)
    {
        release();
        m_ptr = ptr;

        return *this;
    }

    ~OwnerPtr()
    {
        if (m_ptr) {
            delete m_ptr;
        }
    }

    T* get() const
    {
        return m_ptr;
    }

    T* operator->() const
    {
        return m_ptr;
    }

private:
    void release()
    {
        if (m_ptr) {
            delete m_ptr;
        }
    }

    mutable T* m_ptr;
};
}

using util::OwnerPtr;

#endif
