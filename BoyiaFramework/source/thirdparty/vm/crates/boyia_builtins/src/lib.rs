//! Builtin classes and methods (String, Map, MicroTask, Array, Json).
//! Port of BoyiaValue.cpp builtins: GenBuiltinClassFunction, BuiltinStringClass, BuiltinMapClass, BuiltinMicroTaskClass, etc.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod array;
mod json;
mod map;
mod microtask;
mod string;

use boyia_vm::{
    alloc_builtin_function, BoyiaFunction, NativePtr, ValueType, LIntPtr, LUintPtr, LVoid,
};

// ---------------------------------------------------------------------------
// Helpers: gen_builtin_class_function, gen_builtin_class_prop_function
// ---------------------------------------------------------------------------

/// Add a builtin method to a class body (BY_NAV_FUNC). Match GenBuiltinClassFunction.
pub unsafe fn gen_builtin_class_function(
    key: LUintPtr,
    func: NativePtr,
    class_body: *mut BoyiaFunction,
    vm: *mut LVoid,
) {
    eprintln!("[gen_builtin_class_function] key={}", key);
    if class_body.is_null() || vm.is_null() {
        eprintln!("[gen_builtin_class_function] null class_body or vm");
        return;
    }
    let method_fn = alloc_builtin_function(vm, func);
    if method_fn.is_null() {
        eprintln!("[gen_builtin_class_function] alloc_builtin_function returned null");
        return;
    }
    let put_func_val = (*class_body).mParams.add((*class_body).mParamSize as usize);
    (*put_func_val).mValueType = ValueType::BY_NAV_FUNC;
    (*put_func_val).mNameKey = key;
    (*put_func_val).mValue.mObj.mPtr = method_fn as LIntPtr;
    (*class_body).mParamSize += 1;
}

/// Add a builtin property function to a class body (BY_NAV_PROP). Match GenBuiltinClassPropFunction.
pub unsafe fn gen_builtin_class_prop_function(
    key: LUintPtr,
    func: NativePtr,
    class_body: *mut BoyiaFunction,
    vm: *mut LVoid,
) {
    if class_body.is_null() || vm.is_null() {
        return;
    }
    let method_fn = alloc_builtin_function(vm, func);
    if method_fn.is_null() {
        return;
    }
    let put_func_val = (*class_body).mParams.add((*class_body).mParamSize as usize);
    (*put_func_val).mValueType = ValueType::BY_NAV_PROP;
    (*put_func_val).mNameKey = key;
    (*put_func_val).mValue.mObj.mPtr = method_fn as LIntPtr;
    (*put_func_val).mValue.mObj.mSuper = class_body as LIntPtr;
    (*class_body).mParamSize += 1;
}

// ---------------------------------------------------------------------------
// String builtin (string.rs)
// ---------------------------------------------------------------------------
pub use string::builtin_string_class;

// ---------------------------------------------------------------------------
// Map builtin (map.rs)
// ---------------------------------------------------------------------------
pub use map::{builtin_map_class, create_map_object};

// ---------------------------------------------------------------------------
// MicroTask builtin (microtask.rs)
// ---------------------------------------------------------------------------
pub use microtask::{builtin_micro_task_class, create_micro_task_object};

// ---------------------------------------------------------------------------
// Array builtin (array.rs)
// ---------------------------------------------------------------------------
pub use array::{builtin_array_class, create_array_object};

// ---------------------------------------------------------------------------
// Json builtin (json.rs)
// ---------------------------------------------------------------------------
pub use json::builtin_json_class;
