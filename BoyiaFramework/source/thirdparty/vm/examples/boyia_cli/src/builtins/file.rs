//! File builtin for `boyia_cli`: async read/write/createDirs/create/delete/exists on `ThreadPool`, callback on the runtime task thread.
//! Mirrors the Https builtin (runner from last local after `LocalPush(mClass)`).

#![allow(dead_code)]

use super::r#async::{
    install_runner_param_slot, make_callback_info, runner_from_class, schedule_task, value_to_string,
    AsyncBuiltinResult, CallbackInfo,
};
use crate::runner::BoyiaRunner;
use boyia_builtins::gen_builtin_class_function;
use boyia_vm::{
    create_global_class, get_local_size, get_local_value, set_int_result, BoyiaFunction, BoyiaValue,
    K_BOYIA_NULL, NativePtr, LIntPtr, LUintPtr, LVoid, OpHandleResult,
};
use std::fs::{self, File};
use std::io::ErrorKind;

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
        if !install_runner_param_slot(class_body, gen_id, runner_ptr) {
            return;
        }

        gen_builtin_class_function(gen_id("read"), file_read_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("write"), file_write_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("createDirs"), file_create_dirs_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("create"), file_create_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("delete"), file_delete_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("exists"), file_exists_impl as NativePtr, class_body, vm);
    }
}

fn path_exists_result(path: &str) -> AsyncBuiltinResult {
    match fs::metadata(path) {
        Ok(meta) => {
            let tag = if meta.is_dir() {
                "dir"
            } else if meta.is_file() {
                "file"
            } else {
                "other"
            };
            AsyncBuiltinResult::Ok {
                data: Some(tag.to_string()),
            }
        }
        Err(e) if e.kind() == ErrorKind::NotFound => AsyncBuiltinResult::Fail {
            message: format!("File.exists: not found ({e})"),
        },
        Err(err) => AsyncBuiltinResult::Fail {
            message: format!("File.exists error: {err}"),
        },
    }
}

fn schedule_exists(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || path_exists_result(&path),
        callback,
        |_| (),
        || (),
    )
}

fn schedule_read(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || match fs::read_to_string(&path) {
            Ok(text) => AsyncBuiltinResult::Ok {
                data: Some(text),
            },
            Err(err) => AsyncBuiltinResult::Fail {
                message: format!("File.read error: {err}"),
            },
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
            Ok(()) => AsyncBuiltinResult::Ok { data: None },
            Err(err) => AsyncBuiltinResult::Fail {
                message: format!("File.write error: {err}"),
            },
        },
        callback,
        |_| (),
        || (),
    )
}

fn schedule_create_dirs(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || match fs::create_dir_all(&path) {
            Ok(()) => AsyncBuiltinResult::Ok { data: None },
            Err(err) => AsyncBuiltinResult::Fail {
                message: format!("File.createDirs error: {err}"),
            },
        },
        callback,
        |_| (),
        || (),
    )
}

fn schedule_create_file(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || match File::create(&path) {
            Ok(_f) => AsyncBuiltinResult::Ok { data: None },
            Err(err) => AsyncBuiltinResult::Fail {
                message: format!("File.create error: {err}"),
            },
        },
        callback,
        |_| (),
        || (),
    )
}

fn schedule_delete(
    runner_ptr: *mut BoyiaRunner,
    path: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || match fs::remove_file(&path) {
            Ok(()) => AsyncBuiltinResult::Ok { data: None },
            Err(err) => AsyncBuiltinResult::Fail {
                message: format!("File.delete error: {err}"),
            },
        },
        callback,
        |_| (),
        || (),
    )
}

/// `File.read(path, callback)` — callback Map: ok + `data` (file text), or fail + `message`.
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

/// `File.write(path, content, callback)` — callback gets a Map (`status`, optional `data` / `message`).
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

/// `File.createDirs(path, callback)` — callback Map: ok (no `data`), or fail + `message`.
unsafe fn file_create_dirs_impl(vm: *mut LVoid) -> OpHandleResult {
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

    let scheduled = schedule_create_dirs(runner_ptr, path, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}

/// `File.create(path, callback)` — creates or truncates an empty file (`std::fs::File::create`); parent directory must exist.
unsafe fn file_create_impl(vm: *mut LVoid) -> OpHandleResult {
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

    let scheduled = schedule_create_file(runner_ptr, path, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}

/// `File.delete(path, callback)` — removes a **file** only (`std::fs::remove_file`); callback Map like `write` / `createDirs`.
unsafe fn file_delete_impl(vm: *mut LVoid) -> OpHandleResult {
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

    let scheduled = schedule_delete(runner_ptr, path, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}

/// `File.exists(path, callback)` — callback Map: ok + `data` is `"file"`, `"dir"`, or `"other"`; fail + `message` if path not found or on I/O error.
unsafe fn file_exists_impl(vm: *mut LVoid) -> OpHandleResult {
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

    let scheduled = schedule_exists(runner_ptr, path, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}
