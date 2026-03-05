//! Boyia runtime: VM lifecycle, native function table, init and execution.
//! Rust port of BoyiaRuntime.cpp (without platform/UI/GC; stubs where needed).

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use boyia_builtins::{builtin_map_class, builtin_micro_task_class, builtin_string_class};
use boyia_vm::{
    cache_vm_code, compile_code, consume_micro_task, create_global_class, execute_global_code,
    init_vm, BuiltinId,
    BoyiaStr, LUintPtr, LInt, LVoid, NativeFunction, NativePtr, OpHandleResult, Runtime,
};

const K_NATIVE_FUNCTION_CAPACITY: usize = 100;

/// Builtin name -> BuiltinId (BoyiaValue.h). Order must match BuiltinId enum.
const BUILTIN_NAMES: [&str; 6] = ["this", "super", "String", "Array", "Map", "MicroTask"];

/// String-to-id generator for builtin and native names (replaces util::IDCreator).
/// Reserved ids 1..=6 for BuiltinId (this, super, String, Array, Map, MicroTask) per BoyiaValue.h.
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

/// Runtime state: VM + native table + id creator. Matches BoyiaRuntime.cpp responsibilities.
pub struct BoyiaRuntime {
    /// VM instance (creator set to self for native dispatch).
    vm: *mut LVoid,
    /// Native function table: (name_key, ptr). Terminated by mAddr == null (we use 0 index as sentinel or check length).
    native_fun_table: Vec<NativeFunction>,
    id_creator: IdCreator,
    /// Whether VM code was loaded from file (we don't implement file load; always false).
    is_load_exe_file: bool,
}

impl BoyiaRuntime {
    /// Create runtime; VM is created in `init()` with `self` as creator. No global dispatchers.
    /// After `init()`, the runtime must not be moved (e.g. use `Box<BoyiaRuntime>`) so that the VM's mCreator stays valid.
    pub fn new() -> Self {
        Self {
            vm: ptr::null_mut(),
            native_fun_table: Vec::with_capacity(K_NATIVE_FUNCTION_CAPACITY),
            id_creator: IdCreator::new(),
            is_load_exe_file: false,
        }
    }

    /// Initialize: create VM with `self` as creator (like C++ `InitVM(this)`), register builtin ids, native table, classes. Call after new().
    pub fn init(&mut self) {
        eprintln!("[init] 1 init_vm");
        self.vm = unsafe { init_vm(self as &mut dyn Runtime as *mut dyn Runtime) };
        if self.vm.is_null() {
            eprintln!("[init] ERROR init_vm returned null");
            return;
        }
        eprintln!("[init] 2 builtin ids (BuiltinId 1..6 reserved)");
        // Ensure builtin names are registered so compile/lookup use same keys as CreateGlobalClass
        let _ = self.id_creator.gen_ident_by_str("this");
        let _ = self.id_creator.gen_ident_by_str("super");
        let _ = self.id_creator.gen_ident_by_str("String");
        let _ = self.id_creator.gen_ident_by_str("Array");
        let _ = self.id_creator.gen_ident_by_str("Map");
        let _ = self.id_creator.gen_ident_by_str("MicroTask");

        eprintln!("[init] 3 init_native_function");
        self.init_native_function();

        eprintln!("[init] 4 builtin_string_class");
        // Builtin classes: use BuiltinId keys per BoyiaValue.h (CreateGlobalClass(kBoyiaString, vm) etc.)
        let mut gen_id = |s: &str| self.id_creator.gen_ident_by_str(s);
        builtin_string_class(self.vm, &mut gen_id);
        eprintln!("[init] 5 builtin_map_class");
        builtin_map_class(self.vm, &mut gen_id);
        eprintln!("[init] 6 builtin_micro_task_class");
        builtin_micro_task_class(self.vm, &mut gen_id);

        eprintln!("[init] 8 done");
    }

    /// Minimal init for tests: VM + natives + dispatcher only (no builtin classes).
    #[doc(hidden)]
    pub fn init_minimal_for_test(&mut self) {
        self.vm = unsafe { init_vm(self as &mut dyn Runtime as *mut dyn Runtime) };
        if self.vm.is_null() {
            return;
        }
        self.id_creator.gen_ident_by_str("this");
        self.id_creator.gen_ident_by_str("String");
        self.init_native_function();
    }

    fn init_native_function(&mut self) {
        let keys: [LUintPtr; 8] = [
            self.id_creator.gen_ident_by_str("new"),
            self.id_creator.gen_ident_by_str("BY_GetFromArray"),
            self.id_creator.gen_ident_by_str("BY_AddInArray"),
            self.id_creator.gen_ident_by_str("BY_GetArraySize"),
            self.id_creator.gen_ident_by_str("BY_ClearArray"),
            self.id_creator.gen_ident_by_str("BY_RemoveWidthIndex"),
            self.id_creator.gen_ident_by_str("BY_RemoveFromArray"),
            self.id_creator.gen_ident_by_str("BY_Log"),
        ];
        self.append_native(keys[0], boyia_lib::create_object as NativePtr);
        self.append_native(keys[1], boyia_lib::get_element_from_vector as NativePtr);
        self.append_native(keys[2], boyia_lib::add_element_to_vector as NativePtr);
        self.append_native(keys[3], boyia_lib::get_vector_size as NativePtr);
        self.append_native(keys[4], boyia_lib::clear_vector as NativePtr);
        self.append_native(keys[5], boyia_lib::remove_element_width_index as NativePtr);
        self.append_native(keys[6], boyia_lib::remove_element_from_vector as NativePtr);
        self.append_native(keys[7], boyia_lib::log_print as NativePtr);
        self.append_native_sentinel();
    }

    fn append_native(&mut self, id: LUintPtr, ptr: NativePtr) {
        if self.native_fun_table.len() < self.native_fun_table.capacity() {
            self.native_fun_table.push(NativeFunction {
                mNameKey: id,
                mAddr: ptr,
            });
        }
    }

    fn append_native_sentinel(&mut self) {
        self.native_fun_table.push(NativeFunction {
            mNameKey: 0,
            mAddr: sentinel_native as NativePtr,
        });
    }

    /// Compile script into the VM.
    pub fn compile(&self, script: &str) {
        let script_c = std::ffi::CString::new(script).unwrap_or_default();
        unsafe {
            compile_code(script_c.as_ptr() as *mut _, self.vm);
        }
    }

    /// VM pointer for use with boyia_vm APIs.
    pub fn vm(&self) -> *mut LVoid {
        self.vm
    }

    /// Id creator for string keys.
    pub fn id_creator(&mut self) -> &mut IdCreator {
        &mut self.id_creator
    }

    /// Whether VM code was loaded from exe file (we always return false).
    pub fn is_load_exe_file(&self) -> bool {
        self.is_load_exe_file
    }

    /// Run global code (entry table). Match ExecuteGlobalCode.
    pub fn run_exe_file(&self) {
        unsafe {
            execute_global_code(self.vm);
        }
    }

    /// Cache VM code (patch instructions). Match CacheVMCode.
    pub fn cache_code(&self) {
        unsafe {
            cache_vm_code(self.vm);
        }
    }

    /// Consume micro tasks in the queue.
    pub fn consume_micro_task(&self) {
        unsafe {
            consume_micro_task(self.vm);
        }
    }
}

impl Runtime for BoyiaRuntime {
    fn find_native_func(&self, key: LUintPtr) -> LInt {
        for (idx, nf) in self.native_fun_table.iter().enumerate() {
            if nf.mNameKey == 0 || nf.mAddr as *const () == sentinel_native as *const () {
                break;
            }
            if nf.mNameKey == key {
                return idx as LInt;
            }
        }
        -1
    }

    fn call_native_function(&self, idx: LInt) -> LInt {
        if idx < 0 || idx as usize >= self.native_fun_table.len() {
            return OpHandleResult::kOpResultEnd as i32;
        }
        let nf = &self.native_fun_table[idx as usize];
        if nf.mAddr as *const () == sentinel_native as *const () {
            return OpHandleResult::kOpResultEnd as i32;
        }
        unsafe { (nf.mAddr)(self.vm) }
    }

    fn gen_identifier(&mut self, key: &str) -> LUintPtr {
        self.id_creator.gen_ident_by_str(key)
    }

    fn gen_ident_by_str(&mut self, s: *const BoyiaStr) -> LUintPtr {
        self.id_creator.gen_ident_by_boyia_str(s)
    }
}

impl Default for BoyiaRuntime {
    fn default() -> Self {
        Self::new()
    }
}

impl Drop for BoyiaRuntime {
    fn drop(&mut self) {
        unsafe {
            boyia_vm::destroy_vm(self.vm);
        }
        self.vm = ptr::null_mut();
    }
}

use std::ptr;

/// Sentinel: end of native table (never called with valid idx).
unsafe extern "C" fn sentinel_native(_vm: *mut LVoid) -> LInt {
    OpHandleResult::kOpResultEnd as i32
}
