//! Boyia memory. Port of BoyiaMemory.cpp / BoyiaMemory.h.
//! Provides: FastMalloc/FastFree, memory pool (InitMemoryPool, NewData, DeleteData),
//! and memory cache (CreateMemoryCache, AllocMemoryChunk, FreeMemoryChunk).

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::alloc::{alloc, dealloc, Layout};
use std::ffi::c_void;
use std::os::raw::c_int;
use std::ptr;

// Type aliases matching PlatformLib.h / boyia_vm for ABI compatibility
pub type LInt = c_int;
pub type LIntPtr = isize;
pub type LUintPtr = usize;
pub type LVoid = c_void;
pub type LByte = u8;
pub type LBool = LInt;

const K_MEMORY_BLOCK_HEADER_FLAG: LInt = 18;

/// Memory block header (MemoryBlockHeader in C++).
#[repr(C)]
pub struct MemoryBlockHeader {
    pub mSize: LInt,
    pub mFlag: LInt,
    pub mNext: *mut MemoryBlockHeader,
    pub mPrevious: *mut MemoryBlockHeader,
}

/// Memory pool (BoyiaMemoryPool in C++).
#[repr(C)]
pub struct BoyiaMemoryPool {
    pub mSize: LInt,
    pub mUsed: LInt,
    pub mAddress: *mut LByte,
    pub mFirstBlock: *mut MemoryBlockHeader,
}

const K_MEMORY_HEADER_LEN: usize = std::mem::size_of::<MemoryBlockHeader>();
const K_MEMORY_ALIGN_NUM: usize = std::mem::size_of::<LUintPtr>();

#[inline]
fn mem_align(size: LInt) -> LInt {
    ((size as usize + K_MEMORY_ALIGN_NUM - 1) & !(K_MEMORY_ALIGN_NUM - 1)) as LInt
}

#[inline]
fn addr_align(addr: *mut LByte) -> *mut LByte {
    let u = addr as LUintPtr;
    let aligned = (u + (K_MEMORY_ALIGN_NUM - 1)) & !(K_MEMORY_ALIGN_NUM - 1);
    aligned as *mut LByte
}

#[inline]
fn data_tail(header: *mut MemoryBlockHeader) -> *mut LByte {
    unsafe {
        (header as *mut u8).add(K_MEMORY_HEADER_LEN + (*header).mSize as usize) as *mut LByte
    }
}

#[inline]
fn addr_delta(addr1: *mut LByte, addr2: *mut LByte) -> LIntPtr {
    (addr1 as LIntPtr) - (addr2 as LIntPtr)
}

/// FastMalloc: allocate raw memory (VM fixed blocks). Port of FastMalloc.
pub unsafe fn fast_malloc(size: LInt) -> *mut LVoid {
    fast_malloc_sized(size)
}

/// FastFree: free pointer from FastMalloc. Port of FastFree.
pub unsafe fn fast_free(data: *mut LVoid) {
    fast_free_sized(data);
}

unsafe fn fast_malloc_sized(size: LInt) -> *mut LVoid {
    if size <= 0 {
        return ptr::null_mut();
    }
    let total = (size as usize) + std::mem::size_of::<usize>();
    let layout = Layout::from_size_align(total, std::mem::align_of::<usize>()).unwrap_or(Layout::new::<u8>());
    let p = alloc(layout);
    if p.is_null() {
        return ptr::null_mut();
    }
    *(p as *mut usize) = size as usize;
    p.add(std::mem::size_of::<usize>()) as *mut LVoid
}

unsafe fn fast_free_sized(data: *mut LVoid) {
    if data.is_null() {
        return;
    }
    let size_ptr = (data as *mut u8).sub(std::mem::size_of::<usize>());
    let size = *(size_ptr as *const usize);
    let total = size + std::mem::size_of::<usize>();
    let layout = Layout::from_size_align(total, std::mem::align_of::<usize>()).unwrap_or(Layout::new::<u8>());
    dealloc(size_ptr as *mut u8, layout);
}

/// ContainAddress: return whether addr is within the pool's buffer.
pub unsafe fn contain_address(addr: *mut LVoid, mempool: *mut LVoid) -> LBool {
    if mempool.is_null() || addr.is_null() {
        return 0;
    }
    let pool = &*(mempool as *const BoyiaMemoryPool);
    let i_addr = addr as LUintPtr;
    let base = pool.mAddress as LUintPtr;
    if i_addr >= base && i_addr < base + (pool.mSize as LUintPtr) {
        1
    } else {
        0
    }
}

/// InitMemoryPool: create a new memory pool of given size.
pub unsafe fn init_memory_pool(size: LInt) -> *mut LVoid {
    if size <= 0 {
        return ptr::null_mut();
    }
    let pool = fast_malloc_sized(std::mem::size_of::<BoyiaMemoryPool>() as LInt) as *mut BoyiaMemoryPool;
    if pool.is_null() {
        return ptr::null_mut();
    }
    (*pool).mSize = size;
    (*pool).mUsed = 0;
    let layout = Layout::from_size_align(size as usize, K_MEMORY_ALIGN_NUM).unwrap_or(Layout::new::<u8>());
    (*pool).mAddress = alloc(layout) as *mut LByte;
    if (*pool).mAddress.is_null() {
        fast_free_sized(pool as *mut LVoid);
        return ptr::null_mut();
    }
    (*pool).mFirstBlock = ptr::null_mut();
    pool as *mut LVoid
}

/// FreeMemoryPool: free pool and its buffer.
pub unsafe fn free_memory_pool(mempool: *mut LVoid) {
    if mempool.is_null() {
        return;
    }
    let pool = &*(mempool as *const BoyiaMemoryPool);
    let layout = Layout::from_size_align(pool.mSize as usize, K_MEMORY_ALIGN_NUM).unwrap_or(Layout::new::<u8>());
    dealloc(pool.mAddress as *mut u8, layout);
    fast_free_sized(mempool);
}

/// NewData: allocate a block from the pool. Returns pointer to data (after header).
pub unsafe fn new_data(size: LInt, mempool: *mut LVoid) -> *mut LVoid {
    if mempool.is_null() || size <= 0 {
        return ptr::null_mut();
    }
    let size = mem_align(size);
    let pool = &mut *(mempool as *mut BoyiaMemoryPool);
    let malloc_size = (size as usize + K_MEMORY_HEADER_LEN) as LInt;
    if malloc_size > pool.mSize {
        return ptr::null_mut();
    }

    let mut p_header: *mut MemoryBlockHeader = ptr::null_mut();

    if pool.mFirstBlock.is_null() {
        p_header = addr_align(pool.mAddress) as *mut MemoryBlockHeader;
        (*p_header).mSize = size;
        (*p_header).mNext = ptr::null_mut();
        (*p_header).mPrevious = ptr::null_mut();
        (*p_header).mFlag = K_MEMORY_BLOCK_HEADER_FLAG;
        pool.mFirstBlock = p_header;
        pool.mUsed = malloc_size;
        return (p_header as *mut u8).add(K_MEMORY_HEADER_LEN) as *mut LVoid;
    }

    let max_address = pool.mAddress.add(pool.mSize as usize);
    let mut current = pool.mFirstBlock;

    let new_addr = addr_align(data_tail(current));
    if addr_delta(max_address, new_addr) >= malloc_size as LIntPtr {
        p_header = new_addr as *mut MemoryBlockHeader;
        (*p_header).mSize = size;
        (*p_header).mNext = current;
        (*p_header).mPrevious = ptr::null_mut();
        (*p_header).mFlag = K_MEMORY_BLOCK_HEADER_FLAG;
        (*current).mPrevious = p_header;
        pool.mFirstBlock = p_header;
        pool.mUsed += malloc_size;
        return (p_header as *mut u8).add(K_MEMORY_HEADER_LEN) as *mut LVoid;
    }

    while !current.is_null() {
        if (*current).mNext.is_null() {
            let new_addr = addr_align(pool.mAddress);
            if addr_delta(current as *mut LByte, new_addr) >= malloc_size as LIntPtr {
                p_header = new_addr as *mut MemoryBlockHeader;
                (*p_header).mSize = size;
                (*p_header).mPrevious = current;
                (*p_header).mNext = ptr::null_mut();
                (*p_header).mFlag = K_MEMORY_BLOCK_HEADER_FLAG;
                (*current).mNext = p_header;
                break;
            }
            return ptr::null_mut();
        }
        let new_addr = addr_align(data_tail((*current).mNext));
        if addr_delta(current as *mut LByte, new_addr) >= malloc_size as LIntPtr {
            p_header = new_addr as *mut MemoryBlockHeader;
            (*p_header).mSize = size;
            (*p_header).mPrevious = current;
            (*p_header).mNext = (*current).mNext;
            (*p_header).mFlag = K_MEMORY_BLOCK_HEADER_FLAG;
            (*(*current).mNext).mPrevious = p_header;
            (*current).mNext = p_header;
            break;
        }
        current = (*current).mNext;
    }

    if !p_header.is_null() {
        pool.mUsed += malloc_size;
        (p_header as *mut u8).add(K_MEMORY_HEADER_LEN) as *mut LVoid
    } else {
        ptr::null_mut()
    }
}

/// DeleteData: free a block previously returned by NewData.
pub unsafe fn delete_data(data: *mut LVoid, mempool: *mut LVoid) {
    if data.is_null() || mempool.is_null() {
        return;
    }
    let pool = &mut *(mempool as *mut BoyiaMemoryPool);
    let p_header = (data as *mut u8).sub(K_MEMORY_HEADER_LEN) as *mut MemoryBlockHeader;
    if (p_header as LUintPtr) < (pool.mAddress as LUintPtr) {
        return;
    }
    if (*p_header).mFlag != K_MEMORY_BLOCK_HEADER_FLAG {
        return;
    }
    if !(*p_header).mNext.is_null() {
        (*(*p_header).mNext).mPrevious = (*p_header).mPrevious;
    }
    if pool.mFirstBlock == p_header {
        pool.mFirstBlock = (*p_header).mNext;
    } else {
        if !(*p_header).mNext.is_null() {
            (*(*p_header).mPrevious).mNext = (*p_header).mNext;
        } else {
            (*(*p_header).mPrevious).mNext = ptr::null_mut();
        }
    }
    pool.mUsed -= K_MEMORY_HEADER_LEN as LInt + (*p_header).mSize;
}

/// GetUsedMemory: return bytes used in pool.
pub unsafe fn get_used_memory(mempool: *mut LVoid) -> LInt {
    if mempool.is_null() {
        return 0;
    }
    (*(mempool as *const BoyiaMemoryPool)).mUsed
}

/// PrintPoolSize: no-op (C++ only logged).
pub unsafe fn print_pool_size(_mempool: *mut LVoid) {}

/// MigrateMemory: copy block from one pool to another, return new pointer.
pub unsafe fn migrate_memory(src_mem: *mut LVoid, _from_pool: *mut LVoid, to_pool: *mut LVoid) -> *mut LVoid {
    if src_mem.is_null() || to_pool.is_null() {
        return ptr::null_mut();
    }
    let src_header = (src_mem as *mut u8).sub(K_MEMORY_HEADER_LEN) as *const MemoryBlockHeader;
    let size = (*src_header).mSize as usize;
    let dest_mem = new_data((*src_header).mSize, to_pool);
    if dest_mem.is_null() {
        return ptr::null_mut();
    }
    ptr::copy_nonoverlapping(src_mem as *const u8, dest_mem as *mut u8, size);
    dest_mem
}

// ---------------------------------------------------------------------------
// Memory cache
// ---------------------------------------------------------------------------

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

/// CreateMemoryCache: fixed-size chunk pool.
pub unsafe fn create_memory_cache(type_size: LInt, capacity: LInt) -> *mut LVoid {
    if type_size <= 0 || capacity <= 0 {
        return ptr::null_mut();
    }
    let total = (type_size as usize) * (capacity as usize);
    let cache = fast_malloc_sized(std::mem::size_of::<MemoryCache>() as LInt) as *mut MemoryCache;
    if cache.is_null() {
        return ptr::null_mut();
    }
    let layout_buf = Layout::from_size_align(total, std::mem::align_of::<*mut LVoid>()).unwrap_or(Layout::new::<u8>());
    (*cache).mCacheAddr = alloc(layout_buf) as *mut LVoid;
    if (*cache).mCacheAddr.is_null() {
        fast_free_sized(cache as *mut LVoid);
        return ptr::null_mut();
    }
    ptr::write_bytes((*cache).mCacheAddr as *mut u8, 0, total);
    let chunk_len = std::mem::size_of::<MemoryChunk>() * capacity as usize;
    let layout_chunk = Layout::from_size_align(chunk_len, std::mem::align_of::<MemoryChunk>()).unwrap_or(Layout::new::<u8>());
    (*cache).mChunkCache = alloc(layout_chunk) as *mut MemoryChunk;
    if (*cache).mChunkCache.is_null() {
        dealloc((*cache).mCacheAddr as *mut u8, layout_buf);
        fast_free_sized(cache as *mut LVoid);
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

/// DestroyMemoryCache: free cache and its buffers.
pub unsafe fn destroy_memory_cache(cache_ptr: *mut LVoid) {
    if cache_ptr.is_null() {
        return;
    }
    let cache = &*(cache_ptr as *const MemoryCache);
    let total = (cache.mUnitSize as usize) * (cache.mCapacity as usize);
    let chunk_len = std::mem::size_of::<MemoryChunk>() * (cache.mCapacity as usize);
    let layout_buf = Layout::from_size_align(total, std::mem::align_of::<*mut LVoid>()).unwrap_or(Layout::new::<u8>());
    let layout_chunk = Layout::from_size_align(chunk_len, std::mem::align_of::<MemoryChunk>()).unwrap_or(Layout::new::<u8>());
    dealloc(cache.mCacheAddr as *mut u8, layout_buf);
    dealloc(cache.mChunkCache as *mut u8, layout_chunk);
    fast_free_sized(cache_ptr);
}

/// GetUsedChunkCount: number of chunks currently allocated.
pub unsafe fn get_used_chunk_count(cache_ptr: *mut LVoid) -> LInt {
    if cache_ptr.is_null() {
        return 0;
    }
    (*(cache_ptr as *const MemoryCache)).mCount
}
