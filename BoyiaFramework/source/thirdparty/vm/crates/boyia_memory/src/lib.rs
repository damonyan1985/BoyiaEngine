//! Boyia memory. Strict port of BoyiaMemory.cpp / BoyiaMemory.h.
//! FastMalloc/FastFree, InitMemoryPool, NewData, DeleteData;
//! CreateMemoryCache, AllocMemoryChunk, FreeMemoryChunk, DestroyMemoryCache.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod chunk;
mod memory;

// Same types as `boyia_vm` (via `boyia_types`). Prefer `boyia_vm::…` in new code.
pub use boyia_types::{
    K_BOYIA_NULL, LBool, LByte, LFalse, LInt, LInt8, LIntPtr, LTrue, LUint8, LUintPtr, LVoid,
};

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
