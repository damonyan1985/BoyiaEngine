//! Https builtin class for `boyia_cli`.
//! Requests run on `ThreadPool`; callbacks are posted back to the runtime task thread.

#![allow(dead_code)]

use crate::runner::BoyiaRunner;
use boyia_builtins::gen_builtin_class_function;
use boyia_runtime::BoyiaRuntime;
use boyia_vm::{
    create_global_class, create_native_string, get_local_size, get_local_value, get_runtime_from_vm,
    get_string_buffer, native_call_impl, set_int_result, BoyiaClass, BoyiaFunction, BoyiaValue,
    Global, K_BOYIA_NULL, NativePtr, RealValue, Runtime, ValueType, LInt, LIntPtr, LUintPtr, LVoid,
    OpHandleResult,
};
use reqwest::blocking::Client;
use reqwest::header::{HeaderMap, HeaderName, HeaderValue};
use serde_json::Value;
use std::str;
use std::time::Duration;

const DEFAULT_TIMEOUT_SECS: u64 = 30;
/// Slot index of the runner pointer (BY_INT) in the Https class params.
const HTTPS_CLASS_RUNNER_PROP_INDEX: usize = 0;

#[derive(Clone, Copy)]
struct CallbackInfo {
    name_key: LUintPtr,
    value_type: ValueType,
    func_ptr: LIntPtr,
    /// Persistent Global node for the object; [Global::value] holds the [BoyiaValue] whose mValue.mObj.mPtr is the object address from the callback.
    object_global: *mut Global,
}
unsafe impl Send for CallbackInfo {}

/// Register the Https builtin class. Stores `runner_ptr` as a BY_INT prop on the class so load/request can get the runner and schedule callbacks.
pub fn builtin_https_class<F>(vm: *mut LVoid, gen_id: &mut F, runner_ptr: *mut crate::runner::BoyiaRunner)
where
    F: FnMut(&str) -> LUintPtr,
{
    if vm.is_null() {
        return;
    }

    let https_key = gen_id("Https");
    let class_ref = unsafe { create_global_class(https_key, vm) } as *mut BoyiaValue;
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

        gen_builtin_class_function(gen_id("load"), https_load_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(
            gen_id("request"),
            https_request_impl as NativePtr,
            class_body,
            vm,
        );
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

fn schedule_request(
    runner_ptr: *mut BoyiaRunner,
    url: String,
    params: Option<String>,
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
            let body = match execute_https_request(&url, params.as_deref()) {
                Ok(text) => text,
                Err(err) => format!("Https error: {err}"),
            };
            let _ = runtime_handle.post_task(move |runtime| unsafe {
                callback_string(body, callback, runtime.as_mut());
                runtime.consume_micro_task();
            });
        })
        .is_ok()
}

fn execute_https_request(url: &str, params: Option<&str>) -> Result<String, String> {
    let client = Client::builder()
        .timeout(Duration::from_secs(DEFAULT_TIMEOUT_SECS))
        .build()
        .map_err(|err| err.to_string())?;

    let mut method = "get".to_string();
    let mut headers = HeaderMap::new();
    let mut body = None::<String>;

    if let Some(params) = params {
        let json: Value = serde_json::from_str(params).map_err(|err| err.to_string())?;

        if let Some(method_value) = json.get("method").and_then(Value::as_str) {
            method = method_value.to_ascii_lowercase();
        }

        if let Some(header_obj) = json.get("headers").and_then(Value::as_object) {
            for (name, value) in header_obj {
                let Some(value) = value.as_str() else {
                    continue;
                };
                let header_name =
                    HeaderName::from_bytes(name.as_bytes()).map_err(|err| err.to_string())?;
                let header_value = HeaderValue::from_str(value).map_err(|err| err.to_string())?;
                headers.insert(header_name, header_value);
            }
        }

        body = json
            .get("body")
            .and_then(Value::as_str)
            .map(ToOwned::to_owned);
    }

    let mut request = match method.as_str() {
        "post" => client.post(url),
        _ => client.get(url),
    };

    if !headers.is_empty() {
        request = request.headers(headers);
    }

    if let Some(body) = body {
        request = request.body(body);
    }

    request
        .send()
        .map_err(|err| err.to_string())?
        .text()
        .map_err(|err| err.to_string())
}

unsafe fn callback_string(result: String, callback: CallbackInfo, runtime: &mut BoyiaRuntime) {
    println!("https result: {}", result);
    
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

unsafe fn https_load_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    if size < 3 {
        return OpHandleResult::kOpResultEnd;
    }

    let class_val = get_local_value(size - 1, vm) as *const BoyiaValue;
    let class_body = (*class_val).mValue.mObj.mPtr as *mut BoyiaFunction;
    let runner_ptr = (*class_body)
        .mParams
        .add(HTTPS_CLASS_RUNNER_PROP_INDEX)
        .read()
        .mValue
        .mIntVal as *mut BoyiaRunner;

    let url_val = get_local_value(1, vm) as *const BoyiaValue;
    let callback_val = get_local_value(2, vm) as *const BoyiaValue;

    let Some(url) = value_to_string(url_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    if callback_val.is_null() {
        return OpHandleResult::kOpResultEnd;
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

    let callback = CallbackInfo {
        name_key: (*callback_val).mNameKey,
        value_type: (*callback_val).mValueType,
        func_ptr: (*callback_val).mValue.mObj.mPtr,
        object_global,
    };

    let scheduled = schedule_request(runner_ptr, url, None, callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}

unsafe fn https_request_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    if size < 4 {
        return OpHandleResult::kOpResultEnd;
    }

    let class_val = get_local_value(size - 1, vm) as *const BoyiaValue;
    let class_body = (*class_val).mValue.mObj.mPtr as *mut BoyiaFunction;
    let runner_ptr = (*class_body)
        .mParams
        .add(HTTPS_CLASS_RUNNER_PROP_INDEX)
        .read()
        .mValue
        .mIntVal as *mut BoyiaRunner;

    let url_val = get_local_value(1, vm) as *const BoyiaValue;
    let params_val = get_local_value(2, vm) as *const BoyiaValue;
    let callback_val = get_local_value(3, vm) as *const BoyiaValue;

    let Some(url) = value_to_string(url_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(params) = value_to_string(params_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    if callback_val.is_null() {
        return OpHandleResult::kOpResultEnd;
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

    let callback = CallbackInfo {
        name_key: (*callback_val).mNameKey,
        value_type: (*callback_val).mValueType,
        func_ptr: (*callback_val).mValue.mObj.mPtr,
        object_global,
    };

    let scheduled = schedule_request(runner_ptr, url, Some(params), callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}
