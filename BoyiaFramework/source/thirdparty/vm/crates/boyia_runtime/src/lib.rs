//! Boyia runtime: VM lifecycle, native function table, init and execution.
//! Rust port of BoyiaRuntime.cpp (without platform/UI/GC; stubs where needed).

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod id_creator;
mod runtime;

pub use id_creator::IdCreator;
pub use runtime::BoyiaRuntime;
