//! Map builtin class: put, get, remove, clear, map.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::gen_builtin_class_function;
use std::ptr;

use boyia_vm::{
    copy_object, create_global_class, gen_identifier_from_str, get_boyia_class_id, get_local_size,
    get_local_value, get_string_buffer, native_call_impl, set_native_result, value_copy, BuiltinId,
    BoyiaClass, BoyiaFunction, BoyiaStr, BoyiaValue, K_BOYIA_NULL, NativePtr, RealValue, ValueType,
    LInt, LUintPtr, LVoid, OpHandleResult,
};

unsafe fn map_put_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *mut BoyiaValue;
    let key_val = get_local_value(1, vm) as *const BoyiaValue;
    let value_val = get_local_value(2, vm) as *const BoyiaValue;
    let key_str = get_string_buffer(key_val);
    let key_id = gen_identifier_from_str(vm, key_str);
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    let slot = (*fun).mParams.add((*fun).mParamSize as usize);
    value_copy(slot, value_val);
    (*slot).mNameKey = key_id;
    (*fun).mParamSize += 1;
    OpHandleResult::kOpResultSuccess
}

/// Copy UTF-8 from `buf` into an owned `String` (null / `mLen <= 0` → empty; invalid UTF-8 lossy).
/// Rejects **`mLen > 0` with null `mPtr`** (corrupt [`BoyiaStr`]) so [`std::slice::from_raw_parts`] is not called on null.
unsafe fn boyia_str_ptr_to_string(buf: *const BoyiaStr) -> String {
    if buf.is_null() {
        return String::new();
    }
    let len = (*buf).mLen.max(0) as usize;
    if len == 0 {
        return String::new();
    }
    let p = (*buf).mPtr as *const u8;
    if p.is_null() {
        return String::new();
    }
    let slice = std::slice::from_raw_parts(p, len);
    String::from_utf8_lossy(slice).into_owned()
}

/// If `v` is a Boyia string ([`ValueType::BY_STRING`] or [`ValueType::BY_CLASS`] with [`BuiltinId::kBoyiaString`]),
/// copy UTF-8 via [`boyia_str_ptr_to_string`]. Otherwise returns empty without treating the value as a string body.
unsafe fn boyia_str_to_string(v: *const BoyiaValue) -> String {
    if v.is_null() {
        return String::new();
    }
    let string_class = BuiltinId::kBoyiaString.as_key();
    let buf = match (*v).mValueType {
        ValueType::BY_STRING => ptr::addr_of!((*v).mValue.mStrVal),
        ValueType::BY_CLASS if get_boyia_class_id(v) == string_class => get_string_buffer(v),
        _ => return String::new(),
    };
    boyia_str_ptr_to_string(buf)
}

unsafe fn map_get_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let key_val = get_local_value(1, vm) as *const BoyiaValue;
    let key_str = get_string_buffer(key_val);
    let key_id = gen_identifier_from_str(vm, key_str);
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        let mut val = BoyiaValue {
            mNameKey: 0,
            mValueType: ValueType::BY_INT,
            mValue: RealValue { mIntVal: K_BOYIA_NULL },
        };
        set_native_result(&mut val, vm);
        return OpHandleResult::kOpResultSuccess;
    }
    for i in 0..(*fun).mParamSize {
        if (*fun).mParams.add(i as usize).read().mNameKey == key_id {
            // println!("map_get_impl find key: {}", boyia_str_to_string(key_val));
            // let value_val = (*fun).mParams.add(i as usize);
            // let re = get_string_buffer(value_val);
            
            // println!("map_get_impl find value: {:?}", (*re).mLen);
            // if (*re).mLen > 0 {
            //     println!("map_get_impl find value: {}", boyia_str_ptr_to_string(re));
            // }
            set_native_result((*fun).mParams.add(i as usize), vm);
            return OpHandleResult::kOpResultSuccess;
        }
    }
    let mut val = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue { mIntVal: K_BOYIA_NULL },
    };
    set_native_result(&mut val, vm);
    OpHandleResult::kOpResultSuccess
}

unsafe fn map_remove_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *mut BoyiaValue;
    let key_val = get_local_value(1, vm) as *const BoyiaValue;
    let key_str = get_string_buffer(key_val);
    let key_id = gen_identifier_from_str(vm, key_str);
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    let mut idx = -1i32;
    for i in 0..(*fun).mParamSize {
        if (*fun).mParams.add(i as usize).read().mNameKey == key_id {
            idx = i as i32;
            break;
        }
    }
    if idx < 0 {
        return OpHandleResult::kOpResultSuccess;
    }
    let i = idx as LInt;
    for j in i..(*fun).mParamSize - 1 {
        value_copy(
            (*fun).mParams.add(j as usize),
            (*fun).mParams.add((j + 1) as usize) as *const BoyiaValue,
        );
    }
    (*fun).mParamSize -= 1;
    OpHandleResult::kOpResultSuccess
}

unsafe fn map_clear_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *mut BoyiaValue;
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if !fun.is_null() {
        (*fun).mParamSize = 0;
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn map_map_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let cb = get_local_value(1, vm) as *const BoyiaValue;
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    if (*cb).mValueType == ValueType::BY_PROP_FUNC {
        for i in 0..(*fun).mParamSize {
            let mut cb_obj = BoyiaValue {
                mNameKey: 0,
                mValueType: ValueType::BY_CLASS,
                mValue: RealValue {
                    mObj: BoyiaClass {
                        mPtr: (*cb).mValue.mObj.mSuper,
                        mSuper: K_BOYIA_NULL,
                    },
                },
            };
            let mut args = [
                *cb,
                (*fun).mParams.add(i as usize).read(),
            ];
            let cb_fun = (*cb).mValue.mObj.mPtr as *const BoyiaFunction;
            args[1].mNameKey = (*cb_fun).mParams.add(0).read().mNameKey;
            native_call_impl(args.as_mut_ptr(), 2, &mut cb_obj, vm);
        }
    }
    OpHandleResult::kOpResultSuccess
}

/// Register Map builtin class: put, get, remove, clear, map.
pub fn builtin_map_class<F>(vm: *mut LVoid, gen_id: &mut F)
where
    F: FnMut(&str) -> LUintPtr,
{
    eprintln!("[builtin_map_class] 1");
    if vm.is_null() {
        return;
    }
    let map_key = gen_id("Map");
    eprintln!("[builtin_map_class] 2 create_global_class");
    let class_ref = unsafe { create_global_class(map_key, vm) } as *mut BoyiaValue;
    if class_ref.is_null() {
        eprintln!("[builtin_map_class] class_ref null");
        return;
    }
    eprintln!("[builtin_map_class] 3");
    unsafe {
        (*class_ref).mValue.mObj.mSuper = K_BOYIA_NULL;
        let class_body = (*class_ref).mValue.mObj.mPtr as *mut BoyiaFunction;
        if class_body.is_null() || (*class_body).mParams.is_null() {
            eprintln!("[builtin_map_class] class_body or mParams null");
            return;
        }
        eprintln!("[builtin_map_class] 4 put");
        gen_builtin_class_function(gen_id("put"), map_put_impl as NativePtr, class_body, vm);
        eprintln!("[builtin_map_class] 5 get,remove,clear,map");
        gen_builtin_class_function(gen_id("get"), map_get_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("remove"), map_remove_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("clear"), map_clear_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("map"), map_map_impl as NativePtr, class_body, vm);
    }
    eprintln!("[builtin_map_class] 6 done");
}

/// Create a Map instance. Match CreatMapObject.
pub unsafe fn create_map_object(vm: *mut LVoid, map_class_key: LUintPtr) -> *mut LVoid {
    copy_object(map_class_key, 32, vm)
}
