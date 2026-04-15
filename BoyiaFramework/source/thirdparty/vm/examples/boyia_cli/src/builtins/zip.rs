//! Zip builtin: compress / extract on [crate::thread_pool::ThreadPool]; callback receives result Map (see `async` module).
//! Password: AES-256 when non-empty (WinZip-compatible); extract uses `by_index_decrypt` / `by_index`.

#![allow(dead_code)]

use super::r#async::{
    install_runner_param_slot, make_callback_info, runner_from_class, schedule_task, value_to_string,
    AsyncBuiltinResult, CallbackInfo,
};
use crate::runner::BoyiaRunner;
use boyia_builtins::gen_builtin_class_function;
use boyia_vm::{
    create_global_class, get_local_size, get_local_value, set_int_result, BoyiaFunction, BoyiaValue,
    K_BOYIA_NULL, NativePtr, LUintPtr, LVoid, OpHandleResult,
};
use std::fs::{self, File};
use std::io::copy;
use std::path::{Path, PathBuf};
use walkdir::WalkDir;
use zip::write::{FileOptions, ZipWriter};
use zip::CompressionMethod;
use zip::ZipArchive;

/// `Zip.compress` / `Zip.extract`: local **0** = callee, **1..** = args, last = class (`this`).
/// No password: **5** locals `(callee, src, dest, callback, this)`. With password: **6** `(callee, src, dest, password, callback, this)`.
pub fn builtin_zip_class<F>(vm: *mut LVoid, gen_id: &mut F, runner_ptr: *mut BoyiaRunner)
where
    F: FnMut(&str) -> LUintPtr,
{
    if vm.is_null() {
        return;
    }

    let zip_key = gen_id("Zip");
    let class_ref = unsafe { create_global_class(zip_key, vm) } as *mut BoyiaValue;
    if class_ref.is_null() {
        return;
    }

    unsafe {
        (*class_ref).mValue.mObj.mSuper = K_BOYIA_NULL;
        let class_body = (*class_ref).mValue.mObj.mPtr as *mut BoyiaFunction;
        if !install_runner_param_slot(class_body, gen_id, runner_ptr) {
            return;
        }

        gen_builtin_class_function(gen_id("compress"), zip_compress_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("extract"), zip_extract_impl as NativePtr, class_body, vm);
    }
}

fn file_options<'a>(password: &'a str) -> FileOptions<'a, ()> {
    let base = FileOptions::<()>::default().compression_method(CompressionMethod::Deflated);
    if password.is_empty() {
        base
    } else {
        base.with_aes_encryption(zip::AesMode::Aes256, password)
    }
}

fn run_compress(src: PathBuf, dest_zip: PathBuf, password: String) -> AsyncBuiltinResult {
    let meta = match fs::metadata(&src) {
        Ok(m) => m,
        Err(e) => {
            return AsyncBuiltinResult::Fail {
                message: format!("Zip.compress error: metadata {e}"),
            };
        }
    };

    let dest_file = match File::create(&dest_zip) {
        Ok(f) => f,
        Err(e) => {
            return AsyncBuiltinResult::Fail {
                message: format!("Zip.compress error: create zip {e}"),
            };
        }
    };

    let mut writer = ZipWriter::new(dest_file);
    let opts = file_options(&password);

    let r = if meta.is_file() {
        let name = src
            .file_name()
            .and_then(|n| n.to_str())
            .unwrap_or("file")
            .to_string();
        if let Err(e) = writer.start_file(&name, opts) {
            let _ = writer.finish();
            return AsyncBuiltinResult::Fail {
                message: format!("Zip.compress error: start_file {e}"),
            };
        }
        match File::open(&src) {
            Ok(mut f) => match copy(&mut f, &mut writer) {
                Ok(_) => Ok(()),
                Err(e) => Err(e.to_string()),
            },
            Err(e) => Err(e.to_string()),
        }
    } else if meta.is_dir() {
        compress_dir(&mut writer, &src, opts)
    } else {
        Err("Zip.compress error: unsupported path type".into())
    };

    if let Err(msg) = r {
        let _ = writer.finish();
        return AsyncBuiltinResult::Fail { message: msg };
    }

    match writer.finish() {
        Ok(_) => AsyncBuiltinResult::Ok { data: None },
        Err(e) => AsyncBuiltinResult::Fail {
            message: format!("Zip.compress error: finish {e}"),
        },
    }
}

fn compress_dir<'a>(
    writer: &mut ZipWriter<File>,
    root: &Path,
    opts: FileOptions<'a, ()>,
) -> Result<(), String> {
    for entry in WalkDir::new(root).into_iter().filter_map(|e| e.ok()) {
        let path = entry.path();
        if path.is_file() {
            let rel = path.strip_prefix(root).map_err(|e| e.to_string())?;
            let name = rel
                .to_str()
                .ok_or_else(|| "Zip.compress error: non-utf8 path".to_string())?
                .replace('\\', "/");
            writer
                .start_file(&name, opts.clone())
                .map_err(|e| format!("Zip.compress error: start_file {e}"))?;
            let mut f = File::open(path).map_err(|e| e.to_string())?;
            copy(&mut f, writer).map_err(|e| e.to_string())?;
        }
    }
    Ok(())
}

fn open_entry<'a>(
    archive: &'a mut ZipArchive<File>,
    i: usize,
    password: &str,
) -> Result<zip::read::ZipFile<'a>, String> {
    if password.is_empty() {
        archive.by_index(i).map_err(|e| e.to_string())
    } else {
        archive
            .by_index_decrypt(i, password.as_bytes())
            .map_err(|e| e.to_string())
    }
}

fn run_extract(src_zip: PathBuf, dest_dir: PathBuf, password: String) -> AsyncBuiltinResult {
    let file = match File::open(&src_zip) {
        Ok(f) => f,
        Err(e) => {
            return AsyncBuiltinResult::Fail {
                message: format!("Zip.extract error: open {e}"),
            };
        }
    };

    let mut archive = match ZipArchive::new(file) {
        Ok(a) => a,
        Err(e) => {
            return AsyncBuiltinResult::Fail {
                message: format!("Zip.extract error: {e}"),
            };
        }
    };

    if let Err(e) = fs::create_dir_all(&dest_dir) {
        return AsyncBuiltinResult::Fail {
            message: format!("Zip.extract error: mkdir {e}"),
        };
    }

    for i in 0..archive.len() {
        let mut entry = match open_entry(&mut archive, i, &password) {
            Ok(e) => e,
            Err(e) => {
                return AsyncBuiltinResult::Fail {
                    message: format!("Zip.extract error: entry {i} {e}"),
                };
            }
        };

        let enclosed = match entry.enclosed_name() {
            Some(p) => p.to_owned(),
            None => continue,
        };

        let out_path = dest_dir.join(&enclosed);
        if entry.is_dir() {
            if let Err(e) = fs::create_dir_all(&out_path) {
                return AsyncBuiltinResult::Fail {
                    message: format!("Zip.extract error: mkdir {e}"),
                };
            }
            continue;
        }

        if let Some(parent) = out_path.parent() {
            if let Err(e) = fs::create_dir_all(parent) {
                return AsyncBuiltinResult::Fail {
                    message: format!("Zip.extract error: mkdir {e}"),
                };
            }
        }

        let mut out_file = match File::create(&out_path) {
            Ok(f) => f,
            Err(e) => {
                return AsyncBuiltinResult::Fail {
                    message: format!("Zip.extract error: create file {e}"),
                };
            }
        };

        if let Err(e) = copy(&mut entry, &mut out_file) {
            return AsyncBuiltinResult::Fail {
                message: format!("Zip.extract error: write {e}"),
            };
        }
    }

    AsyncBuiltinResult::Ok { data: None }
}

fn schedule_compress(
    runner_ptr: *mut BoyiaRunner,
    src: String,
    dest: String,
    password: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || run_compress(PathBuf::from(src), PathBuf::from(dest), password),
        callback,
        |_| (),
        || (),
    )
}

fn schedule_extract(
    runner_ptr: *mut BoyiaRunner,
    src: String,
    dest: String,
    password: String,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || run_extract(PathBuf::from(src), PathBuf::from(dest), password),
        callback,
        |_| (),
        || (),
    )
}

/// `Zip.compress(src, destZip, callback)` or `Zip.compress(src, destZip, password, callback)`.
unsafe fn zip_compress_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    if size != 5 && size != 6 {
        return OpHandleResult::kOpResultEnd;
    }

    let class_val = get_local_value(size - 1, vm) as *const BoyiaValue;
    let runner_ptr = runner_from_class(class_val);

    let (src_v, dest_v, password, cb_v) = if size == 5 {
        (
            get_local_value(1, vm) as *const BoyiaValue,
            get_local_value(2, vm) as *const BoyiaValue,
            String::new(),
            get_local_value(3, vm) as *const BoyiaValue,
        )
    } else {
        (
            get_local_value(1, vm) as *const BoyiaValue,
            get_local_value(2, vm) as *const BoyiaValue,
            value_to_string(get_local_value(3, vm) as *const BoyiaValue).unwrap_or_default(),
            get_local_value(4, vm) as *const BoyiaValue,
        )
    };

    let Some(src) = value_to_string(src_v) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(dest) = value_to_string(dest_v) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(callback) = make_callback_info(vm, cb_v) else {
        return OpHandleResult::kOpResultEnd;
    };

    let scheduled = schedule_compress(runner_ptr, src, dest, password, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}

/// `Zip.extract(srcZip, destDir, callback)` or `Zip.extract(srcZip, destDir, password, callback)`.
unsafe fn zip_extract_impl(vm: *mut LVoid) -> OpHandleResult {
    println!("call zip_extract_impl");
    let size = get_local_size(vm);
    if size != 5 && size != 6 {
        return OpHandleResult::kOpResultEnd;
    }

    let class_val = get_local_value(size - 1, vm) as *const BoyiaValue;
    let runner_ptr = runner_from_class(class_val);

    println!("call zip_extract_impl1");
    let (src_v, dest_v, password, cb_v) = if size == 5 {
        (
            get_local_value(1, vm) as *const BoyiaValue,
            get_local_value(2, vm) as *const BoyiaValue,
            String::new(),
            get_local_value(3, vm) as *const BoyiaValue,
        )
    } else {
        (
            get_local_value(1, vm) as *const BoyiaValue,
            get_local_value(2, vm) as *const BoyiaValue,
            value_to_string(get_local_value(3, vm) as *const BoyiaValue).unwrap_or_default(),
            get_local_value(4, vm) as *const BoyiaValue,
        )
    };

    let Some(src) = value_to_string(src_v) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(dest) = value_to_string(dest_v) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(callback) = make_callback_info(vm, cb_v) else {
        return OpHandleResult::kOpResultEnd;
    };

    let scheduled = schedule_extract(runner_ptr, src, dest, password, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}
