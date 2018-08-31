#ifndef MiniMemory_h
#define MiniMemory_h

#include "PlatformLib.h"

typedef struct MemoryBlockHeader
{
	LInt               m_size;
	LByte*             m_address;
    MemoryBlockHeader* m_next;
	MemoryBlockHeader* m_previous;
} MemoryBlockHeader;

typedef struct MiniMemoryPool
{
	LInt               m_size;
	LInt               m_used;
	LByte*             m_address;
	MiniMemoryPool*    m_next;
	MemoryBlockHeader* m_firstBlock;
} MiniMemoryPool;


MiniMemoryPool* initMemoryPool(LInt size);
LVoid freeMemoryPool(MiniMemoryPool* pool);

LVoid* newData(LInt size, MiniMemoryPool* pool);
LVoid deleteData(LVoid* data, MiniMemoryPool* pool);

LBool containAddress(LVoid* addr, MiniMemoryPool* pool);

LVoid printPoolSize(MiniMemoryPool* pool);

#endif // Mini_Memory_h
