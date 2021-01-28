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
    //LByte* mAddress;
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

const LInt kMemoryHeaderLen = sizeof(MemoryBlockHeader);
// 字节对齐数
const LInt kMemoryAlignNum = sizeof(LUintPtr);
// 数据块尾部地址值
#define DATA_TAIL(header) ((LUintPtr)header + kMemoryHeaderLen + header->mSize)
// 字节对齐后的地址值
//#define ADDR_ALIGN(addr) (addr % kMemoryAlignNum == 0 ? addr : (addr + (kMemoryAlignNum - addr % kMemoryAlignNum)))
#define MEM_ALIGN(size) (((size) + (kMemoryAlignNum - 1)) & ~(kMemoryAlignNum - 1))
#define ADDR_ALIGN(addr) (((LUintPtr)(addr) + (kMemoryAlignNum - 1)) & ~(kMemoryAlignNum - 1))
#define ADDR_DELTA(addr1, addr2) ((LUintPtr)(addr1) - (LUintPtr)(addr2))

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
        //pHeader->mAddress = (LByte*)pHeader + kMemoryHeaderLen;
        pHeader->mNext = kBoyiaNull;
        pHeader->mPrevious = kBoyiaNull;
        pool->mFirstBlock = pHeader;
    } else {
        MemoryBlockHeader* current = pool->mFirstBlock;
        // 如果第一个内存单元与内存池首地址之间存在一块空白区域
        // 该空白区域大小大于mallocSize，则尝试利用该区域进行分配
        LUintPtr newAddr = ADDR_ALIGN(pool->mAddress);
        if (ADDR_DELTA(current, newAddr) >= mallocSize) {
            pHeader = (MemoryBlockHeader*)newAddr;
            pHeader->mSize = size;
            //pHeader->mAddress = (LByte*)(newAddr + kMemoryHeaderLen);
            pHeader->mNext = current;
            current->mPrevious = pHeader;
            pHeader->mPrevious = kBoyiaNull;

            pool->mFirstBlock = pHeader;
            pool->mUsed += mallocSize;
            //return pHeader->mAddress;
            return (LByte*)(newAddr + kMemoryHeaderLen);
        }

        while (current) {
            // 如果当前单元没有下一个元素
            // 则直接利用剩余的空白空间
            // 如果当前单元存在下一个元素
            // 则尝试利用当前与下一个元素之间的空白区域进行分配
            LUintPtr newAddr = ADDR_ALIGN(DATA_TAIL(current));
            if (!current->mNext) {
                if (ADDR_DELTA(((LUintPtr)pool->mAddress + pool->mSize), newAddr) >= mallocSize) {
                    pHeader = (MemoryBlockHeader*)newAddr;
                    pHeader->mSize = size;
                    //pHeader->mAddress = (LByte*)(newAddr + kMemoryHeaderLen);
                    pHeader->mPrevious = current;
                    pHeader->mNext = kBoyiaNull;
                    current->mNext = pHeader;
                    break;
                }
                BOYIA_LOG("%s: Boyia VM Out of Memory ", "NewData");
                // Out Of Memory
                return kBoyiaNull;
            }

            if (ADDR_DELTA((current->mNext), newAddr) >= mallocSize) {
                pHeader = (MemoryBlockHeader*)newAddr;
                pHeader->mSize = size;
                //pHeader->mAddress = (LByte*)(newAddr + kMemoryHeaderLen);
                pHeader->mPrevious = current;
                pHeader->mNext = current->mNext;
                current->mNext->mPrevious = pHeader;
                current->mNext = pHeader;
                break;
            }

            current = current->mNext;
        }
    }

    pool->mUsed += pHeader ? mallocSize : 0;
    //return pHeader ? pHeader->mAddress : kBoyiaNull;
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

    if (pool->mFirstBlock == pHeader) {
        pool->mFirstBlock = pHeader->mNext;
    } else {
        if (pHeader->mNext) {
            pHeader->mPrevious->mNext = pHeader->mNext;
            pHeader->mNext->mPrevious = pHeader->mPrevious;
        }
    }

    pool->mUsed -= kMemoryHeaderLen + pHeader->mSize;
}

LInt GetUsedMemory(LVoid* mempool)
{
    BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    return pool->mUsed;
}

LVoid PrintPoolSize(LVoid* mempool)
{
    //BoyiaMemoryPool* pool = (BoyiaMemoryPool*)mempool;
    //BOYIA_LOG("BoyiaVM POOL addr=%x used=%d maxsize=%d", (LIntPtr)pool->mAddress, pool->mUsed, pool->mSize);
}
