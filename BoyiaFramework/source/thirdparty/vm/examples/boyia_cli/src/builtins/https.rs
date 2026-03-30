//! Https builtin class for `boyia_cli`.
//! Requests run on `ThreadPool`; callbacks are posted back to the runtime task thread.

#![allow(dead_code)]

use super::r#async::{make_callback_info, schedule_task, value_to_string, CallbackInfo};
use boyia_builtins::gen_builtin_class_function;
use boyia_vm::{
    create_global_class, get_local_size, get_local_value, set_int_result, BoyiaFunction, BoyiaValue,
    K_BOYIA_NULL, NativePtr, ValueType, LIntPtr, LUintPtr, LVoid, OpHandleResult,
};
use reqwest::blocking::Client;
use reqwest::header::{HeaderMap, HeaderName, HeaderValue};
use serde_json::Value;
use std::time::Duration;

const DEFAULT_TIMEOUT_SECS: u64 = 30;
/// Slot index of the runner pointer (BY_INT) in the Https class params.
const HTTPS_CLASS_RUNNER_PROP_INDEX: usize = 0;

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

fn schedule_request(
    runner_ptr: *mut crate::runner::BoyiaRunner,
    url: String,
    params: Option<String>,
    callback: CallbackInfo,
) -> bool {
    schedule_task(
        runner_ptr,
        move || match execute_https_request(&url, params.as_deref()) {
            Ok(text) => text,
            Err(err) => format!("Https error: {err}"),
        },
        callback,
        |body| println!("https result: {}", body),
        || (),
    )
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
        .mIntVal as *mut crate::runner::BoyiaRunner;

    let url_val = get_local_value(1, vm) as *const BoyiaValue;
    let callback_val = get_local_value(2, vm) as *const BoyiaValue;

    let Some(url) = value_to_string(url_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(callback) = make_callback_info(vm, callback_val) else {
        return OpHandleResult::kOpResultEnd;
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
        .mIntVal as *mut crate::runner::BoyiaRunner;

    let url_val = get_local_value(1, vm) as *const BoyiaValue;
    let params_val = get_local_value(2, vm) as *const BoyiaValue;
    let callback_val = get_local_value(3, vm) as *const BoyiaValue;

    let Some(url) = value_to_string(url_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(params) = value_to_string(params_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    let Some(callback) = make_callback_info(vm, callback_val) else {
        return OpHandleResult::kOpResultEnd;
    };

    let scheduled = schedule_request(runner_ptr, url, Some(params), callback);
    set_int_result(if scheduled { 1 } else { 0 }, vm);
    OpHandleResult::kOpResultSuccess
}
