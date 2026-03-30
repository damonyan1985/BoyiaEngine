//! Platform type aliases matching PlatformLib.h / C++ Boyia types.
//!
//! **Application code should import these via [`boyia_vm`]** (e.g. `boyia_vm::LInt`),
//! not depend on this crate directly. This crate exists only so `boyia_memory` can share
//! the same definitions without a circular dependency on `boyia_vm`.

#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]

pub type LInt8 = std::os::raw::c_char;
pub type LInt = std::os::raw::c_int;
pub type LUint8 = u8;
pub type LUintPtr = usize;
pub type LIntPtr = isize;
pub type LVoid = std::ffi::c_void;
pub type LByte = u8;

/// C++ `kBoyiaNull`: null stored in integer-backed fields (e.g. `mSuper`, `mIntVal` as handle). Prefer via `boyia_vm::K_BOYIA_NULL`.
pub const K_BOYIA_NULL: LIntPtr = 0;

/// Boolean type for VM / repr(C) structs; `LFalse` = 0, `LTrue` = 1. Matches `LInt` width (`repr(i32)`).
#[repr(i32)]
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum LBool {
    LFalse = 0,
    LTrue = 1,
}

/// Same as [`LBool::LFalse`].
pub const LFalse: LBool = LBool::LFalse;
/// Same as [`LBool::LTrue`].
pub const LTrue: LBool = LBool::LTrue;
