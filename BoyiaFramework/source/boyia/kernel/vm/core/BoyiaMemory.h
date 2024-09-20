#ifndef BoyiaMemory_h
#define BoyiaMemory_h

#include "PlatformLib.h"


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


#define CREATE_MEMCACHE(type, capacity) CreateMemoryCache(sizeof(type), capacity)
#define ALLOC_CHUNK(type, cache) ((type*)AllocMemoryChunk(cache))
#define FREE_CHUNK(addr, cache) FreeMemoryChunk(addr, cache)
#define DESTROY_MEMCACHE(cache) DestroyMemoryCache(cache);

LVoid* CreateMemoryCache(LInt typeSize, LInt capacity);
LVoid* AllocMemoryChunk(LVoid* cache);
LVoid FreeMemoryChunk(LVoid* addr, LVoid* cache);
LVoid DestroyMemoryCache(LVoid* cache);
LInt GetUsedChunkCount(LVoid* cache);

#endif // Boyia_Memory_h
