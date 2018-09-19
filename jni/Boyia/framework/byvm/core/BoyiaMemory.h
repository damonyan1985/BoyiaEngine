#ifndef BoyiaMemory_h
#define BoyiaMemory_h

#include "PlatformLib.h"

typedef struct MemoryBlockHeader {
	LInt               m_size;
	LByte*             m_address;
    MemoryBlockHeader* m_next;
	MemoryBlockHeader* m_previous;
} MemoryBlockHeader;

typedef struct BoyiaMemoryPool {
    LInt               m_size;
    LInt               m_used;
    LByte*             m_address;
    BoyiaMemoryPool*   m_next;
    MemoryBlockHeader* m_firstBlock;
} BoyiaMemoryPool;


BoyiaMemoryPool* initMemoryPool(LInt size);
LVoid freeMemoryPool(BoyiaMemoryPool* pool);

LVoid* newData(LInt size, BoyiaMemoryPool* pool);
LVoid deleteData(LVoid* data, BoyiaMemoryPool* pool);

LBool containAddress(LVoid* addr, BoyiaMemoryPool* pool);

LVoid printPoolSize(BoyiaMemoryPool* pool);

#endif // Boyia_Memory_h
