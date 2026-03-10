//! Boyia memory. Strict port of BoyiaMemory.cpp / BoyiaMemory.h.
//! FastMalloc/FastFree, InitMemoryPool, NewData, DeleteData;
//! CreateMemoryCache, AllocMemoryChunk, FreeMemoryChunk, DestroyMemoryCache.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod chunk;
mod memory;

// Type aliases matching PlatformLib.h / boyia_vm for ABI compatibility
pub type LInt = std::os::raw::c_int;
pub type LIntPtr = isize;
pub type LUintPtr = usize;
pub type LVoid = std::ffi::c_void;
pub type LByte = u8;
pub type LBool = LInt;

// ---------------------------------------------------------------------------
// Memory pool (memory.rs)
// ---------------------------------------------------------------------------
pub use memory::{
    contain_address, delete_data, fast_free, fast_malloc, free_memory_pool, get_used_memory,
    init_memory_pool, migrate_memory, new_data, print_pool_size, BoyiaMemoryPool, MemoryBlockHeader,
};

// ---------------------------------------------------------------------------
// Memory chunk cache (chunk.rs)
// ---------------------------------------------------------------------------
pub use chunk::{
    alloc_memory_chunk, create_memory_cache, destroy_memory_cache, free_memory_chunk,
    get_used_chunk_count,
};
