#ifndef KRefPtr_h
#define KRefPtr_h

#include "KRef.h"

namespace util
{
template<class T>
class KRefPtr
{
public:
	KRefPtr() : m_ptr(0) { }
	KRefPtr(T* ptr);
    KRefPtr(const KRefPtr<T>& ptr);
	~KRefPtr();

	T* get() const;

	KRefPtr<T>& operator=(const KRefPtr<T>& ptr);
	KRefPtr<T>& operator=(T* ptr);
    LBool operator==(T* ptr) const;
    LBool operator!=(T* ptr) const;
	T* operator->() const;

private:
	void refIfNotNull(T* ptr);
	void defIfNotNull(T* ptr);

private:
    T* m_ptr;
};

template<class T>
KRefPtr<T>::KRefPtr(T* ptr)
    : m_ptr(ptr)
{
	refIfNotNull(m_ptr);
}

template<class T>
KRefPtr<T>::KRefPtr(const KRefPtr<T>& ptr)
{
    T* dptr = ptr.get();
    // 新持有的指针进行加操作
    refIfNotNull(dptr);
    T* sptr = m_ptr;
    m_ptr = dptr;
    // 原来持有的指针进行减操作
    defIfNotNull(sptr);
}

template<class T>
KRefPtr<T>::~KRefPtr()
{
	defIfNotNull(m_ptr);
}

template<class T>
void KRefPtr<T>::refIfNotNull(T* ptr)
{
    if (ptr)
    {
        const_cast<KRef*>(static_cast<const KRef*>(ptr))->ref();
    }
}

template<class T>
void KRefPtr<T>::defIfNotNull(T* ptr)
{
    if (ptr)
    {
        const_cast<KRef*>(static_cast<const KRef*>(ptr))->deref();
    }
}

template<class T>
T* KRefPtr<T>::get() const
{
	return m_ptr;
}

template<class T>
KRefPtr<T>& KRefPtr<T>::operator=(const KRefPtr<T>& ptr)
{
	T* dptr = ptr.get();
	// 新持有的指针进行加操作
	refIfNotNull(dptr);
    T* sptr = m_ptr;
    m_ptr = dptr;
    // 原来持有的指针进行减操作
    defIfNotNull(sptr);
    return *this;
}

template<class T>
KRefPtr<T>& KRefPtr<T>::operator=(T* optr)
{
	// 新持有的指针进行加操作
    refIfNotNull(optr);
    T* ptr = m_ptr;
    m_ptr = optr;
    // 原来持有的指针进行减操作
    defIfNotNull(ptr);
    return *this;
}

template<class T>
T* KRefPtr<T>::operator->() const
{
    return m_ptr;
}

template<class T>
LBool KRefPtr<T>::operator==(T* ptr) const
{
    return m_ptr == ptr ? LTrue : LFalse;
}

template<class T>
LBool KRefPtr<T>::operator!=(T* ptr) const
{
    return m_ptr != ptr ? LTrue : LFalse;
}

}

using util::KRefPtr;
#endif
