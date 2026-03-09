//! Boyia native lib: `new` (CreateObject), array APIs, BY_Log.
//! Port of BoyiaRuntime::initNativeFunction subset and BoyiaLib.cpp.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use boyia_vm::{
    get_local_value, get_string_buffer, get_boyia_class_id, set_int_result, set_native_result,
    value_copy, BoyiaValue, BoyiaFunction, ValueType, LInt, LVoid, OpHandleResult,
};

/// Capacity mask for GET_FUNCTION_COUNT (C++ mParamCount & 0x0000FFFF)
fn get_function_count(fun: *const BoyiaFunction) -> LInt {
    if fun.is_null() {
        return 0;
    }
    unsafe { (*fun).mParamCount & 0x0000_FFFF }
}

/// new: create object from local 0 (class). Match CreateObject in BoyiaCore.cpp.
pub unsafe fn create_object(vm: *mut LVoid) -> OpHandleResult {
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if boyia_vm::create_object(vm) == 0 {
        OpHandleResult::kOpResultEnd
    } else {
        OpHandleResult::kOpResultSuccess
    }
}

/// BY_GetFromArray: local0 = array, local1 = index; result = array[index].
pub unsafe fn get_element_from_vector(vm: *mut LVoid) -> OpHandleResult {
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let val = get_local_value(0, vm) as *const BoyiaValue;
    let index_val = get_local_value(1, vm) as *const BoyiaValue;
    if val.is_null() || index_val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*val).mValue.mObj.mPtr as *const BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let idx = (*index_val).mValue.mIntVal;
    let size = (*fun).mParamSize as isize;
    if idx < 0 || idx >= size {
        return OpHandleResult::kOpResultEnd;
    }
    let result = (*fun).mParams.add(idx as usize);
    set_native_result(result as *mut BoyiaValue as *mut LVoid, vm);
    OpHandleResult::kOpResultSuccess
}

/// BY_AddInArray: local0 = array, local1 = element. No grow (fixed capacity).
pub unsafe fn add_element_to_vector(vm: *mut LVoid) -> OpHandleResult {
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let val = get_local_value(0, vm) as *mut BoyiaValue;
    let element = get_local_value(1, vm) as *const BoyiaValue;
    if val.is_null() || element.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*val).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let cap = get_function_count(fun);
    if (*fun).mParamSize >= cap {
        return OpHandleResult::kOpResultEnd;
    }
    let dst = (*fun).mParams.add((*fun).mParamSize as usize);
    value_copy(dst, element);
    (*fun).mParamSize += 1;
    OpHandleResult::kOpResultSuccess
}

/// BY_GetArraySize: local0 = array; result = size (int).
pub unsafe fn get_vector_size(vm: *mut LVoid) -> OpHandleResult {
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let val = get_local_value(0, vm) as *const BoyiaValue;
    if val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*val).mValue.mObj.mPtr as *const BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    set_int_result((*fun).mParamSize, vm);
    OpHandleResult::kOpResultSuccess
}

/// BY_ClearArray: local0 = array, local1 = new size (deltaIndex); set mParamSize = new size.
pub unsafe fn clear_vector(vm: *mut LVoid) -> OpHandleResult {
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let val = get_local_value(0, vm) as *const BoyiaValue;
    let delta_index = get_local_value(1, vm) as *const BoyiaValue;
    if val.is_null() || delta_index.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*val).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let new_size = (*delta_index).mValue.mIntVal as LInt;
    (*fun).mParamSize = if new_size < 0 { 0 } else { new_size };
    OpHandleResult::kOpResultSuccess
}

/// BY_RemoveWidthIndex: local0 = array, local1 = index; remove at index, shift down.
pub unsafe fn remove_element_width_index(vm: *mut LVoid) -> OpHandleResult {
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let array = get_local_value(0, vm) as *const BoyiaValue;
    let idx_val = get_local_value(1, vm) as *const BoyiaValue;
    if array.is_null() || idx_val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let idx = (*idx_val).mValue.mIntVal;
    let fun = (*array).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let size = (*fun).mParamSize as isize;
    if idx < 0 || idx >= size {
        return OpHandleResult::kOpResultEnd;
    }
    let params = (*fun).mParams;
    for i in idx..(size - 1) {
        value_copy(params.add(i as usize), params.add(i as usize + 1));
    }
    (*fun).mParamSize -= 1;
    OpHandleResult::kOpResultSuccess
}

/// Compare two BoyiaValue for equality (match compareValue in BoyiaLib.cpp).
unsafe fn compare_value(src: *const BoyiaValue, dest: *const BoyiaValue) -> bool {
    if src.is_null() || dest.is_null() {
        return false;
    }
    if (*src).mValueType != (*dest).mValueType {
        return false;
    }
    match (*src).mValueType {
        ValueType::BY_CHAR | ValueType::BY_INT | ValueType::BY_NAVCLASS => {
            (*src).mValue.mIntVal == (*dest).mValue.mIntVal
        }
        ValueType::BY_CLASS | ValueType::BY_FUNC => {
            (*src).mValue.mObj.mPtr == (*dest).mValue.mObj.mPtr
        }
        ValueType::BY_STRING => {
            let a = get_string_buffer(src);
            let b = get_string_buffer(dest);
            if a.is_null() || b.is_null() {
                return a.is_null() && b.is_null();
            }
            if (*a).mLen != (*b).mLen {
                return false;
            }
            let len = (*a).mLen.max(0) as usize;
            std::slice::from_raw_parts((*a).mPtr as *const u8, len)
                == std::slice::from_raw_parts((*b).mPtr as *const u8, len)
        }
        _ => false,
    }
}

/// BY_RemoveFromArray: local0 = array, local1 = value; remove first match from end.
pub unsafe fn remove_element_from_vector(vm: *mut LVoid) -> OpHandleResult {
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let array = get_local_value(0, vm) as *const BoyiaValue;
    let val = get_local_value(1, vm) as *const BoyiaValue;
    if array.is_null() || val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = (*array).mValue.mObj.mPtr as *mut BoyiaFunction;
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

/// BY_Log: local0 = value; print int or string to stdout.
pub unsafe fn log_print(vm: *mut LVoid) -> OpHandleResult {
    eprintln!("[log_print] called");
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let val = get_local_value(0, vm) as *const BoyiaValue;
    if val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }

    eprintln!("[log_print] called : {}", (*val).mValueType as i32);
    match (*val).mValueType {
        ValueType::BY_INT => {
            println!("Boyia [info]: {}", (*val).mValue.mIntVal);
        }
        ValueType::BY_CLASS => {
            if get_boyia_class_id(val) == string_class_key(vm) {
                let buf = get_string_buffer(val);
                if !buf.is_null() && (*buf).mLen > 0 {
                    let len = (*buf).mLen as usize;
                    let slice = std::slice::from_raw_parts((*buf).mPtr as *const u8, len);
                    let s = std::str::from_utf8(slice).unwrap_or("");
                    println!("Boyia [info]: {}", s);
                }
            }
        }
        _ => {}
    }
    OpHandleResult::kOpResultSuccess
}

/// String class key for BY_Log (same hash as builtins "String").
unsafe fn string_class_key(vm: *mut boyia_vm::LVoid) -> boyia_vm::LUintPtr {
    static B: [u8; 7] = *b"String\0";
    let s = boyia_vm::BoyiaStr {
        mPtr: B.as_ptr() as *mut _,
        mLen: 6,
    };
    boyia_vm::gen_identifier_from_str(vm, &s)
}
