//! Builtin classes and methods (String, Map, MicroTask, Array).
//! Port of BoyiaValue.cpp builtins: GenBuiltinClassFunction, BuiltinStringClass, BuiltinMapClass, BuiltinMicroTaskClass, etc.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use boyia_vm::{
    alloc_builtin_function, copy_object, create_global_class, create_micro_task, gen_identifier_from_str,
    get_local_size, get_local_value, get_string_buffer, get_string_hash, native_call_impl,
    resume_micro_task, set_native_result, value_copy,
    BoyiaClass, BoyiaFunction, BoyiaStr, BoyiaValue, NativePtr, RealValue, ValueType,
    LInt, LIntPtr, LUintPtr, LVoid, OpHandleResult,
};
use std::ptr;

const K_BOYIA_NULL: LIntPtr = 0;

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

// ---------------------------------------------------------------------------
// String builtin
// ---------------------------------------------------------------------------

unsafe extern "C" fn string_length_impl(vm: *mut LVoid) -> OpHandleResult {
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
    set_native_result(&mut value as *mut BoyiaValue as *mut LVoid, vm);
    OpHandleResult::kOpResultSuccess
}

unsafe extern "C" fn string_equal_impl(vm: *mut LVoid) -> OpHandleResult {
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
    set_native_result(&mut value as *mut BoyiaValue as *mut LVoid, vm);
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

// ---------------------------------------------------------------------------
// Map builtin
// ---------------------------------------------------------------------------

unsafe extern "C" fn map_put_impl(vm: *mut LVoid) -> OpHandleResult {
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

unsafe extern "C" fn map_get_impl(vm: *mut LVoid) -> OpHandleResult {
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
        set_native_result(&mut val as *mut BoyiaValue as *mut LVoid, vm);
        return OpHandleResult::kOpResultSuccess;
    }
    for i in 0..(*fun).mParamSize {
        if (*fun).mParams.add(i as usize).read().mNameKey == key_id {
            set_native_result((*fun).mParams.add(i as usize) as *mut LVoid, vm);
            return OpHandleResult::kOpResultSuccess;
        }
    }
    let mut val = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue { mIntVal: K_BOYIA_NULL },
    };
    set_native_result(&mut val as *mut BoyiaValue as *mut LVoid, vm);
    OpHandleResult::kOpResultSuccess
}

unsafe extern "C" fn map_remove_impl(vm: *mut LVoid) -> OpHandleResult {
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

unsafe extern "C" fn map_clear_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *mut BoyiaValue;
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if !fun.is_null() {
        (*fun).mParamSize = 0;
    }
    OpHandleResult::kOpResultSuccess
}

unsafe extern "C" fn map_map_impl(vm: *mut LVoid) -> OpHandleResult {
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
                        mSuper: 0,
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

// ---------------------------------------------------------------------------
// MicroTask builtin
// ---------------------------------------------------------------------------

unsafe extern "C" fn micro_task_resolve_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let result = get_local_value(1, vm) as *mut BoyiaValue;
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    let task_ptr = (*fun).mParams.add(1).read().mValue.mIntVal;
    resume_micro_task(task_ptr as *mut LVoid, result, vm);
    OpHandleResult::kOpResultSuccess
}

unsafe extern "C" fn micro_task_init_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *mut BoyiaValue;
    let worker = get_local_value(1, vm) as *const BoyiaValue;
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    let task = create_micro_task(vm, obj);
    if task.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    (*fun).mParams.add(1).write(BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue {
            mIntVal: task as LIntPtr,
        },
    });
    let resolve_val = (*fun).mParams.add(0).read();
    let mut args = [*worker, resolve_val];
    let mut cb_obj = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_CLASS,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: (*worker).mValue.mObj.mSuper,
                mSuper: 0,
            },
        },
    };
    native_call_impl(args.as_mut_ptr(), 2, &mut cb_obj, vm)
}

/// Register MicroTask builtin class: task prop; init, resolve (prop).
pub fn builtin_micro_task_class<F>(vm: *mut LVoid, gen_id: &mut F)
where
    F: FnMut(&str) -> LUintPtr,
{
    eprintln!("[builtin_micro_task_class] 1");
    if vm.is_null() {
        return;
    }
    let micro_task_key = gen_id("MicroTask");
    eprintln!("[builtin_micro_task_class] 2 create_global_class");
    let class_ref = unsafe { create_global_class(micro_task_key, vm) } as *mut BoyiaValue;
    if class_ref.is_null() {
        eprintln!("[builtin_micro_task_class] class_ref null");
        return;
    }
    eprintln!("[builtin_micro_task_class] 3");
    unsafe {
        (*class_ref).mValue.mObj.mSuper = K_BOYIA_NULL;
        let class_body = (*class_ref).mValue.mObj.mPtr as *mut BoyiaFunction;
        if class_body.is_null() || (*class_body).mParams.is_null() {
            eprintln!("[builtin_micro_task_class] class_body or mParams null");
            return;
        }
        eprintln!("[builtin_micro_task_class] 4 task prop");
        let task_slot = (*class_body).mParams.add((*class_body).mParamSize as usize);
        (*task_slot).mValueType = ValueType::BY_INT;
        (*task_slot).mNameKey = gen_id("task");
        (*task_slot).mValue.mIntVal = K_BOYIA_NULL;
        (*class_body).mParamSize += 1;
        eprintln!("[builtin_micro_task_class] 5 init, resolve");
        gen_builtin_class_function(gen_id("init"), micro_task_init_impl as NativePtr, class_body, vm);
        gen_builtin_class_prop_function(gen_id("resolve"), micro_task_resolve_impl as NativePtr, class_body, vm);
    }
    eprintln!("[builtin_micro_task_class] 6 done");
}

/// Create a MicroTask instance. Match CreateMicroTaskObject.
pub unsafe fn create_micro_task_object(vm: *mut LVoid, micro_task_class_key: LUintPtr) -> *mut LVoid {
    copy_object(micro_task_class_key, 32, vm)
}

// ---------------------------------------------------------------------------
// Array builtin
// ---------------------------------------------------------------------------

/// Create an Array instance. Match CreateArrayObject.
pub unsafe fn create_array_object(vm: *mut LVoid, array_class_key: LUintPtr) -> *mut LVoid {
    copy_object(array_class_key, 32, vm)
}
