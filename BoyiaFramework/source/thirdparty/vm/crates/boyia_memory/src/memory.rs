//! Memory pool. BoyiaMemoryPool, FastMalloc/FastFree, NewData/DeleteData, MigrateMemory.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::{LByte, LInt, LIntPtr, LUintPtr, LVoid};
use std::alloc::{alloc, dealloc, Layout};
use std::ptr;

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

// ---------------------------------------------------------------------------
// FastMalloc / FastFree
// ---------------------------------------------------------------------------
// fast_malloc(size) 实际分配 total = size + size_of::<usize>() 字节，布局为：
//   [ prefix (usize) | data (size bytes) ] 返回 data 指针。
// fast_free(data) 从 data 反推 block 起始与 size，用相同 Layout dealloc。
// ---------------------------------------------------------------------------

/// FastMalloc: 使用 Layout 分配，在块首存 size 供 fast_free 使用。
pub unsafe fn fast_malloc(size: LInt) -> *mut LVoid {
    if size <= 0 {
        return ptr::null_mut();
    }
    let size_usize = size as usize;
    let total = size_usize
        .checked_add(std::mem::size_of::<usize>())
        .unwrap_or(0);
    if total == 0 {
        return ptr::null_mut();
    }
    let layout = Layout::from_size_align(total, std::mem::align_of::<usize>())
        .unwrap_or(Layout::new::<u8>());
    let p = alloc(layout);
    if p.is_null() {
        return ptr::null_mut();
    }
    *(p as *mut usize) = size_usize;
    p.add(std::mem::size_of::<usize>()) as *mut LVoid
}

/// FastFree: 从 data 反推 block 起始与 size，用相同 Layout 调用 dealloc。
pub unsafe fn fast_free(data: *mut LVoid) {
    if data.is_null() {
        return;
    }
    let size_ptr = (data as *mut u8).sub(std::mem::size_of::<usize>());
    let size_usize = *(size_ptr as *const usize);
    let total = size_usize
        .checked_add(std::mem::size_of::<usize>())
        .unwrap_or(0);
    if total == 0 {
        return;
    }
    let layout = Layout::from_size_align(total, std::mem::align_of::<usize>())
        .unwrap_or(Layout::new::<u8>());
    dealloc(size_ptr, layout);
}

/// ContainAddress: return whether addr is within the pool's buffer.
pub unsafe fn contain_address(addr: *mut LVoid, mempool: *mut LVoid) -> LInt {
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
    let pool = fast_malloc(std::mem::size_of::<BoyiaMemoryPool>() as LInt) as *mut BoyiaMemoryPool;
    if pool.is_null() {
        return ptr::null_mut();
    }
    (*pool).mAddress = fast_malloc(size) as *mut LByte;
    if (*pool).mAddress.is_null() {
        fast_free(pool as *mut LVoid);
        return ptr::null_mut();
    }
    (*pool).mSize = size;
    (*pool).mUsed = 0;
    (*pool).mFirstBlock = ptr::null_mut();
    pool as *mut LVoid
}

/// FreeMemoryPool: free pool and its buffer (FastFree only).
pub unsafe fn free_memory_pool(mempool: *mut LVoid) {
    if mempool.is_null() {
        return;
    }
    let pool = &*(mempool as *const BoyiaMemoryPool);
    fast_free(pool.mAddress as *mut LVoid);
    fast_free(mempool);
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
