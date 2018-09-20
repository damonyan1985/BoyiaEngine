/*
============================================================================
 Name        : KSet.h
 Author      : yanbo
 Version     : KSet v1.0
 Copyright   : All Copyright Reserved
 Description : support Original
============================================================================
*/
#ifndef KSet_h
#define KSet_h

#include "KList.h"

namespace util
{
template<class T>
class KSet
{
public:
	typedef KList<T> SetContainer;
	typedef typename SetContainer::Iterator Iterator;

public:
	KSet() {}
	~KSet() {}

	void add(const T& v)
	{
		typename SetContainer::Iterator iter = m_container.begin();
		typename SetContainer::Iterator iterEnd = m_container.end();

		bool find = false;
		for (; iter != iterEnd; ++iter)
		{
			if (*iter == v)
			{
				find = true;
				break;
			}
		}

		if (!find)
		{
			m_container.push(v);
		}
	}

	Iterator begin() const
	{
		m_container.begin();
	}

	Iterator end() const
	{
		m_container.end();
	}

private:
	SetContainer m_container;
};

}

using util::KSet;

#endif
