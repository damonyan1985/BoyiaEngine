//! Test that init_vm and destroy_vm run without crashing.

use boyia_vm::{destroy_vm, init_vm};
use std::ptr;

#[test]
fn test_init_destroy_vm_null_creator() {
    unsafe {
        let vm = init_vm(ptr::null_mut());
        assert!(!vm.is_null());
        destroy_vm(vm);
    }
}