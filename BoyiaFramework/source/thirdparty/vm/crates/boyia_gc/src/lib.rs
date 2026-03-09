//! Boyia GC. Public API only; implementation in gc.rs.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod gc;

pub use gc::{BoyiaGc, create_gc, destroy_gc, gc_append_ref, gc_collect_garbage};
