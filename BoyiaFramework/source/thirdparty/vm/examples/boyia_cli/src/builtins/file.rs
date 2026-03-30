//! File builtin for `boyia_cli`: async read/write on `ThreadPool`, callback on the runtime task thread.
//! Mirrors the Https builtin (runner from last local after `LocalPush(mClass)`).

#![allow(dead_code)]

use super::r#async::{make_callback_info, runner_from_class, schedule_task, value_to_string, CallbackInfo};
use crate::runner::BoyiaRunner;
use boyia_builtins::gen_builtin_class_function;
use boyia_vm::{
    create_global_class, get_local_size, get_local_value, set_int_result, BoyiaFunction, BoyiaValue,
    K_BOYIA_NULL, NativePtr, ValueType, LIntPtr, LUintPtr, LVoid, OpHandleResult,
};
use std::fs;

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

fn schedule_read(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || match fs::read_to_string(&path) {
            Ok(text) => text,
            Err(err) => format!("File.read error: {err}"),
        },
        callback,
        |_| (),
        || println!("call schedule_read {}", runner_ptr as LIntPtr),
    )
}

fn schedule_write(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    content: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || match fs::write(&path, content.as_bytes()) {
            Ok(()) => String::from("ok"),
            Err(err) => format!("File.write error: {err}"),
        },
        callback,
        |_| (),
        || (),
    )
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
