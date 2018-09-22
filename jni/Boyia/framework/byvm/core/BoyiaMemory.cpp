/*
* Date: 2018-9-1
* Author: yanbo
* Description: Boyia Memory
* Copyright (c) reserved
*/
#include "BoyiaMemory.h"
#include <android/log.h>


const LInt constHeaderLen = sizeof(MemoryBlockHeader);
// 字节对齐数
const LInt constAlignNum = sizeof(LIntPtr);
// 数据块尾部地址值
#define DATA_TAIL(data) ((LIntPtr)data + data->m_size + constHeaderLen)
// 字节对齐后的地址值
#define ADDR_ALIGN(addr) (addr%constAlignNum == 0 ? addr : (addr + (constAlignNum - addr%constAlignNum)))

static LVoid* fastMalloc(LInt size)
{
	return new LByte[size];
}

static LVoid fastFree(LVoid* data)
{
	LByte* p = (LByte*)data;
	delete[] p;
}

#define NEW_DATA(type) (type*)fastMalloc(sizeof(type))
#define NEW_DATA_ARRAY(type, n) (type*)fastMalloc(n*sizeof(type))
#define DELETE_DATA(p)  fastFree(p)

LBool containAddress(LVoid* addr, BoyiaMemoryPool* pool)
{
	LInt iAddr = (LInt) addr;
    return iAddr >= (LInt) pool->m_address && iAddr < ((LInt)pool->m_address + pool->m_size);
}

BoyiaMemoryPool* initMemoryPool(LInt size)
{
	BoyiaMemoryPool* pool = NEW_DATA(BoyiaMemoryPool);
    pool->m_address = NEW_DATA_ARRAY(LByte, size);
    pool->m_size = size;
    pool->m_next = NULL;
    pool->m_used = 0;
    pool->m_firstBlock = NULL;
    return pool;
}

LVoid freeMemoryPool(BoyiaMemoryPool* pool)
{
	while (pool)
	{
		BoyiaMemoryPool* poolNext = pool->m_next;
		DELETE_DATA(pool->m_address);
		DELETE_DATA(pool);
		pool = poolNext;
	}
}

LVoid* newData(LInt size, BoyiaMemoryPool* pool)
{
	MemoryBlockHeader* pHeader = NULL;

	LInt mallocSize = size + constHeaderLen;
	if (mallocSize > pool->m_size)
    {
        return NULL;
    }

    if (!pool->m_firstBlock)
	{
        pHeader = (MemoryBlockHeader*)ADDR_ALIGN((LIntPtr)pool->m_address);
		pHeader->m_size = size;
		pHeader->m_address = (LByte*)pHeader + constHeaderLen;
		pHeader->m_next = NULL;
		pHeader->m_previous = NULL;
		pool->m_firstBlock = pHeader;
	}
	else
	{
		MemoryBlockHeader* current = pool->m_firstBlock;
		if ((LIntPtr)current - (LIntPtr)pool->m_address >= mallocSize)
		{
			LIntPtr newAddr = ADDR_ALIGN((LIntPtr)pool->m_address);
			if ((LIntPtr)current - newAddr >= mallocSize)
			{
				pHeader = (MemoryBlockHeader*)newAddr;
				pHeader->m_size = size;
				pHeader->m_address = (LByte*)pHeader + constHeaderLen;
				pHeader->m_next = current;
				current->m_previous = pHeader;
				pHeader->m_previous = NULL;

				pool->m_firstBlock = pHeader;
				pool->m_used += constHeaderLen + size;
				return pHeader->m_address;
			}
		}
		
		while (current)
		{
			if (!current->m_next)
			{
				if ((((LIntPtr)pool->m_address + pool->m_size) - DATA_TAIL(current)) >= mallocSize)
				{
					LIntPtr newAddr = ADDR_ALIGN(DATA_TAIL(current));
					if (((LIntPtr)pool->m_address + pool->m_size) - newAddr >= mallocSize)
					{
	                    pHeader = (MemoryBlockHeader*)newAddr;
					    pHeader->m_size = size;
					    pHeader->m_address = (LByte*)pHeader + constHeaderLen;
	                    pHeader->m_previous = current;
					    pHeader->m_next = NULL;
						current->m_next = pHeader;
					    break;
					}
				}

				// Out Of Memory
				return NULL;
			}
            else
			{
            	if ((LIntPtr)current->m_next - DATA_TAIL(current) >= mallocSize)
            	{
            		LIntPtr newAddr = ADDR_ALIGN(DATA_TAIL(current));
            		if ((LIntPtr)current->m_next - newAddr >= mallocSize)
            		{
                        pHeader = (MemoryBlockHeader*)newAddr;
        				pHeader->m_size = size;
        				pHeader->m_address = (LByte*)pHeader + constHeaderLen;
                        pHeader->m_previous = current;
        				pHeader->m_next = current->m_next;
        				current->m_next->m_previous = pHeader;
        				current->m_next = pHeader;
        				break;
            		}
            	}
			}

			current = current->m_next;
		}
	}

    pool->m_used += pHeader ? (constHeaderLen + size) : 0;
	return pHeader ? pHeader->m_address : NULL;
}

LVoid deleteData(LVoid* data, BoyiaMemoryPool* pool)
{
    MemoryBlockHeader* pHeader = (MemoryBlockHeader*)((LIntPtr)data - constHeaderLen);
	// If error pointer, then return.
	if ((LIntPtr)pHeader < (LIntPtr)pool->m_address)
	{
		return;
	}

	if (pool->m_firstBlock == pHeader)
	{
		if (pHeader->m_next)
		{
			pool->m_firstBlock = pHeader->m_next;
		}
		else
		{
			pool->m_firstBlock = NULL;
		}
	}
	else
	{
		if (pHeader->m_next)
		{
            pHeader->m_previous->m_next = pHeader->m_next;
            pHeader->m_next->m_previous = pHeader->m_previous;
		}
	}

	pool->m_used -= constHeaderLen + pHeader->m_size;
}

LVoid printPoolSize(BoyiaMemoryPool* pool)
{
	__android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "BoyiaVM POOL addr=%x used=%d maxsize=%d", (LIntPtr)pool->m_address, pool->m_used, pool->m_size);
}
