//! Test that init_vm and destroy_vm run without crashing.

use boyia_vm::{destroy_vm, init_vm, BoyiaStr, BoyiaValue, Runtime, ValueType};
use std::ptr;

struct TestRuntime;

impl Runtime for TestRuntime {
    fn find_native_func(&self, _key: usize) -> i32 {
        -1
    }
    fn call_native_function(&self, _idx: i32) -> i32 {
        0
    }
    fn gen_identifier(&mut self, _key: &str) -> usize {
        0
    }
    fn gen_ident_by_str(&mut self, _s: *const BoyiaStr) -> usize {
        0
    }
    fn new_data(&self, _size: i32) -> *mut std::ffi::c_void {
        ptr::null_mut()
    }
    fn delete_data(&self, _data: *mut std::ffi::c_void) {}
    fn memory_pool(&self) -> *mut std::ffi::c_void {
        ptr::null_mut()
    }
    fn create_runtime_to_memory(&self, _vm: *mut std::ffi::c_void) -> *mut std::ffi::c_void {
        ptr::null_mut()
    }
    fn update_runtime_memory(&mut self, _to_pool: *mut std::ffi::c_void, _vm: *mut std::ffi::c_void) {}
    fn gc_append_ref(&self, _address: *mut std::ffi::c_void, _type: ValueType) {}
    fn gc_ptr(&self) -> *mut std::ffi::c_void {
        ptr::null_mut()
    }
    fn vm_ptr(&self) -> *mut std::ffi::c_void {
        ptr::null_mut()
    }
    fn persistent_object(&mut self, _value: *const BoyiaValue) -> *mut boyia_vm::Global {
        ptr::null_mut()
    }
    fn iterate_persistent(&self, _f: &mut dyn FnMut(*mut boyia_vm::Global)) {}
    fn remove_persistent(&mut self, _ptr: *mut boyia_vm::Global) {}
}

#[test]
fn test_init_destroy_vm_null_pool() {
    unsafe {
        let mut rt = TestRuntime;
        let vm = init_vm(&mut rt as *mut dyn Runtime);
        assert!(!vm.is_null());
        destroy_vm(vm);
    }
}