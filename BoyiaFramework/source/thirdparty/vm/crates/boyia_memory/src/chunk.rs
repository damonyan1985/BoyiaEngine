//! Memory chunk cache. CreateMemoryCache, AllocMemoryChunk, FreeMemoryChunk, DestroyMemoryCache.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::memory::{fast_free, fast_malloc};
use crate::{LInt, LUintPtr, LVoid};
use std::ptr;

#[repr(C)]
struct MemoryChunk {
    mMemoryAddr: *mut LVoid,
    mNext: *mut MemoryChunk,
}

#[repr(C)]
struct MemoryCache {
    mCacheAddr: *mut LVoid,
    mChunkCache: *mut MemoryChunk,
    mFreeChunks: *mut MemoryChunk,
    mUseIndex: LInt,
    mUnitSize: LInt,
    mCount: LInt,
    mCapacity: LInt,
}

/// CreateMemoryCache: fixed-size chunk pool. Uses FastMalloc for all three blocks.
pub unsafe fn create_memory_cache(type_size: LInt, capacity: LInt) -> *mut LVoid {
    if type_size <= 0 || capacity <= 0 {
        return ptr::null_mut();
    }
    let total = (type_size as usize) * (capacity as usize);
    let cache = fast_malloc(std::mem::size_of::<MemoryCache>() as LInt) as *mut MemoryCache;
    if cache.is_null() {
        return ptr::null_mut();
    }
    (*cache).mCacheAddr = fast_malloc(total as LInt);
    if (*cache).mCacheAddr.is_null() {
        fast_free(cache as *mut LVoid);
        return ptr::null_mut();
    }
    ptr::write_bytes((*cache).mCacheAddr as *mut u8, 0, total);
    let chunk_len = std::mem::size_of::<MemoryChunk>() * capacity as usize;
    (*cache).mChunkCache = fast_malloc(chunk_len as LInt) as *mut MemoryChunk;
    if (*cache).mChunkCache.is_null() {
        fast_free((*cache).mCacheAddr);
        fast_free(cache as *mut LVoid);
        return ptr::null_mut();
    }
    (*cache).mUnitSize = type_size;
    (*cache).mCount = 0;
    (*cache).mUseIndex = 0;
    (*cache).mCapacity = capacity;
    (*cache).mFreeChunks = (*cache).mChunkCache;
    (*((*cache).mChunkCache)).mMemoryAddr = (*cache).mCacheAddr;
    (*((*cache).mChunkCache)).mNext = ptr::null_mut();
    cache as *mut LVoid
}

/// AllocMemoryChunk: get one chunk from cache.
pub unsafe fn alloc_memory_chunk(cache_ptr: *mut LVoid) -> *mut LVoid {
    if cache_ptr.is_null() {
        return ptr::null_mut();
    }
    let cache = &mut *(cache_ptr as *mut MemoryCache);
    let chunk = cache.mFreeChunks;
    if !chunk.is_null() && !(*chunk).mNext.is_null() {
        cache.mFreeChunks = (*chunk).mNext;
    } else {
        if cache.mUseIndex >= cache.mCapacity - 1 {
            cache.mFreeChunks = ptr::null_mut();
            if chunk.is_null() {
                return ptr::null_mut();
            }
            cache.mCount += 1;
            return (*chunk).mMemoryAddr;
        }
        cache.mUseIndex += 1;
        cache.mFreeChunks = cache.mChunkCache.add(cache.mUseIndex as usize);
        (*cache.mFreeChunks).mMemoryAddr = (cache.mCacheAddr as *mut u8).add(cache.mUseIndex as usize * cache.mUnitSize as usize) as *mut LVoid;
        (*cache.mFreeChunks).mNext = ptr::null_mut();
    }
    cache.mCount += 1;
    if chunk.is_null() {
        ptr::null_mut()
    } else {
        (*chunk).mMemoryAddr
    }
}

/// FreeMemoryChunk: return chunk to cache.
pub unsafe fn free_memory_chunk(addr: *mut LVoid, cache_ptr: *mut LVoid) {
    if addr.is_null() || cache_ptr.is_null() {
        return;
    }
    let cache = &mut *(cache_ptr as *mut MemoryCache);
    let start_addr_value = cache.mCacheAddr as LUintPtr;
    let addr_value = addr as LUintPtr;
    let index = (addr_value - start_addr_value) / (cache.mUnitSize as LUintPtr);
    ptr::write_bytes(addr as *mut u8, 0, cache.mUnitSize as usize);
    let chunk = cache.mChunkCache.add(index);
    (*chunk).mNext = cache.mFreeChunks;
    cache.mFreeChunks = chunk;
    cache.mCount -= 1;
}

/// DestroyMemoryCache: free cache and its buffers (FastFree for all three).
pub unsafe fn destroy_memory_cache(cache_ptr: *mut LVoid) {
    if cache_ptr.is_null() {
        return;
    }
    let cache = &*(cache_ptr as *const MemoryCache);
    fast_free(cache.mCacheAddr);
    fast_free(cache.mChunkCache as *mut LVoid);
    fast_free(cache_ptr);
}

/// GetUsedChunkCount: number of chunks currently allocated.
pub unsafe fn get_used_chunk_count(cache_ptr: *mut LVoid) -> LInt {
    if cache_ptr.is_null() {
        return 0;
    }
    (*(cache_ptr as *const MemoryCache)).mCount
}
