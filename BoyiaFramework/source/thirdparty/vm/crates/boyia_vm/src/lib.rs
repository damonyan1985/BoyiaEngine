//! Boyia VM Rust Implementation
//! Pure Rust API; no C/FFI exports. All logic lives in `core`.
//!
//! Platform types (`LInt`, `LIntPtr`, `LUintPtr`, `LVoid`, `LByte`, `LInt8`, `LUint8`, [`LBool`],
//! [`LFalse`], [`LTrue`], [`K_BOYIA_NULL`]) are exported from this crate (`pub use types::*`). They
//! are defined in `boyia_types` so `boyia_memory` can share them without depending on `boyia_vm`.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod global;
mod types;
mod compile;
mod core;
mod inlinecache;
mod execute;

// Re-export types for crate users (includes Runtime trait).
pub use types::*;
pub use global::{Global, GlobalList};

// Re-export memory API from boyia_memory for backward compatibility.
pub use boyia_memory::{
    alloc_memory_chunk,
    contain_address,
    create_memory_cache,
    delete_data,
    destroy_memory_cache,
    fast_free,
    fast_malloc,
    free_memory_chunk,
    free_memory_pool,
    get_used_chunk_count,
    get_used_memory,
    init_memory_pool,
    migrate_memory,
    new_data,
    print_pool_size,
    BoyiaMemoryPool,
    MemoryBlockHeader,
};

// Re-export core VM API (Rust-only, no extern "C").
pub use core::{
    value_copy,
    set_native_result,
    get_native_result,
    get_native_helper_result,
    get_local_size,
    get_local_value,
    get_callee_and_captures_from_locals,
    local_push,
    get_local_stack,
    get_global_table,
    get_vm_creator,
    set_int_result,
    init_vm,
    destroy_vm,
    load_string_table,
    load_instructions,
    load_entry_table,
    compile_code,
    create_object,
    cache_vm_code,
    copy_object,
    native_call_impl,
    native_call_by_index,
    get_runtime_from_vm,
    get_boyia_class_id,
    get_function_count,
    vector_params_grow_if_full,
    gen_identifier_from_str,
    create_global_class,
    alloc_builtin_function,
    get_string_buffer,
    get_string_buffer_from_body,
    get_string_hash,
    gen_hash_code,
    create_string_object,
    create_const_string,
    create_native_string,
    create_micro_task,
    resume_micro_task,
    consume_micro_task,
    iterate_micro_task,
};
pub use execute::{execute_code, execute_global_code};
