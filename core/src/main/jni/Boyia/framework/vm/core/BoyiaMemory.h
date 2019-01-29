#ifndef BoyiaMemory_h
#define BoyiaMemory_h

#include "PlatformLib.h"

#define FAST_NEW(type) (type*)FastMalloc(sizeof(type))
#define FAST_NEW_ARRAY(type, n) (type*)FastMalloc(n*sizeof(type))
#define FAST_DELETE(p)  FastFree(p)

LVoid* FastMalloc(LInt size);
LVoid FastFree(LVoid* data);

LVoid* InitMemoryPool(LInt size);
LVoid FreeMemoryPool(LVoid* mempool);

LVoid* NewData(LInt size, LVoid* mempool);
LVoid DeleteData(LVoid* data, LVoid* mempool);

LBool ContainAddress(LVoid* addr, LVoid* mempool);
LInt GetUsedMemory(LVoid* mempool);

LVoid PrintPoolSize(LVoid* mempool);

#endif // Boyia_Memory_h
