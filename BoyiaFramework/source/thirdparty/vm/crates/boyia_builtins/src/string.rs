//! String builtin class: buffer, hash props; length, equal methods.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::gen_builtin_class_function;
use boyia_vm::{
    create_global_class, get_local_size, get_local_value, get_string_buffer, get_string_hash,
    set_native_result, BoyiaFunction, BoyiaStr, BoyiaValue, K_BOYIA_NULL, NativePtr, RealValue,
    ValueType, LIntPtr, LUintPtr, LVoid, OpHandleResult,
};
use std::ptr;

fn str_eq(a: *const BoyiaStr, b: *const BoyiaStr) -> bool {
    if a.is_null() || b.is_null() {
        return a.is_null() && b.is_null();
    }
    let a = unsafe { &*a };
    let b = unsafe { &*b };
    if a.mLen != b.mLen {
        return false;
    }
    if a.mPtr == b.mPtr {
        return true;
    }
    let len = a.mLen.max(0) as usize;
    for i in 0..len {
        if unsafe { *a.mPtr.add(i) } != unsafe { *b.mPtr.add(i) } {
            return false;
        }
    }
    true
}

unsafe fn string_length_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let str_ref = get_string_buffer(obj);
    let len = if str_ref.is_null() {
        0
    } else {
        (*str_ref).mLen
    };
    let mut value = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue { mIntVal: len as LIntPtr },
    };
    set_native_result(&mut value, vm);
    OpHandleResult::kOpResultSuccess
}

unsafe fn string_equal_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let cmp_val = get_local_value(1, vm) as *const BoyiaValue;
    let str_a = get_string_buffer(obj);
    let str_b = get_string_buffer(cmp_val);
    let hash_a = get_string_hash(obj);
    let hash_b = get_string_hash(cmp_val);
    let eq = hash_a == hash_b && str_eq(str_a, str_b);
    let mut value = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue {
            mIntVal: if eq { 1 } else { 0 },
        },
    };
    set_native_result(&mut value, vm);
    OpHandleResult::kOpResultSuccess
}

/// Register String builtin class: buffer, hash props; length, equal methods.
pub fn builtin_string_class<F>(vm: *mut LVoid, gen_id: &mut F)
where
    F: FnMut(&str) -> LUintPtr,
{
    eprintln!("[builtin_string_class] 1");
    if vm.is_null() {
        eprintln!("[builtin_string_class] vm null");
        return;
    }
    let string_key = gen_id("String");
    eprintln!("[builtin_string_class] 2 create_global_class string_key={}", string_key);
    let class_ref = unsafe { create_global_class(string_key, vm) } as *mut BoyiaValue;
    if class_ref.is_null() {
        eprintln!("[builtin_string_class] class_ref null");
        return;
    }
    eprintln!("[builtin_string_class] 3 class_ref ok");
    unsafe {
        (*class_ref).mValue.mObj.mSuper = K_BOYIA_NULL;
        let class_body = (*class_ref).mValue.mObj.mPtr as *mut BoyiaFunction;
        eprintln!("[builtin_string_class] 4 class_body={:?}", class_body);
        if class_body.is_null() || (*class_body).mParams.is_null() {
            eprintln!("[builtin_string_class] class_body or mParams null");
            return;
        }
        eprintln!("[builtin_string_class] 5 buffer prop");
        // prop: buffer
        let buf_slot = (*class_body).mParams.add((*class_body).mParamSize as usize);
        (*buf_slot).mValueType = ValueType::BY_STRING;
        (*buf_slot).mNameKey = gen_id("buffer");
        (*buf_slot).mValue.mStrVal.mPtr = ptr::null_mut();
        (*buf_slot).mValue.mStrVal.mLen = 0;
        (*class_body).mParamSize += 1;
        eprintln!("[builtin_string_class] 6 hash prop");
        // prop: hash
        let hash_slot = (*class_body).mParams.add((*class_body).mParamSize as usize);
        (*hash_slot).mValueType = ValueType::BY_INT;
        (*hash_slot).mNameKey = gen_id("hash");
        (*hash_slot).mValue.mIntVal = 0;
        (*class_body).mParamSize += 1;
        eprintln!("[builtin_string_class] 7 length");
        gen_builtin_class_function(gen_id("length"), string_length_impl as NativePtr, class_body, vm);
        eprintln!("[builtin_string_class] 8 equal");
        gen_builtin_class_function(gen_id("equal"), string_equal_impl as NativePtr, class_body, vm);
    }
    eprintln!("[builtin_string_class] 9 done");
}
