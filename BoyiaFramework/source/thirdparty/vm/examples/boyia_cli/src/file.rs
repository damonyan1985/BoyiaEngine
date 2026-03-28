//! File builtin for `boyia_cli`: async read/write on `ThreadPool`, callback on the runtime task thread.
//! Mirrors `https.rs` (runner from last local after `LocalPush(mClass)`).

#![allow(dead_code)]

use crate::runner::BoyiaRunner;
use boyia_builtins::gen_builtin_class_function;
use boyia_runtime::BoyiaRuntime;
use boyia_vm::{
    create_global_class, create_native_string, get_local_size, get_local_value, get_runtime_from_vm,
    get_string_buffer, native_call_impl, set_int_result, BoyiaClass, BoyiaFunction, BoyiaValue,
    Global, NativePtr, RealValue, Runtime, ValueType, LInt, LIntPtr, LUintPtr, LVoid, OpHandleResult,
};
use std::fs;
use std::str;

const K_BOYIA_NULL: LIntPtr = 0;
const FILE_CLASS_RUNNER_PROP_INDEX: usize = 0;

#[derive(Clone, Copy)]
struct CallbackInfo {
    name_key: LUintPtr,
    value_type: ValueType,
    func_ptr: LIntPtr,
    object_global: *mut Global,
}
unsafe impl Send for CallbackInfo {}

pub fn builtin_file_class<F>(vm: *mut LVoid, gen_id: &mut F, runner_ptr: *mut crate::runner::BoyiaRunner)
where
    F: FnMut(&str) -> LUintPtr,
{
    if vm.is_null() {
        return;
    }

    let file_key = gen_id("File");
    let class_ref = unsafe { create_global_class(file_key, vm) } as *mut BoyiaValue;
    if class_ref.is_null() {
        return;
    }

    unsafe {
        (*class_ref).mValue.mObj.mSuper = K_BOYIA_NULL;
        let class_body = (*class_ref).mValue.mObj.mPtr as *mut BoyiaFunction;
        if class_body.is_null() || (*class_body).mParams.is_null() {
            return;
        }

        let runner_slot = (*class_body).mParams.add((*class_body).mParamSize as usize);
        (*runner_slot).mValueType = ValueType::BY_INT;
        (*runner_slot).mNameKey = gen_id("runner");
        (*runner_slot).mValue.mIntVal = runner_ptr as LIntPtr;
        (*class_body).mParamSize += 1;

        gen_builtin_class_function(gen_id("read"), file_read_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("write"), file_write_impl as NativePtr, class_body, vm);
    }
}

fn value_to_string(value: *const BoyiaValue) -> Option<String> {
    if value.is_null() {
        return None;
    }
    let str_ref = unsafe { get_string_buffer(value) };
    if str_ref.is_null() {
        return None;
    }
    let len = unsafe { (*str_ref).mLen.max(0) as usize };
    let ptr = unsafe { (*str_ref).mPtr as *const u8 };
    let slice = unsafe { std::slice::from_raw_parts(ptr, len) };
    str::from_utf8(slice).ok().map(ToOwned::to_owned)
}

fn schedule_read(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    callback: CallbackInfo,
) -> bool {
    let Some((runtime_handle, thread_pool_weak)) =
        (unsafe { BoyiaRunner::get_handle_and_pool_from_ptr(runner_ptr) })
    else {
        println!("call schedule_read {}", runner_ptr as LIntPtr);
        return false;
    };
    let Some(thread_pool) = thread_pool_weak.upgrade() else {
        return false;
    };

    thread_pool
        .post_task(move || {
            let body = match fs::read_to_string(&path) {
                Ok(text) => text,
                Err(err) => format!("File.read error: {err}"),
            };
            let _ = runtime_handle.post_task(move |runtime| unsafe {
                callback_string(body, callback, runtime.as_mut());
                runtime.consume_micro_task();
            });
        })
        .is_ok()
}

fn schedule_write(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    content: String,
    callback: CallbackInfo,
) -> bool {
    let Some((runtime_handle, thread_pool_weak)) =
        (unsafe { BoyiaRunner::get_handle_and_pool_from_ptr(runner_ptr) })
    else {
        return false;
    };
    let Some(thread_pool) = thread_pool_weak.upgrade() else {
        return false;
    };

    thread_pool
        .post_task(move || {
            let body = match fs::write(&path, content.as_bytes()) {
                Ok(()) => String::from("ok"),
                Err(err) => format!("File.write error: {err}"),
            };
            let _ = runtime_handle.post_task(move |runtime| unsafe {
                callback_string(body, callback, runtime.as_mut());
                runtime.consume_micro_task();
            });
        })
        .is_ok()
}

unsafe fn callback_string(result: String, callback: CallbackInfo, runtime: &mut BoyiaRuntime) {
    let cb_fun = callback.func_ptr as *mut BoyiaFunction;
    if cb_fun.is_null() {
        return;
    }

    let obj_super = if callback.object_global.is_null() {
        0
    } else {
        (*callback.object_global).value().mValue.mObj.mPtr
    };

    let callback_value = BoyiaValue {
        mNameKey: callback.name_key,
        mValueType: callback.value_type,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: callback.func_ptr,
                mSuper: obj_super,
            },
        },
    };

    let len = result.len() as LInt;
    let buffer = leak_string_buffer(result);

    let mut value = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue { mIntVal: 0 },
    };
    create_native_string(&mut value, buffer, len, runtime.vm());

    let mut args = [callback_value, value];
    if !(*cb_fun).mParams.is_null() {
        args[1].mNameKey = (*(*cb_fun).mParams).mNameKey;
    }

    let mut obj = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_CLASS,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: obj_super,
                mSuper: 0,
            },
        },
    };
    native_call_impl(args.as_mut_ptr(), 2, &mut obj, runtime.vm());

    if !callback.object_global.is_null() {
        runtime.remove_persistent(callback.object_global);
    }
}

unsafe fn leak_string_buffer(result: String) -> *mut i8 {
    let boxed = result.into_bytes().into_boxed_slice();
    Box::into_raw(boxed) as *mut u8 as *mut i8
}

unsafe fn make_callback_info(vm: *mut LVoid, callback_val: *const BoyiaValue) -> Option<CallbackInfo> {
    if callback_val.is_null() {
        return None;
    }
    let object_addr = (*callback_val).mValue.mObj.mSuper;
    let object_value = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_CLASS,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: object_addr,
                mSuper: 0,
            },
        },
    };
    let object_global = {
        let rt = get_runtime_from_vm(vm);
        if rt.is_null() {
            std::ptr::null_mut()
        } else {
            (*rt).persistent_object(&object_value as *const BoyiaValue)
        }
    };
    Some(CallbackInfo {
        name_key: (*callback_val).mNameKey,
        value_type: (*callback_val).mValueType,
        func_ptr: (*callback_val).mValue.mObj.mPtr,
        object_global,
    })
}

unsafe fn runner_from_class(class_val: *const BoyiaValue) -> *mut BoyiaRunner {
    let class_body = (*class_val).mValue.mObj.mPtr as *mut BoyiaFunction;
    (*class_body)
        .mParams
        .add(FILE_CLASS_RUNNER_PROP_INDEX)
        .read()
        .mValue
        .mIntVal as *mut BoyiaRunner
}

/// `File.read(path, callback)` — UTF-8 text or error string.
unsafe fn file_read_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    if size < 3 {
        return OpHandleResult::kOpResultEnd;
    }

    let class_val = get_local_value(size - 1, vm) as *const BoyiaValue;
    let runner_ptr = runner_from_class(class_val);

    let path_val = get_local_value(1, vm) as *const BoyiaValue;
    let callback_val = get_local_value(2, vm) as *const BoyiaValue;

    let Some(path) = value_to_string(path_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(callback) = make_callback_info(vm, callback_val) else {
        return OpHandleResult::kOpResultEnd;
    };

    let scheduled = schedule_read(runner_ptr, path, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}

/// `File.write(path, content, callback)` — callback gets "" on success, else error message.
unsafe fn file_write_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    if size < 4 {
        return OpHandleResult::kOpResultEnd;
    }

    let class_val = get_local_value(size - 1, vm) as *const BoyiaValue;
    let runner_ptr = runner_from_class(class_val);

    let path_val = get_local_value(1, vm) as *const BoyiaValue;
    let content_val = get_local_value(2, vm) as *const BoyiaValue;
    let callback_val = get_local_value(3, vm) as *const BoyiaValue;

    let Some(path) = value_to_string(path_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(content) = value_to_string(content_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(callback) = make_callback_info(vm, callback_val) else {
        return OpHandleResult::kOpResultEnd;
    };

    let scheduled = schedule_write(runner_ptr, path, content, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}
