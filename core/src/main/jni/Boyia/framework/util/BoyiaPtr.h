#ifndef BoyiaPtr_h
#define BoyiaPtr_h

#include "BoyiaRef.h"

namespace util
{
template<class T>
class BoyiaPtr
{
public:
	BoyiaPtr() : m_ptr(0) { }
	BoyiaPtr(T* ptr);
    BoyiaPtr(const BoyiaPtr<T>& ptr);
	~BoyiaPtr();

	T* get() const;

	BoyiaPtr<T>& operator=(const BoyiaPtr<T>& ptr);
	BoyiaPtr<T>& operator=(T* ptr);
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
BoyiaPtr<T>::BoyiaPtr(T* ptr)
    : m_ptr(ptr)
{
	refIfNotNull(m_ptr);
}

template<class T>
BoyiaPtr<T>::BoyiaPtr(const BoyiaPtr<T>& ptr)
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
BoyiaPtr<T>::~BoyiaPtr()
{
	defIfNotNull(m_ptr);
}

template<class T>
void BoyiaPtr<T>::refIfNotNull(T* ptr)
{
    if (ptr)
    {
        const_cast<BoyiaRef*>(static_cast<const BoyiaRef*>(ptr))->ref();
    }
}

template<class T>
void BoyiaPtr<T>::defIfNotNull(T* ptr)
{
    if (ptr)
    {
        const_cast<BoyiaRef*>(static_cast<const BoyiaRef*>(ptr))->deref();
    }
}

template<class T>
T* BoyiaPtr<T>::get() const
{
	return m_ptr;
}

template<class T>
BoyiaPtr<T>& BoyiaPtr<T>::operator=(const BoyiaPtr<T>& ptr)
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
BoyiaPtr<T>& BoyiaPtr<T>::operator=(T* optr)
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
T* BoyiaPtr<T>::operator->() const
{
    return m_ptr;
}

template<class T>
LBool BoyiaPtr<T>::operator==(T* ptr) const
{
    return m_ptr == ptr ? LTrue : LFalse;
}

template<class T>
LBool BoyiaPtr<T>::operator!=(T* ptr) const
{
    return m_ptr != ptr ? LTrue : LFalse;
}

}

using util::BoyiaPtr;


#endif
