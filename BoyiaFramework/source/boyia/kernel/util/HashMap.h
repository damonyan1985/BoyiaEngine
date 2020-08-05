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
    MapEntry(const K& k, const V& v)
        : next(kBoyiaNull)
        , key(k)
        , value(v)
        , hash(0)
    {
    }

    K key;
    V value;
    LUint hash;
    MapEntry* next;
};

// 1. 成员变量初始化顺序与构造函数中成员变量初始化列表顺序无关
// 与成员变量在类中声明的顺序有关
// 2. 如果构造函数中成员变量初始化列表没有包含某成员变量
// 则其初始化会默认排在成员变量初始化列表之后才进行初始化
template <typename K, typename V>
class HashMap {
public:
    using HashMapEntryPtr = MapEntry<K, V>*;
    using HashMapEntry = MapEntry<K, V>;

public:
    HashMap()
        : m_size(0)
        , m_threshold(HASH_TABLE_DEFAULT_SIZE * HASH_TABLE_DEFAULT_FACTOR)
        , m_capacity(HASH_TABLE_DEFAULT_SIZE)
        , m_table(new HashMapEntryPtr[HASH_TABLE_DEFAULT_SIZE])
    {
        LMemset(m_table, 0, m_capacity * sizeof(HashMapEntryPtr));
    }

    ~HashMap()
    {
        for (LInt i = 0; i < m_capacity; i++) {
            HashMapEntryPtr entry = m_table[i];
            while (entry) {
                HashMapEntryPtr next = entry->next;
                delete entry;
                entry = next;
            }
        }
        delete[] m_table;
    }

    LVoid put(const K& key, const V& val)
    {
        // 如果超过阈值指定的元素数量
        if (++m_size > m_threshold) {
            resize();
        }
        
        addEntry(genHash(key), key, val);
    }

    V get(const K& key)
    {
        LUint hash = genHash(key);
        HashMapEntryPtr entry = m_table[indexHash(hash)];
        for (; entry; entry = entry->next) {
            // 先判断hash值来节省时间
            if (hash == entry->hash && entry->key == key) {
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
        HashMapEntryPtr* oldTable = m_table;

        // 双倍扩容
        m_capacity <<= 1;
        m_table = new HashMapEntryPtr[m_capacity];
        LMemset(m_table, 0, m_capacity * sizeof(HashMapEntryPtr));

        // 拷贝原table数据到新table中
        for (LInt i = 0; i < oldCapacity; ++i) {
            HashMapEntryPtr entry = oldTable[i];
            while (entry) {
                HashMapEntryPtr next = entry->next;
                resetEntry(entry);
                entry = next;
            }
        }

        // 释放之前容器
        delete[] oldTable;
        // 重新设置阈值
        m_threshold = (LInt)m_capacity * HASH_TABLE_DEFAULT_FACTOR;
    }

private:
    LUint genHash(const K& key)
    {
        LUint hash = (hash = key.hash()) ^ (hash >> 16);
        return hash;
    }

    LInt indexHash(LUint hash)
    {
        return (m_capacity - 1) & hash;
    }

    LVoid resetEntry(HashMapEntryPtr ptr)
    {
        LUint index = indexHash(ptr->hash);
        // 如果链表头部为空，则创建头部
        if (!m_table[index]) {
            m_table[index] = ptr;
            // next置空
            ptr->next = kBoyiaNull;
        } else {
            // 如果头部不为空，则将entry插在最前面
            // ptr的next指针指向头部
            ptr->next = m_table[index];
            m_table[index] = ptr;
        }
    }

    LVoid addEntry(LUint hash, const K& key, V value)
    {
        LInt index = indexHash(hash);
        if (m_table[index]) {
            // 遍历hash碰撞链表，比较key值
            // 如果key值相同，则直接复制，否则
            HashMapEntryPtr entry = m_table[index];
            for (; entry; entry = entry->next) {
                if (hash == entry->hash && entry->key == key) {
                    entry->value = value;
                    return;
                }
            }
        }

        // 初始化一个新的entry
        HashMapEntryPtr entry = new HashMapEntry(key, value);
        entry->hash = hash;
        // entry插入hash碰撞链表头部
        entry->next = m_table[index];
        m_table[index] = entry;
    }

    HashMapEntryPtr* m_table;
    LInt m_size;
    LInt m_capacity;
    LInt m_threshold;
};
}

using util::HashMap;
#endif