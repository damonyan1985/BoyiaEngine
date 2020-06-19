//
// Author: Yanbo
// Create: 2019-3-9
// All Copyright reserved
//

#ifndef HashMap_h
#define HashMap_h

#include "PlatformLib.h"

namespace util {
const LInt HASH_TABLE_DEFAULT_SIZE = 1 << 4;
// 阈值，超过容量75%就开始扩容
const LReal HASH_TABLE_DEFAULT_FACTOR = 0.75f;

template <typename K, typename V>
class MapEntry {
public:
    MapEntry()
        : next(kBoyiaNull)
    {
    }
    MapEntry(K k, V v)
        : next(kBoyiaNull)
        , key(k)
        , value(v)
    {
    }

    K key;
    V value;
    LUint hash;
    MapEntry* next;
};

template <typename K, typename V>
class HashMap {
public:
    using HashMapEntryPtr = MapEntry<K, V>*;
    using HashMapEntry = MapEntry<K, V>;

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
        LUint hash = genHash(key);
        HashMapEntryPtr entry = m_table[indexHash(hash)];
        // 如果entry存在，则比较碰撞链表中值
        for (; entry; entry->next) {
            if (entry->key == key) {
                entry->value = val;
                return;
            }
        }

        // 如果超过阈值指定的元素数量
        if (++m_size > m_threshold) {
            resize();
            addEntry(hash, key, val);
        } else {
            addEntry(hash, key, val);
        }
    }

    V get(K key)
    {
        LUint hash = genHash(key);
        HashMapEntryPtr entry = m_table[indexHash(hash)];
        for (; entry; entry->next) {
            if (entry->key == key) {
                return entry->value;
            }
        }

        return V();
    }

    LInt size() const
    {
        return m_size;
    }

    LVoid resize()
    {
        // 2的n次方，满足(n-1)&hash = hash % n
        // 与运算效率远高于取余运算
        LInt oldCapacity = m_capacity;
        HashMapEntryPtr* table = m_table;

        // 双倍扩容
        m_capacity <<= 1;
        m_table = new HashMapEntryPtr[m_capacity];
        LMemset(m_table, 0, m_capacity * sizeof(HashMapEntryPtr));

        // 拷贝原table数据到新table中
        for (LInt i = 0; i < oldCapacity; ++i) {
            HashMapEntryPtr entry = table[i];
            for (; entry; entry->next) {
                addEntry(indexHash(entry->hash), entry);
            }
        }

        m_threshold = (LInt)m_capacity * HASH_TABLE_DEFAULT_FACTOR;
        // 释放之前容器
        delete table;
    }

private:
    LUint genHash(K key)
    {
        LUint hash = (hash = key.hash()) ^ (hash >> 16);
        return hash;
    }

    LInt indexHash(LUint hash)
    {
        return (m_capacity - 1) & hash;
    }

    LVoid addEntry(LInt index, HashMapEntryPtr ptr)
    {
        // 如果链表头部为空，则创建头部
        if (!m_table[index]) {
            m_table[index] = ptr;
        } else {
            // 如果头部不为空，则将entry插在最前面
            // ptr的next指针指向头部
            ptr->next = m_table[index];
            m_table[index] = ptr;
        }
    }

    LVoid addEntry(LUint hash, K key, V value)
    {
        HashMapEntryPtr entry = new HashMapEntry(key, value);
        entry->hash = hash;
        addEntry(indexHash(hash), entry);
    }

    HashMapEntryPtr* m_table;
    LInt m_size;
    LInt m_capacity;
    LInt m_threshold;
};
}

using util::HashMap;
#endif