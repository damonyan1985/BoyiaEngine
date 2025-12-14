/*
* Date: 2018-9-1
* Author: yanbo
* Description: Boyia Memory
* Copyright (c) reserved
*/
#include "BoyiaMemory.h"
#include "SalLog.h"
#include <stdlib.h>

typedef struct MemoryBlockHeader {
    LInt mSize;
    LInt mFlag;
    MemoryBlockHeader* mNext;
    MemoryBlockHeader* mPrevious;
} MemoryBlockHeader;

typedef struct BoyiaMemoryPool {
    LInt mSize;
    LInt mUsed;
    LByte* mAddress;
    MemoryBlockHeader* mFirstBlock;
} BoyiaMemoryPool;

const LInt kMemoryBlockHeaderFlag = 18;

const LInt kMemoryHeaderLen = sizeof(MemoryBlockHeader);
// 字节对齐数
const LInt kMemoryAlignNum = sizeof(LUintPtr);
// 数据块尾部地址值
#define DATA_TAIL(header) ((LByte*)header + kMemoryHeaderLen + header->mSize)
// 字节对齐后的地址值

#define MEM_ALIGN(size) (((size) + (kMemoryAlignNum - 1)) & ~(kMemoryAlignNum - 1))
#define ADDR_ALIGN(addr) (((LUintPtr)(addr) + ((LUintPtr)kMemoryAlignNum - 1)) & ~((LUintPtr)kMemoryAlignNum - 1))
//#define ADDR_ALIGN(addr) (((LUintPtr)addr) % kMemoryAlignNum == 0 ? ((LUintPtr)addr) : (((LUintPtr)addr) + (kMemoryAlignNum - ((LUintPtr)addr) % kMemoryAlignNum)))

#define ADDR_DELTA(addr1, addr2) ((LIntPtr)(addr1) - (LIntPtr)(addr2))

// VM中固定不变的内存块使用FastMalloc
LVoid* FastMalloc(LInt size)
{
    return malloc(size);
}

LVoid FastFree(LVoid* data)
{
    free(data);
}

LBool ContainAddress(LVoid* addr, LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    LUintPtr iAddr = (LUintPtr)addr;
    return iAddr >= (LUintPtr)pool->mAddress && iAddr < ((LUintPtr)pool->mAddress + pool->mSize);
}

LVoid* InitMemoryPool(LInt size)
{
    BoyiaMemoryPool* pool = FAST_NEW(BoyiaMemoryPool);
    pool->mAddress = FAST_NEW_ARRAY(LByte, size);
    pool->mSize = size;
    pool->mUsed = 0;
    pool->mFirstBlock = kBoyiaNull;
    return pool;
}

LVoid FreeMemoryPool(LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    FAST_DELETE(pool->mAddress);
    FAST_DELETE(pool);
}

LVoid* NewData(LInt size, LVoid* mempool)
{
    size = MEM_ALIGN(size);
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    LInt mallocSize = size + kMemoryHeaderLen;
    if (mallocSize > pool->mSize) {
        return kBoyiaNull;
    }

    MemoryBlockHeader* pHeader = kBoyiaNull;
    // 如果firstBlock不存在，则创建内存单元，然后完成
    if (!pool->mFirstBlock) {
        pHeader = (MemoryBlockHeader*)ADDR_ALIGN(pool->mAddress);
        BOYIA_LOG("BoyiaMemory pool->address: %lx pHeader %lx constAlignNum %d", (LUintPtr)pool->mAddress, (LUintPtr)pHeader, kMemoryAlignNum);
        pHeader->mSize = size;
        pHeader->mNext = kBoyiaNull;
        pHeader->mPrevious = kBoyiaNull;
        pHeader->mFlag = kMemoryBlockHeaderFlag;
        pool->mFirstBlock = pHeader;
        pool->mUsed = mallocSize;

        return (LByte*)pHeader + kMemoryHeaderLen;
    }


    MemoryBlockHeader* current = pool->mFirstBlock;
    // 如果第一个内存单元与内存池首地址之间存在一块空白区域
    // 该空白区域大小大于mallocSize，则尝试利用该区域进行分配
    LByte* maxAddress = pool->mAddress + pool->mSize;
    // 获取内存块末尾对齐地址
    LUintPtr newAddr = ADDR_ALIGN(DATA_TAIL(current));
    // 最大内存地址减去first内存末尾地址
    if (ADDR_DELTA(maxAddress, newAddr) >= mallocSize) {
        pHeader = (MemoryBlockHeader*)newAddr;
        pHeader->mSize = size;
        pHeader->mNext = current;
        current->mPrevious = pHeader;
        pHeader->mPrevious = kBoyiaNull;
        pHeader->mFlag = kMemoryBlockHeaderFlag;

        // 将新分配的地址作为内存链表头部
        pool->mFirstBlock = pHeader;
        pool->mUsed += mallocSize;
        return (LByte*)(newAddr + kMemoryHeaderLen);
    }

    while (current) {
        // 如果当前单元没有下一个元素
        // 则直接利用剩余的空白空间
        //BOYIA_LOG("CURRENT PTR %llx %lld, curr.size=%d size=%d", (LUintPtr)current, (LIntPtr)current, (current->mSize), size);
        if (!current->mNext) {
            LUintPtr newAddr = ADDR_ALIGN(pool->mAddress);
            if (ADDR_DELTA(current, newAddr) >= mallocSize) {
                pHeader = (MemoryBlockHeader*)newAddr;
                pHeader->mSize = size;
                pHeader->mPrevious = current;
                pHeader->mNext = kBoyiaNull;
                pHeader->mFlag = kMemoryBlockHeaderFlag;
                current->mNext = pHeader;
                break;
            }
            BOYIA_LOG("%s: Boyia VM Out of Memory ", "NewData");
            // Out Of Memory
            return kBoyiaNull;
        }

        // 如果当前单元存在下一个元素
        // 则尝试利用当前与下一个元素之间的空白区域进行分配
        newAddr = ADDR_ALIGN(DATA_TAIL(current->mNext));
        if (ADDR_DELTA(current, newAddr) >= mallocSize) {
            pHeader = (MemoryBlockHeader*)newAddr;
            pHeader->mSize = size;
            pHeader->mPrevious = current;
            pHeader->mNext = current->mNext;
            pHeader->mFlag = kMemoryBlockHeaderFlag;
            current->mNext->mPrevious = pHeader;
            current->mNext = pHeader;
            break;
        }

        current = current->mNext;
    }
    

    pool->mUsed += pHeader ? mallocSize : 0;
    return pHeader ? ((LByte*)pHeader + kMemoryHeaderLen) : kBoyiaNull;
}

LVoid DeleteData(LVoid* data, LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    MemoryBlockHeader* pHeader = (MemoryBlockHeader*)((LUintPtr)data - kMemoryHeaderLen);
    // If error pointer, then return.
    if ((LUintPtr)pHeader < (LUintPtr)pool->mAddress) {
        return;
    }

    if (pHeader->mFlag != kMemoryBlockHeaderFlag) {
        return;
    }

    if (pHeader->mNext) {
        pHeader->mNext->mPrevious = pHeader->mPrevious;
    }

    if (pool->mFirstBlock == pHeader) {
        pool->mFirstBlock = pHeader->mNext;
    } else {
        if (pHeader->mNext) {
            pHeader->mPrevious->mNext = pHeader->mNext;
        }
    }

    pool->mUsed -= kMemoryHeaderLen + pHeader->mSize;
}

LInt GetUsedMemory(LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    return pool->mUsed;
}

LVoid* MigrateMemory(LVoid* srcMem, LVoid* fromPool, LVoid* toPool)
{
    // 获取内存块头部
    MemoryBlockHeader* srcHeader = (MemoryBlockHeader*)((LUintPtr)srcMem - kMemoryHeaderLen);
    // 根据大小从新内存池中生成一个新的内存块
    LVoid* destMem = NewData(srcHeader->mSize, toPool);
    // 拷贝内存
    LMemcpy(destMem, srcMem, srcHeader->mSize);
    // 返回地址
    return destMem;
}

LVoid PrintPoolSize(LVoid* mempool)
{
    //BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    //BOYIA_LOG("BoyiaVM POOL addr=%x used=%d maxsize=%d", (LIntPtr)pool->mAddress, pool->mUsed, pool->mSize);
}

typedef struct MemoryChunk {
    LVoid* mMemoryAddr;
    MemoryChunk* mNext;
} MemoryChunk;

typedef struct {
    LVoid* mCacheAddr;
    MemoryChunk* mChunkCache;
    MemoryChunk* mFreeChunks;
    LInt mUseIndex;

    LInt mUnitSize;
    LInt mCount;
    LInt mCapacity;
} MemoryCache;

LVoid* CreateMemoryCache(LInt typeSize, LInt capacity) {
    MemoryCache* cache = FAST_NEW(MemoryCache);
    cache->mCacheAddr = FastMalloc(typeSize * capacity);
    LMemset(cache->mCacheAddr, 0, typeSize * capacity);

    cache->mChunkCache = (MemoryChunk*)FastMalloc(sizeof(MemoryChunk) * capacity);
    cache->mUnitSize = typeSize;
    cache->mCount = 0;
    cache->mUseIndex = 0;
    cache->mCapacity = capacity;
    cache->mFreeChunks = &cache->mChunkCache[0];
    {
        cache->mFreeChunks->mMemoryAddr = cache->mCacheAddr;
        cache->mFreeChunks->mNext = kBoyiaNull;
    }

    return cache;
}

LVoid* AllocMemoryChunk(LVoid* cachePtr) {
    MemoryCache* cache = (MemoryCache*)cachePtr;
    // 分配从mFreeChunks链表中进行分配
    MemoryChunk* chunk = cache->mFreeChunks;
    if (chunk && chunk->mNext) {
        cache->mFreeChunks = chunk->mNext;
    } else {
        if (cache->mUseIndex >= cache->mCapacity - 1) {
            // mUseIndex表示从0开始到capacity按顺序分配完了
            cache->mFreeChunks = kBoyiaNull;
            if (!chunk) {
                // (TODO) Out of Memory
                return kBoyiaNull;
            }
            ++cache->mCount;
            return chunk->mMemoryAddr;
        }
        cache->mFreeChunks = &cache->mChunkCache[++cache->mUseIndex];
        {
            cache->mFreeChunks->mMemoryAddr = (LByte*)cache->mCacheAddr + cache->mUseIndex * cache->mUnitSize;
            cache->mFreeChunks->mNext = kBoyiaNull;
        }
    }

    ++cache->mCount;
    return chunk ? chunk->mMemoryAddr : kBoyiaNull;
}

LVoid FreeMemoryChunk(LVoid* addr, LVoid* cachePtr) {
    MemoryCache* cache = (MemoryCache*)cachePtr;

    LUintPtr startAddrValue = (LUintPtr)((LByte*)cache->mCacheAddr);
    LUintPtr addrValue = (LUintPtr)((LByte*)addr);
    LInt index = (addrValue - startAddrValue) / cache->mUnitSize;

    LMemset(addr, 0, cache->mUnitSize);

    MemoryChunk* chunk = &cache->mChunkCache[index];
    chunk->mNext = cache->mFreeChunks;
    cache->mFreeChunks = chunk;
    --cache->mCount;
}

LVoid DestroyMemoryCache(LVoid* cachePtr) {
    MemoryCache* cache = (MemoryCache*)cachePtr;
    FastFree(cache->mCacheAddr);
    FastFree(cache->mChunkCache);
    FastFree(cache);
}

LInt GetUsedChunkCount(LVoid* cachePtr) {
    return ((MemoryCache*)cachePtr)->mCount;
}