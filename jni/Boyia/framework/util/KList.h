/*
============================================================================
 Name        : klist.h
 Author      : yanbo
 Version     : klist v2.0
 Copyright   : All Copyright Reserved
 Description : klist
 Modified    : 2018-9-1
============================================================================
*/

#ifndef KList_h
#define KList_h

#include "PlatformLib.h"
#include "SalLog.h"

namespace util
{
template<class NodeValue>
class ListNode
{
public:
	NodeValue m_value;
	ListNode<NodeValue>* m_next;
	ListNode<NodeValue>* m_prev;

	ListNode()
	{
		m_next = NULL;
		m_prev = NULL;
	}

	~ListNode()
	{
		m_next = NULL;
		m_prev = NULL;
	}
};

template<class NodeValue>
class ListIterator
{
public:
	typedef ListNode<NodeValue>* NodePtr;

public:
	ListIterator()
		: m_nodePtr(NULL)
	{
	}

	ListIterator(const NodePtr& nodePtr)
		: m_nodePtr(nodePtr)
	{
	}

	ListIterator(const ListIterator<NodeValue>& iter)
	{
		m_nodePtr = iter.m_nodePtr;
	}
	~ListIterator(){}

public:
	NodeValue& operator * ()
	{
		return m_nodePtr->m_value;
	}

	ListIterator<NodeValue>& operator = (ListIterator<NodeValue>& iter)
	{
		m_nodePtr = iter.m_nodePtr;
		return *this;
	}

	ListIterator<NodeValue>& operator = (const ListIterator<NodeValue>& iter)
	{
		m_nodePtr = iter.m_nodePtr;
		return *this;
	}

	ListIterator<NodeValue>& operator ++ ()
	{
		m_nodePtr = m_nodePtr->m_next;
		return *this;
	}

	ListIterator<NodeValue>& operator -- ()
	{
		m_nodePtr = m_nodePtr->m_prev;
		return *this;
	}

	ListIterator<NodeValue> operator ++ (int)
	{
		NodePtr tmp = m_nodePtr;
		m_nodePtr = m_nodePtr->m_next;
		return ListIterator<NodeValue>(tmp);
	}

	ListIterator<NodeValue> operator -- (int)
	{
		NodePtr tmp = m_nodePtr;
		m_nodePtr = m_nodePtr->m_prev;
		return ListIterator<NodeValue>(tmp);
	}

	LBool operator == (const ListIterator<NodeValue>& iter)
	{
		return m_nodePtr == iter.m_nodePtr ? LTrue : LFalse;
	}

	LBool operator != (const ListIterator<NodeValue>& iter)
	{
		return m_nodePtr == iter.m_nodePtr ? LFalse : LTrue;
	}

public:
	NodePtr& operator & () { return m_nodePtr; }

protected:
	NodePtr m_nodePtr;
};

template<class NodeValue>
class KList
{
public:
	typedef ListNode<NodeValue>* NodePtr;
	typedef ListIterator<NodeValue> Iterator;

public:
	KList()
	{
		m_header = new ListNode<NodeValue>;
		m_header->m_prev = m_header;
		m_header->m_next = m_header;

		m_count = 0;
	}

	KList(const KList<NodeValue>& oldKlist)
	{
		m_header = new ListNode<NodeValue>;
		m_header->m_prev = m_header;
		m_header->m_next = m_header;

		Iterator it = oldKlist.begin();
		Iterator itEnd = oldKlist.end();
		for (; it != itEnd; ++it)
		{
			push(*it);
		}

		m_count = oldKlist.count();
	}

	~KList()
	{
		clear();
		if (m_header != NULL)
		{
			delete m_header;
			m_header = NULL;
		}
	}

	const Iterator begin() const
	{
		return Iterator(m_header->m_next);
	}

	Iterator begin()
	{
		return Iterator(m_header->m_next);
	}

	const Iterator end() const
	{
		return Iterator(m_header);
	}

	Iterator end()
	{
		return Iterator(m_header);
	}

	Iterator push(const NodeValue& value)
	{
		NodePtr nodePtr = new ListNode<NodeValue>;
		nodePtr->m_value = value;

		nodePtr->m_next = m_header;
		nodePtr->m_prev = m_header->m_prev;
		m_header->m_prev->m_next = nodePtr;
		m_header->m_prev = nodePtr;

		++m_count;
		return Iterator(nodePtr);
	}

	LVoid erase(Iterator& iter)
	{
		NodePtr prevPtr = (&iter)->m_prev;
		NodePtr nextPtr = (&iter)->m_next;


		Iterator iterEnd = end();
		if (iter != iterEnd)
		{
			prevPtr->m_next = nextPtr;
			nextPtr->m_prev = prevPtr;

			--m_count;
		}
		else
		{
			return;
		}

		delete &iter;
		&iter = NULL;
	}

	Iterator insert(Iterator& iter, const NodeValue& value)
	{
		NodePtr nodePtr = new ListNode<NodeValue>;
		nodePtr->m_value = value;
		nodePtr->m_next = NULL;
		nodePtr->m_prev = NULL;

		if (m_header->m_next == m_header)
		{
			m_header->m_next = nodePtr;
			m_header->m_next->m_prev = m_header;
			m_header->m_next->m_next = m_header;
			m_header->m_prev = m_header->m_next;
		}
		else
		{
			nodePtr->m_next = (&iter);
			nodePtr->m_prev = (&iter)->m_prev;
			(&iter)->m_prev->m_next = nodePtr;
			(&iter)->m_prev = nodePtr;
		}

		++m_count;
		return Iterator(nodePtr);
	}

	LVoid clear()
	{
		if (!m_count)
		{
			return;
		}

		KLOG("klist::clear()");
		Iterator iter = begin();
		Iterator iterEnd = end();

		KLOG("klist::clear()1");
		Iterator iterNext = iter;
		for (; iter != iterEnd;)
		{
			KLOG("klist::clear()2");
			iterNext++;

			erase(iter);
			KLOG("klist::clear()3");
			iter = iterNext;
		}
		KLOG("klist::clear()4");
		m_count = 0;
	}

	LBool empty()
	{
		return m_count == 0 ? LTrue : LFalse;
	}

	NodeValue operator [] (LInt aIndex)
	{
		Iterator iter = begin();
		for (LInt i = 0 ; i < aIndex; i++)
		{
			iter++;
		}

		return *iter;
	}

	LInt count() const
	{
		return m_count;
	}

protected:
	NodePtr m_header;
	LInt m_count;
};

}

using util::KList;

#endif
