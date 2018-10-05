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

#define FAST_NEW(type) (type*)FastMalloc(sizeof(type))
#define FAST_NEW_ARRAY(type, n) (type*)FastMalloc(n*sizeof(type))
#define FAST_DELETE(p)  FastFree(p)

LVoid* FastMalloc(LInt size);
LVoid FastFree(LVoid* data);

BoyiaMemoryPool* InitMemoryPool(LInt size);
LVoid FreeMemoryPool(BoyiaMemoryPool* pool);

LVoid* NewData(LInt size, BoyiaMemoryPool* pool);
LVoid DeleteData(LVoid* data, BoyiaMemoryPool* pool);

LBool ContainAddress(LVoid* addr, BoyiaMemoryPool* pool);

LVoid printPoolSize(BoyiaMemoryPool* pool);

#endif // Boyia_Memory_h
