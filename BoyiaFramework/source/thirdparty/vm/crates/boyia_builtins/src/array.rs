//! Array builtin class: get, add, size, clear, removeAt, remove.
//! Mirrors boyia_lib: get_element_from_vector, add_element_to_vector, get_vector_size, clear_vector, remove_element_width_index, remove_element_from_vector.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::gen_builtin_class_function;
use boyia_vm::{
    copy_object, create_global_class, get_function_count, get_local_size, get_local_value,
    compare_value, set_int_result, set_native_result, value_copy, vector_params_grow_if_full,
    BoyiaFunction, BoyiaValue, K_BOYIA_NULL, NativePtr, ValueType, LUintPtr, LVoid, OpHandleResult,
};

/// Compare two BoyiaValue for equality (match compareValue in BoyiaLib.cpp).
// unsafe fn compare_value(src: *const BoyiaValue, dest: *const BoyiaValue) -> bool {
//     if src.is_null() || dest.is_null() {
//         return false;
//     }
//     if (*src).mValueType != (*dest).mValueType {
//         return false;
//     }
//     match (*src).mValueType {
//         ValueType::BY_CHAR | ValueType::BY_INT | ValueType::BY_NAVCLASS => {
//             (*src).mValue.mIntVal == (*dest).mValue.mIntVal
//         }
//         ValueType::BY_CLASS | ValueType::BY_FUNC => {
//             (*src).mValue.mObj.mPtr == (*dest).mValue.mObj.mPtr
//         }
//         ValueType::BY_STRING => {
//             let a = get_string_buffer(src);
//             let b = get_string_buffer(dest);
//             if a.is_null() || b.is_null() {
//                 return a.is_null() && b.is_null();
//             }
//             if (*a).mLen != (*b).mLen {
//                 return false;
//             }
//             let len = (*a).mLen.max(0) as usize;
//             std::slice::from_raw_parts((*a).mPtr as *const u8, len)
//                 == std::slice::from_raw_parts((*b).mPtr as *const u8, len)
//         }
//         _ => false,
//     }
// }

/// get(index): return element at index. Match get_element_from_vector.
unsafe fn array_get_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let index_val = get_local_value(1, vm) as *const BoyiaValue;
    if obj.is_null() || index_val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*obj).mValue.mObj.mPtr as *const BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let idx = (*index_val).mValue.mIntVal;
    let len = (*fun).mParamSize as isize;
    if idx < 0 || idx >= len {
        return OpHandleResult::kOpResultEnd;
    }
    let result = (*fun).mParams.add(idx as usize);
    set_native_result(result as *mut BoyiaValue as *mut LVoid, vm);
    OpHandleResult::kOpResultSuccess
}

/// add(element): append element. Match add_element_to_vector.
unsafe fn array_add_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *mut BoyiaValue;
    let element = get_local_value(1, vm) as *const BoyiaValue;
    if obj.is_null() || element.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let cap = get_function_count(fun);
    if (*fun).mParamSize >= cap {
        if !vector_params_grow_if_full(fun, vm) {
            return OpHandleResult::kOpResultEnd;
        }
    }
    if (*fun).mParams.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let dst = (*fun).mParams.add((*fun).mParamSize as usize);
    value_copy(dst, element);
    (*fun).mParamSize += 1;
    OpHandleResult::kOpResultSuccess
}

/// size(): return array length (int). Match get_vector_size.
unsafe fn array_size_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    if obj.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*obj).mValue.mObj.mPtr as *const BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    set_int_result((*fun).mParamSize, vm);
    OpHandleResult::kOpResultSuccess
}

/// clear(): set length to 0. Match clear_vector with newSize=0.
unsafe fn array_clear_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    if obj.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    (*fun).mParamSize = 0;
    OpHandleResult::kOpResultSuccess
}

/// removeAt(index): remove element at index, shift down. Match remove_element_width_index.
unsafe fn array_remove_at_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let idx_val = get_local_value(1, vm) as *const BoyiaValue;
    if obj.is_null() || idx_val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let idx = (*idx_val).mValue.mIntVal;
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let len = (*fun).mParamSize as isize;
    if idx < 0 || idx >= len {
        return OpHandleResult::kOpResultEnd;
    }
    let params = (*fun).mParams;
    for i in idx..(len - 1) {
        value_copy(params.add(i as usize), params.add(i as usize + 1));
    }
    (*fun).mParamSize -= 1;
    OpHandleResult::kOpResultSuccess
}

/// remove(value): remove first match from end. Match remove_element_from_vector.
unsafe fn array_remove_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let val = get_local_value(1, vm) as *const BoyiaValue;
    if obj.is_null() || val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let params = (*fun).mParams;
    let mut idx = (*fun).mParamSize - 1;
    while idx >= 0 {
        let elem = params.add(idx as usize);
        if compare_value(elem, val) {
            for i in idx..((*fun).mParamSize - 1) {
                value_copy(params.add(i as usize), params.add(i as usize + 1));
            }
            (*fun).mParamSize -= 1;
            return OpHandleResult::kOpResultSuccess;
        }
        idx -= 1;
    }
    OpHandleResult::kOpResultSuccess
}

/// Register Array builtin class: get, add, size, clear, removeAt, remove.
pub fn builtin_array_class<F>(vm: *mut LVoid, gen_id: &mut F)
where
    F: FnMut(&str) -> LUintPtr,
{
    if vm.is_null() {
        return;
    }
    let array_key = gen_id("Array");
    let class_ref = unsafe { create_global_class(array_key, vm) } as *mut BoyiaValue;
    if class_ref.is_null() {
        return;
    }
    unsafe {
        (*class_ref).mValue.mObj.mSuper = K_BOYIA_NULL;
        let class_body = (*class_ref).mValue.mObj.mPtr as *mut BoyiaFunction;
        if class_body.is_null() || (*class_body).mParams.is_null() {
            return;
        }
        gen_builtin_class_function(gen_id("get"), array_get_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("add"), array_add_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("size"), array_size_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("clear"), array_clear_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("removeAt"), array_remove_at_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("remove"), array_remove_impl as NativePtr, class_body, vm);
    }
}

/// Create an Array instance. Match CreateArrayObject.
pub unsafe fn create_array_object(vm: *mut LVoid, array_class_key: LUintPtr) -> *mut LVoid {
    copy_object(array_class_key, 32, vm)
}
