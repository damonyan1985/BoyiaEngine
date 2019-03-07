#ifndef HashMap_h
#define HashMap_h

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
class EntryList
{
public:
	EntryList()
		: head(NULL)
		, tail(NULL)
	{
	}

	MapEntry<K, V>* head;
	MapEntry<K, V>* tail;
};

template <typename K, typename V>
class HashMap
{
public:
	HashMap()
		: m_size(HASH_TABLE_DEFAULT_SIZE)
		, m_table(new EntryList<K, V>[m_size])
		, m_threshold(HASH_TABLE_DEFAULT_SIZE * HASH_TABLE_DEFAULT_FACTOR)
	{
	}

	LVoid put(K key, V val)
	{
		LInt index = indexHash(key);
		if (m_table[index].head)
		{
			m_table[index].tail->next = new MapEntry<K, V>(key, val);
			m_table[index].tail = m_table[index].tail->next;
		}
		else
		{
			m_table[index].head = new MapEntry<K, V>(key, val);
			m_table[index].tail = m_table[index].head;
		}
	}
	
	V get(K ket)
	{
		EntryList* list = &m_table[indexHash(key)];
		MapEntry* entry = list->head;
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

private:
	LInt indexHash(K key)
	{
		LUint hash = (hash = key.hash()) ^ (hash >> 16);
		return hash;
	}

	EntryList* m_table;
	LInt m_size;
	LInt m_threshold;
};
}
#endif