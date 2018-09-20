/*
 * Stack.h
 *
 *  Created on: 2011-6-29
 *      Author: yanbo
 */

#ifndef Stack_h
#define Stack_h

#include "KVector.h"
namespace util
{
template <class T>
class Stack
{
public:
	Stack();
	Stack(LInt);
	virtual ~Stack();

public:
	LInt size();
	T pop();
	void push(const T& obj);
	T& elementAt(LInt i);

protected:
	KVector<T> m_vector;
};

template <class T>
Stack<T>::Stack()
{
}

template <class T>
Stack<T>::~Stack()
{
}

template <class T>
Stack<T>::Stack(LInt size)
	: m_vector(0, size)
{
}

template <class T>
LInt Stack<T>::size()
{
	return m_vector.size();
}

template<class T>
T Stack<T>::pop()
{
	T elem = m_vector.lastElement();
	m_vector.remove(m_vector.size()-1);
	return elem;
}

template <class T>
void Stack<T>::push(const T& obj)
{
	m_vector.addElement(obj);
}

template <class T>
T& Stack<T>::elementAt(LInt i)
{
	return m_vector.elementAt(i);
}

}

using util::Stack;
#endif /* STACK_H_ */
