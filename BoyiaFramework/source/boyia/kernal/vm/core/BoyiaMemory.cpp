/*
* Date: 2018-9-1
* Author: yanbo
* Description: Boyia Memory
* Copyright (c) reserved
*/
#include "BoyiaMemory.h"
//#include <android/log.h>
#include "SalLog.h"
#include <stdlib.h>

typedef struct MemoryBlockHeader {
    LInt mSize;
    LByte* mAddress;
    MemoryBlockHeader* mNext;
    MemoryBlockHeader* mPrevious;
} MemoryBlockHeader;

typedef struct BoyiaMemoryPool {
    LInt mSize;
    LInt mUsed;
    LByte* mAddress;
    BoyiaMemoryPool* mNext;
    MemoryBlockHeader* mFirstBlock;
} BoyiaMemoryPool;

const LInt constHeaderLen = sizeof(MemoryBlockHeader);
// 字节对齐数
const LInt constAlignNum = sizeof(LIntPtr);
// 数据块尾部地址值
#define DATA_TAIL(data) ((LIntPtr)data + data->mSize + constHeaderLen)
// 字节对齐后的地址值
#define ADDR_ALIGN(addr) (addr % constAlignNum == 0 ? addr : (addr + (constAlignNum - addr % constAlignNum)))

LVoid* FastMalloc(LInt size)
{
    return malloc(size);
}

LVoid FastFree(LVoid* data)
{
    free(data);
}

LBool ContainAddress(LVoid* addr, BoyiaMemoryPool* pool)
{
    LIntPtr iAddr = (LIntPtr)addr;
    return iAddr >= (LIntPtr)pool->mAddress && iAddr < ((LIntPtr)pool->mAddress + pool->mSize);
}

LVoid* InitMemoryPool(LInt size)
{
    BoyiaMemoryPool* pool = FAST_NEW(BoyiaMemoryPool);
    pool->mAddress = FAST_NEW_ARRAY(LByte, size);
    pool->mSize = size;
    pool->mNext = kBoyiaNull;
    pool->mUsed = 0;
    pool->mFirstBlock = kBoyiaNull;
    return pool;
}

LVoid FreeMemoryPool(LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    while (pool) {
        BoyiaMemoryPool* poolNext = pool->mNext;
        FAST_DELETE(pool->mAddress);
        FAST_DELETE(pool);
        pool = poolNext;
    }
}

LVoid* NewData(LInt size, LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    MemoryBlockHeader* pHeader = kBoyiaNull;

    LInt mallocSize = size + constHeaderLen;
    if (mallocSize > pool->mSize) {
        return kBoyiaNull;
    }

    if (!pool->mFirstBlock) {
        pHeader = (MemoryBlockHeader*)ADDR_ALIGN((LIntPtr)pool->mAddress);
        BOYIA_LOG("BoyiaMemory pool->address: %lx pHeader %lx constAlignNum %d", (LIntPtr)pool->mAddress, (LIntPtr)pHeader, constAlignNum);
        pHeader->mSize = size;
        pHeader->mAddress = (LByte*)pHeader + constHeaderLen;
        pHeader->mNext = kBoyiaNull;
        pHeader->mPrevious = kBoyiaNull;
        pool->mFirstBlock = pHeader;
    } else {
        MemoryBlockHeader* current = pool->mFirstBlock;
        if ((LIntPtr)current - (LIntPtr)pool->mAddress >= mallocSize) {
            LIntPtr newAddr = ADDR_ALIGN((LIntPtr)pool->mAddress);
            if ((LIntPtr)current - newAddr >= mallocSize) {
                pHeader = (MemoryBlockHeader*)newAddr;
                pHeader->mSize = size;
                pHeader->mAddress = (LByte*)pHeader + constHeaderLen;
                pHeader->mNext = current;
                current->mPrevious = pHeader;
                pHeader->mPrevious = kBoyiaNull;

                pool->mFirstBlock = pHeader;
                pool->mUsed += constHeaderLen + size;
                return pHeader->mAddress;
            }
        }

        while (current) {
            if (!current->mNext) {
                if ((((LIntPtr)pool->mAddress + pool->mSize) - DATA_TAIL(current)) >= mallocSize) {
                    LIntPtr newAddr = ADDR_ALIGN(DATA_TAIL(current));
                    if (((LIntPtr)pool->mAddress + pool->mSize) - newAddr >= mallocSize) {
                        pHeader = (MemoryBlockHeader*)newAddr;
                        pHeader->mSize = size;
                        pHeader->mAddress = (LByte*)pHeader + constHeaderLen;
                        pHeader->mPrevious = current;
                        pHeader->mNext = kBoyiaNull;
                        current->mNext = pHeader;
                        break;
                    }
                }

                // Out Of Memory
                return NULL;
            } else {
                if ((LIntPtr)current->mNext - DATA_TAIL(current) >= mallocSize) {
                    LIntPtr newAddr = ADDR_ALIGN(DATA_TAIL(current));
                    if ((LIntPtr)current->mNext - newAddr >= mallocSize) {
                        pHeader = (MemoryBlockHeader*)newAddr;
                        pHeader->mSize = size;
                        pHeader->mAddress = (LByte*)pHeader + constHeaderLen;
                        pHeader->mPrevious = current;
                        pHeader->mNext = current->mNext;
                        current->mNext->mPrevious = pHeader;
                        current->mNext = pHeader;
                        break;
                    }
                }
            }

            current = current->mNext;
        }
    }

    pool->mUsed += pHeader ? (constHeaderLen + size) : 0;
    return pHeader ? pHeader->mAddress : kBoyiaNull;
}

LVoid DeleteData(LVoid* data, LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    MemoryBlockHeader* pHeader = (MemoryBlockHeader*)((LIntPtr)data - constHeaderLen);
    // If error pointer, then return.
    if ((LIntPtr)pHeader < (LIntPtr)pool->mAddress) {
        return;
    }

    if (pool->mFirstBlock == pHeader) {
        if (pHeader->mNext) {
            pool->mFirstBlock = pHeader->mNext;
        } else {
            pool->mFirstBlock = kBoyiaNull;
        }
    } else {
        if (pHeader->mNext) {
            pHeader->mPrevious->mNext = pHeader->mNext;
            pHeader->mNext->mPrevious = pHeader->mPrevious;
        }
    }

    pool->mUsed -= constHeaderLen + pHeader->mSize;
}

LInt GetUsedMemory(LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    return pool->mUsed;
}

LVoid PrintPoolSize(LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    BOYIA_LOG("BoyiaVM POOL addr=%x used=%d maxsize=%d", (LIntPtr)pool->mAddress, pool->mUsed, pool->mSize);
}
