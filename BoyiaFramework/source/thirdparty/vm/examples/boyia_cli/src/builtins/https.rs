//! Https builtin class for `boyia_cli`.
//! Requests run on `ThreadPool`; callbacks are posted back to the runtime task thread.

#![allow(dead_code)]

use super::r#async::{
    make_callback_info, register_runner_builtin_class, runner_from_class, schedule_task, value_to_string,
    AsyncBuiltinResult, CallbackInfo,
};
use boyia_builtins::gen_builtin_class_function;
use boyia_vm::{
    get_local_size, get_local_value, set_int_result, BoyiaValue, NativePtr, LUintPtr, LVoid,
    OpHandleResult,
};
use reqwest::blocking::Client;
use reqwest::header::{HeaderMap, HeaderName, HeaderValue};
use serde_json::Value;
use std::time::Duration;

const DEFAULT_TIMEOUT_SECS: u64 = 30;

/// Register the Https builtin class. Stores `runner_ptr` as a BY_INT prop on the class so load/request can get the runner and schedule callbacks.
pub fn builtin_https_class<F>(vm: *mut LVoid, gen_id: &mut F, runner_ptr: *mut crate::runner::BoyiaRunner)
where
    F: FnMut(&str) -> LUintPtr,
{
    register_runner_builtin_class(vm, gen_id, runner_ptr, "Https", |class_body, vm, gen_id| unsafe {
        gen_builtin_class_function(gen_id("load"), https_load_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(
            gen_id("request"),
            https_request_impl as NativePtr,
            class_body,
            vm,
        );
    });
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
            Ok(text) => {
                if text.is_empty() {
                    AsyncBuiltinResult::Ok { data: None }
                } else {
                    AsyncBuiltinResult::Ok {
                        data: Some(text),
                    }
                }
            }
            Err(err) => AsyncBuiltinResult::Fail {
                message: format!("Https error: {err}"),
            },
        },
        callback,
        |r| println!("https result: {}", r.log_preview()),
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
    let runner_ptr = runner_from_class(class_val);

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
    let runner_ptr = runner_from_class(class_val);

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
