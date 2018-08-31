/*
============================================================================
 Name        : KVector.h
 Author      : yanbo
 Version     : KVector v1.0
 Copyright   : All Copyright Reserved
 Description : KVector support Original
 Description : KVector
 Modified    : 2015-3-03
============================================================================
*/

#ifndef KVector_h
#define KVector_h

#include "PlatformLib.h"
#include "KRef.h"

namespace util
{

template <class T>
class KVector : public KRef
{
public:
	KVector();
	KVector(LInt);
	KVector(LInt, LInt);
	KVector(const KVector<T>& kv, Bool deep = LTrue);// kvector
    virtual ~KVector();

public:
	LVoid addElement(const T& kv);// kvalue
	const T& elementAt(LInt position) const;
	const T& lastElement() const;
	T& elementAt(LInt position);
	T& lastElement();
	LInt size() const { return m_size; }
	LInt capacity() const { return m_capacity; }
	T& operator[] (LInt position);
	const T& operator[](LInt position) const;
	void remove(LInt position);
	const T* getBuffer() const;
	KVector<T>& operator=(const KVector<T>& kv);
	LBool contains(const T& kv);
	void removeAllElements();
	void clear();

private:
	LVoid copyElements(const KVector<T>& kv, LBool deep = LTrue);
	LVoid copyElements(const T* buffer, LInt len);

private:
	LInt m_size;
    T*   m_buffer;
	LInt m_capacity;
	Bool m_deep;
};

template <class T>
KVector<T>::KVector()
    : m_size(0)
    , m_buffer(NULL)
    , m_capacity(0)
    , m_deep(LTrue)
{
}

template <class T>
KVector<T>::KVector(LInt capacity)
    : m_size(capacity)
    , m_capacity(capacity)
    , m_deep(LTrue)
{
	m_buffer = new T[capacity];
}

template <class T>
KVector<T>::KVector(LInt size, LInt capacity)
	: m_size(size)
	, m_capacity(capacity)
	, m_deep(LTrue)
{
	m_buffer = new T[capacity];
}

template <class T>
KVector<T>::~KVector()
{
	if (m_deep)
	{
	    removeAllElements();
	}
}

template <class T> // 深拷贝
KVector<T>::KVector(const KVector<T>& kv, Bool deep)
{
	m_deep = deep;
	if (deep == LTrue)
	{
		copyElements(kv);
	}
	else
	{
        m_buffer   = const_cast<T*>(kv.getBuffer());
        m_size     = kv.size();
        m_capacity = kv.capacity();
	}
}

template <class T>
const T* KVector<T>::getBuffer() const
{
    return m_buffer;
}

template <class T>
LVoid KVector<T>::copyElements(const KVector<T>& kv, LBool deep)
{
	if (deep)
	{
		copyElements(kv.getBuffer(), kv.size());
	}
	else
	{
        m_buffer   = const_cast<T*>(kv.getBuffer());
        m_size     = kv.size();
        m_capacity = kv.capacity();
	}
}

template <class T>
LVoid KVector<T>::copyElements(const T* buffer, LInt len)
{
	LInt size = len;
	T* tmp = m_buffer;

	m_buffer = new T[size];
	
	LInt tmpSize = m_size;
	m_capacity = size;
	m_size = size;
	
    for (LInt i=0; i < tmpSize; i++)
    {
        *(m_buffer+i) = *(buffer+i);
    }
    
	if (m_buffer)
	{
		delete []tmp;
		tmp = NULL;
	}
}

template <class T>
const T& KVector<T>::elementAt(LInt i) const
{
    return *(m_buffer+i);
}

template <class T>
const T& KVector<T>::lastElement() const
{
    return *(m_buffer+m_size);
}

template <class T>
T& KVector<T>::elementAt(LInt position)
{
    return *(m_buffer+position);
}

template <class T>
T& KVector<T>::lastElement()
{
    return *(m_buffer+m_size-1);
}

template <class T>
LVoid KVector<T>::addElement(const T& kv)
{
    if (m_size < m_capacity)
	{
		*(m_buffer+m_size) = kv;
        m_size++;
	}
    else
	{
        copyElements(m_buffer, m_size+1);
		*(m_buffer+m_size-1) = kv;
	}
}

template <class T>
T& KVector<T>::operator[](LInt position)
{
    return elementAt(position);
}

template <class T>
const T& KVector<T>::operator[](LInt position) const
{
	return elementAt(position);
}

template <class T>
KVector<T>& KVector<T>::operator=(const KVector<T>& kv)
{
    copyElements(kv.getBuffer(), kv.size());
	return *this;
}

template <class T>
void KVector<T>::remove(LInt position)
{
	if (position < 0 || position >= m_size)
	{
	    return;
	}
	
	for (LInt i = position; i < m_size-1; i++)
	{
	    *(m_buffer+i) = *(m_buffer+i+1);
	}
	
	m_size--;
}

template <class T>
LBool KVector<T>::contains(const T& kv)
{
	for (LInt i = 0; i < m_size; i++)
	{
	    if (m_buffer[i] == kv)
	    {
			return LTrue;
	    }
	}
	
	return LFalse;
}

template <class T>
void KVector<T>::removeAllElements()
{
    if (m_buffer)
    {
        delete []m_buffer;
        m_buffer = NULL;
        
        m_size = 0;
        m_capacity = 0;
    }
}

template <class T>
void KVector<T>::clear()
{
    m_size = 0;
}

}

using util::KVector;

#endif
