//! Boyia native lib: `new` (CreateObject), BY_Log, BY_Require (`requireFile`).
//! Port of BoyiaRuntime::initNativeFunction subset and BoyiaLib.cpp.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use boyia_vm::{
    get_local_size, get_local_value, get_runtime_from_vm, get_string_buffer, get_boyia_class_id,
    BoyiaValue, LVoid, OpHandleResult, ValueType,
};
use std::path::{Path, PathBuf};

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
        ValueType::BY_REAL => {
            println!("Boyia [info]: {}", (*val).mValue.mRealVal);
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

/// Read script path string from local 0 (String object). `vm` is required for class id check.
unsafe fn require_path_from_local0(vm: *mut LVoid, val: *const BoyiaValue) -> Option<String> {
    if val.is_null() || (*val).mValueType != ValueType::BY_CLASS {
        return None;
    }
    if get_boyia_class_id(val) != string_class_key(vm) {
        return None;
    }
    let buf = get_string_buffer(val);
    if buf.is_null() || (*buf).mLen <= 0 {
        return Some(String::new());
    }
    let len = (*buf).mLen as usize;
    let slice = std::slice::from_raw_parts((*buf).mPtr as *const u8, len);
    Some(String::from_utf8_lossy(slice).into_owned())
}

fn normalize_path_key(path: &str) -> String {
    let p = Path::new(path);
    std::fs::canonicalize(p)
        .map(|p| p.to_string_lossy().into_owned())
        .unwrap_or_else(|_| path.to_string())
}

/// Match `FileUtil::getAbsoluteFilePath` + `isAbsolutePath`: resolve `rel` against directory of `base` (file or dir).
fn resolve_require_path(base: &str, rel: &str) -> String {
    let rel_p = Path::new(rel);
    if rel_p.is_absolute() {
        return normalize_path_key(rel);
    }
    let dir: PathBuf = if base.is_empty() {
        std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."))
    } else {
        let b = Path::new(base);
        if b.is_dir() {
            b.to_path_buf()
        } else {
            b.parent()
                .filter(|p| !p.as_os_str().is_empty())
                .map(Path::to_path_buf)
                .unwrap_or_else(|| PathBuf::from("."))
        }
    };
    let joined = dir.join(rel);
    normalize_path_key(&joined.to_string_lossy())
}

/// `BoyiaLib.cpp` `requireFile` — native name `BY_Require` in C++ `initNativeFunction`.
pub unsafe fn require_file(vm: *mut LVoid) -> OpHandleResult {
    println!("call require_file");
    if vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let rt = get_runtime_from_vm(vm);
    if rt.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*rt).is_load_exe_file() {
        return OpHandleResult::kOpResultSuccess;
    }

    println!("call require_file1");
    if get_local_size(vm) < 1 {
        eprintln!("requireFile: arguments count < 1");
        return OpHandleResult::kOpResultEnd;
    }
    let script_src_val = get_local_value(0, vm) as *const BoyiaValue;
    println!("call require_file2");
    let Some(require_rel) = require_path_from_local0(vm, script_src_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    println!("call require_file3");
    if require_rel.is_empty() {
        return OpHandleResult::kOpResultSuccess;
    }

    println!("call require_file4");
    let current = (*rt).require_path_base();
    let resolved = resolve_require_path(current, &require_rel);
    println!("require_file path = {}", &resolved);
    (*rt).compile_script_file(&resolved);
    OpHandleResult::kOpResultSuccess
}
