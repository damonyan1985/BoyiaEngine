#ifndef BoyiaMemory_h
#define BoyiaMemory_h

#include "PlatformLib.h"

typedef struct MemoryChunk {
    LVoid* mMemoryAddr;
    MemoryChunk* mNext;
} MemoryChunk;

typedef struct {
    LVoid* mCacheAddr;
    MemoryChunk* mChunkCache;
    struct {
        MemoryChunk* mHead;
        MemoryChunk* mEnd;
    } mUsedChunks;

    MemoryChunk* mFreeChunks;
    LInt mUseIndex;
    LInt mSize;
    LInt mCapacity;
} MemoryCache;

#define CREATE_MEMCACHE(type, capacity) CreateMemoryCache(sizeof(type), capacity)
#define ALLOC_CHUNK(type, cache) ((type*)AllocMemoryChunk(sizeof(type), cache))
#define FREE_CHUNK(chunk, cache) FreeMemoryChunk(chunk, cache)

MemoryCache* CreateMemoryCache(LInt typeSize, LInt capacity);
LVoid* AllocMemoryChunk(LInt typeSize, MemoryCache* cache);
LVoid FreeMemoryChunk(MemoryChunk* chunk, MemoryCache* cache);


#define FAST_NEW(type) (type*)FastMalloc(sizeof(type))
#define FAST_NEW_ARRAY(type, n) (type*)FastMalloc(n * sizeof(type))
#define FAST_DELETE(p) FastFree(p)

LVoid* FastMalloc(LInt size);
LVoid FastFree(LVoid* data);

LVoid* InitMemoryPool(LInt size);
LVoid FreeMemoryPool(LVoid* mempool);

LVoid* NewData(LInt size, LVoid* mempool);
LVoid DeleteData(LVoid* data, LVoid* mempool);

LBool ContainAddress(LVoid* addr, LVoid* mempool);
LInt GetUsedMemory(LVoid* mempool);

LVoid PrintPoolSize(LVoid* mempool);

LVoid* MigrateMemory(LVoid* srcMem, LVoid* fromPool, LVoid* toPool);

#endif // Boyia_Memory_h
