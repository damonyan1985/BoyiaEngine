//! BoyiaRuntime: VM lifecycle, native function table, init and execution.
//! Rust port of BoyiaRuntime.cpp (without platform/UI/GC; stubs where needed).

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::id_creator::IdCreator;
use boyia_builtins::{builtin_array_class, builtin_map_class, builtin_micro_task_class, builtin_string_class};
use boyia_vm::{
    cache_vm_code, compile_code, delete_data, consume_micro_task, execute_global_code,
    free_memory_pool, init_memory_pool, init_vm, new_data,
    BoyiaStr, LUintPtr, LInt, LVoid, NativeFunction, NativePtr, OpHandleResult, Runtime,
};
use std::ptr;

const K_NATIVE_FUNCTION_CAPACITY: usize = 100;
/// Memory pool size (6 MB). Match BoyiaRuntime.cpp kMemoryPoolSize.
const K_MEMORY_POOL_SIZE: LInt = 6 * 1024 * 1024;

/// Runtime state: VM + native table + id creator + memory pool + GC. Matches BoyiaRuntime.cpp.
pub struct BoyiaRuntime {
    /// VM instance (creator set to self for native dispatch).
    vm: *mut LVoid,
    /// Memory pool for object allocation (BoyiaRuntime::m_memoryPool). Created in init(); freed in Drop after destroy_vm.
    memory_pool: *mut LVoid,
    /// GC state (BoyiaRuntime::m_gc). Created in init() after VM; destroyed in Drop before destroy_vm.
    gc: *mut boyia_gc::BoyiaGc,
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
            memory_pool: ptr::null_mut(),
            gc: ptr::null_mut(),
            native_fun_table: Vec::with_capacity(K_NATIVE_FUNCTION_CAPACITY),
            id_creator: IdCreator::new(),
            is_load_exe_file: false,
        }
    }

    /// Initialize: create memory pool (BoyiaMemory), then VM with `self` as creator (like C++ BoyiaRuntime ctor). Call after new().
    pub fn init(&mut self) {
        eprintln!("[init] 1 memory pool");
        self.memory_pool = unsafe { init_memory_pool(K_MEMORY_POOL_SIZE) };
        if self.memory_pool.is_null() {
            eprintln!("[init] ERROR init_memory_pool returned null");
            return;
        }
        eprintln!("[init] 2 init_vm");
        self.vm = unsafe { init_vm(self as &mut dyn Runtime as *mut dyn Runtime) };
        if self.vm.is_null() {
            eprintln!("[init] ERROR init_vm returned null");
            return;
        }
        eprintln!("[init] 2a create_gc");
        self.gc = unsafe { boyia_gc::create_gc(self.vm) };
        if self.gc.is_null() {
            eprintln!("[init] WARN create_gc returned null");
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
        eprintln!("[init] 7 builtin_array_class");
        builtin_array_class(self.vm, &mut gen_id);

        eprintln!("[init] 8 done");
    }

    /// Minimal init for tests: VM + natives + dispatcher only (no builtin classes).
    #[doc(hidden)]
    pub fn init_minimal_for_test(&mut self) {
        self.memory_pool = unsafe { init_memory_pool(K_MEMORY_POOL_SIZE) };
        if self.memory_pool.is_null() {
            return;
        }
        self.vm = unsafe { init_vm(self as &mut dyn Runtime as *mut dyn Runtime) };
        if self.vm.is_null() {
            return;
        }
        self.gc = unsafe { boyia_gc::create_gc(self.vm) };
        self.id_creator.gen_ident_by_str("this");
        self.id_creator.gen_ident_by_str("String");
        self.init_native_function();
    }

    fn init_native_function(&mut self) {
        let keys: [LUintPtr; 2] = [
            self.id_creator.gen_ident_by_str("new"),
            self.id_creator.gen_ident_by_str("BY_Log"),
        ];
        self.append_native(keys[0], boyia_lib::create_object as NativePtr);
        self.append_native(keys[1], boyia_lib::log_print as NativePtr);
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
    fn memory_pool(&self) -> *mut LVoid {
        self.memory_pool
    }

    fn gc_ptr(&self) -> *mut LVoid {
        self.gc as *mut LVoid
    }
    fn vm_ptr(&self) -> *mut LVoid {
        self.vm
    }

    fn gc_append_ref(&self, address: *mut LVoid, type_: boyia_vm::ValueType) {
        boyia_gc::gc_append_ref(address, type_, self);
    }

    fn create_runtime_to_memory(&self, _vm: *mut LVoid) -> *mut LVoid {
        unsafe { boyia_vm::init_memory_pool(K_MEMORY_POOL_SIZE) }
    }

    fn update_runtime_memory(&mut self, to_pool: *mut LVoid, _vm: *mut LVoid) {
        if to_pool.is_null() {
            return;
        }
        unsafe {
            if !self.memory_pool.is_null() {
                boyia_vm::free_memory_pool(self.memory_pool);
            }
            self.memory_pool = to_pool;
        }
    }

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
        unsafe { (nf.mAddr)(self.vm) as i32 }
    }

    fn gen_identifier(&mut self, key: &str) -> LUintPtr {
        self.id_creator.gen_ident_by_str(key)
    }

    fn gen_ident_by_str(&mut self, s: *const BoyiaStr) -> LUintPtr {
        self.id_creator.gen_ident_by_boyia_str(s)
    }

    fn new_data(&self, size: LInt) -> *mut LVoid {
        unsafe {
            if !self.gc.is_null() {
                boyia_gc::gc_collect_garbage(self.gc, self.vm);
            }
            new_data(size, self.memory_pool)
        }
    }

    fn delete_data(&self, data: *mut LVoid) {
        unsafe { delete_data(data, self.memory_pool) }
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
            if !self.gc.is_null() {
                boyia_gc::destroy_gc(self.gc);
                self.gc = ptr::null_mut();
            }
            boyia_vm::destroy_vm(self.vm);
            if !self.memory_pool.is_null() {
                free_memory_pool(self.memory_pool);
                self.memory_pool = ptr::null_mut();
            }
        }
        self.vm = ptr::null_mut();
    }
}

/// Sentinel: end of native table (never called with valid idx).
unsafe fn sentinel_native(_vm: *mut LVoid) -> OpHandleResult {
    OpHandleResult::kOpResultEnd
}
