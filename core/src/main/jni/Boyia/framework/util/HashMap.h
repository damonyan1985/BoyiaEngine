#ifndef HashMap_h
#define HashMap_h

// 
// Author: Yanbo
// Create: 2019-3-9
// All Copyright reserved
//
#include "PlatformLib.h"

namespace util
{
const LInt HASH_TABLE_DEFAULT_SIZE = 1 << 4;
const LReal HASH_TABLE_DEFAULT_FACTOR = 0.75f;

template <typename K, typename V>
class MapEntry
{
public:
	MapEntry()
		: next(NULL)
	{		
	}
	MapEntry(K k, V v)
		: next(NULL)
		, key(k)
		, value(v)
	{		
	}

	K key;
	V value;
	MapEntry* next;
};

template <typename K, typename V>
class HashMap
{
public:
	typedef MapEntry<K, V>* HashMapEntryPtr;
	typedef MapEntry<K, V> HashMapEntry;

public:
	HashMap()
		: m_capacity(HASH_TABLE_DEFAULT_SIZE)
		, m_size(0)
		, m_table(new HashMapEntryPtr[m_capacity])
		, m_threshold(HASH_TABLE_DEFAULT_SIZE * HASH_TABLE_DEFAULT_FACTOR)
	{
		LMemset(m_table, 0, m_capacity * sizeof(HashMapEntryPtr));
	}

	LVoid put(K key, V val)
	{
		LInt index = indexHash(key);
		// 如果entry存在，则判断是否有相同的Key
		HashMapEntryPtr entry = m_table[index];
		for (; entry; entry->next)
        {
        	if (entry->key == key)
        	{
        		entry->value = val;
        		return;
        	}
        }
		
        // 如果超过阈值
        if (++m_size > m_threshold)
        {
			resize();
			addEntry(indexHash(key), key, val);
        }
        else
        {
        	addEntry(index, key, val);
        }
	}
	
	V get(K ket)
	{
		HashMapEntryPtr entry = m_table[indexHash(key)];
		for (; entry; entry->next)
		{
			if (entry->key == key)
			{
				return entry->value;
			}
		}

		return (V)NULL;
	}

	LInt size() const
	{
		return m_size;
	}

	LVoid resize()
	{
		// 双倍扩容
		LInt oldCapacity = m_capacity;

		m_capacity *= 2;
		MapEntry<K, V>* table = new MapEntry<K, V>[oldCapacity];

		// 拷贝原table数据到新table中
		for (LInt i = 0; i < oldCapacity; ++i)
		{
			MapEntry<K, V>* entry = &m_table[i];
			for (; entry; entry->next)
			{
				addEntry(indexHash(entry->key), entry);
			}
		}

		m_threshold = (LInt) m_capacity * HASH_TABLE_DEFAULT_FACTOR;
		
		delete m_table;
		m_table = table;
	}

private:
	LInt indexHash(K key)
	{
		LUint hash = (hash = key.hash()) ^ (hash >> 16);
		return hash & m_capacity;
	}

	LVoid addEntry(LInt index, HashMapEntryPtr ptr)
	{
		// 如果链表头部为空，则创建头部
    	if (!m_table[index])
    	{
			m_table[index] = ptr;
    	}
    	else // 如果头部不为空，则将entry插在最前面
    	{
    		HashMapEntryPtr ptr = ptr;
        	ptr->next = m_table[index];
        	m_table[index] = ptr;
    	}
	}

	LVoid addEntry(LInt index, T key, V value)
	{
		addEntry(index, new HashMapEntry(key, value));
	}

	HashMapEntryPtr* m_table;
	LInt m_size;
	LInt m_capacity;
	LInt m_threshold;
};
}
#endif