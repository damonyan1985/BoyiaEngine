//! String-to-id generator for builtin and native names (replaces util::IDCreator).
//! Reserved ids 1..=6 for BuiltinId (this, super, String, Array, Map, MicroTask) per BoyiaValue.h.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use boyia_vm::{BoyiaStr, LUintPtr};

/// Builtin name -> BuiltinId (BoyiaValue.h). Order must match BuiltinId enum.
const BUILTIN_NAMES: [&str; 6] = ["this", "super", "String", "Array", "Map", "MicroTask"];

/// String-to-id generator. Reserved ids 1..=6 for BuiltinId.
#[derive(Default)]
pub struct IdCreator {
    next_id: u64,
    map: std::collections::HashMap<String, LUintPtr>,
}

impl IdCreator {
    pub fn new() -> Self {
        let mut map = std::collections::HashMap::new();
        for (i, name) in BUILTIN_NAMES.iter().enumerate() {
            map.insert((*name).to_owned(), (i + 1) as LUintPtr);
        }
        Self {
            next_id: BUILTIN_NAMES.len() as u64,
            map,
        }
    }

    /// Get or assign id for a string key. Reserved names (this, super, String, Array, Map, MicroTask) return BuiltinId (1..6).
    pub fn gen_ident_by_str(&mut self, key: &str) -> LUintPtr {
        if let Some(&id) = self.map.get(key) {
            return id;
        }
        self.next_id += 1;
        let id = self.next_id as LUintPtr;
        self.map.insert(key.to_owned(), id);
        id
    }

    pub fn get_id(&self, key: &str) -> Option<LUintPtr> {
        self.map.get(key).copied()
    }

    /// Get or assign id for a string from VM (BoyiaStr). Used by builtins (e.g. Map key).
    pub fn gen_ident_by_boyia_str(&mut self, s: *const BoyiaStr) -> LUintPtr {
        if s.is_null() {
            return 0;
        }
        let s = unsafe { &*s };
        let len = s.mLen.max(0) as usize;
        let slice = unsafe { std::slice::from_raw_parts(s.mPtr as *const u8, len) };
        let key = std::str::from_utf8(slice).unwrap_or("");
        self.gen_ident_by_str(key)
    }
}
