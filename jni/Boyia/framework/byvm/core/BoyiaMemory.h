#ifndef BoyiaMemory_h
#define BoyiaMemory_h

#include "PlatformLib.h"

typedef struct MemoryBlockHeader {
    LInt               mSize;
    LByte*             mAddress;
    MemoryBlockHeader* mNext;
    MemoryBlockHeader* mPrevious;
} MemoryBlockHeader;

typedef struct BoyiaMemoryPool {
    LInt               mSize;
    LInt               mUsed;
    LByte*             mAddress;
    BoyiaMemoryPool*   mNext;
    MemoryBlockHeader* mFirstBlock;
} BoyiaMemoryPool;

#define FAST_NEW(type) (type*)fastMalloc(sizeof(type))
#define FAST_NEW_ARRAY(type, n) (type*)fastMalloc(n*sizeof(type))
#define FAST_DELETE(p)  fastFree(p)

LVoid* fastMalloc(LInt size);
LVoid fastFree(LVoid* data);

BoyiaMemoryPool* InitMemoryPool(LInt size);
LVoid FreeMemoryPool(BoyiaMemoryPool* pool);

LVoid* newData(LInt size, BoyiaMemoryPool* pool);
LVoid deleteData(LVoid* data, BoyiaMemoryPool* pool);

LBool containAddress(LVoid* addr, BoyiaMemoryPool* pool);

LVoid printPoolSize(BoyiaMemoryPool* pool);

#endif // Boyia_Memory_h
