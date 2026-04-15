//! Shared helpers for thread-pool → runtime async builtins (File, Https, Zip).

use crate::runner::BoyiaRunner;
use boyia_runtime::BoyiaRuntime;
use boyia_vm::{
    copy_object, create_native_string, create_string_object, gen_identifier_from_str, get_function_count,
    native_call_impl, value_copy, vector_params_grow_if_full, BoyiaClass, BoyiaFunction, BoyiaStr,
    BoyiaValue, BuiltinId, Global, K_BOYIA_NULL, RealValue, Runtime, ValueType, LInt, LInt8, LIntPtr,
    LUintPtr, LVoid,
};
use std::str;

/// Slot in [BoyiaFunction::mParams] where `File` / `Https` store `BoyiaRunner*` as `BY_INT`.
pub const BUILTIN_CLASS_RUNNER_PROP_INDEX: usize = 0;

/// Result posted to script callbacks: a Map with `status` (`"ok"` | `"fail"`),
/// optional `data` (only when ok and non-empty payload), optional `message` (only when fail).
#[derive(Debug)]
pub enum AsyncBuiltinResult {
    Ok { data: Option<String> },
    Fail { message: String },
}

impl AsyncBuiltinResult {
    /// Short string for logging (e.g. HTTPS response body or error text).
    pub fn log_preview(&self) -> &str {
        match self {
            AsyncBuiltinResult::Ok { data: Some(d) } => d.as_str(),
            AsyncBuiltinResult::Ok { data: None } => "",
            AsyncBuiltinResult::Fail { message } => message.as_str(),
        }
    }
}

/// Append a `runner` [ValueType::BY_INT] param on `class_body` at [BoyiaFunction::mParamSize] (stores `runner_ptr`).
/// Returns false if `class_body` or [BoyiaFunction::mParams] is null.
pub unsafe fn install_runner_param_slot<F>(
    class_body: *mut BoyiaFunction,
    gen_id: &mut F,
    runner_ptr: *mut BoyiaRunner,
) -> bool
where
    F: FnMut(&str) -> LUintPtr,
{
    if class_body.is_null() || (*class_body).mParams.is_null() {
        return false;
    }
    let runner_slot = (*class_body).mParams.add((*class_body).mParamSize as usize);
    (*runner_slot).mValueType = ValueType::BY_INT;
    (*runner_slot).mNameKey = gen_id("runner");
    (*runner_slot).mValue.mIntVal = runner_ptr as LIntPtr;
    (*class_body).mParamSize += 1;
    true
}

/// Read the runner pointer from the builtin class value (`get_local_value(size - 1, vm)` in native methods).
pub unsafe fn runner_from_class(class_val: *const BoyiaValue) -> *mut BoyiaRunner {
    let class_body = (*class_val).mValue.mObj.mPtr as *mut BoyiaFunction;
    (*class_body)
        .mParams
        .add(BUILTIN_CLASS_RUNNER_PROP_INDEX)
        .read()
        .mValue
        .mIntVal as *mut BoyiaRunner
}

/// Persistent callback + object handle for posting a result Map back onto the runtime thread.
#[derive(Clone, Copy)]
pub struct CallbackInfo {
    pub name_key: LUintPtr,
    pub value_type: ValueType,
    pub func_ptr: LIntPtr,
    /// Persistent Global node for the object; [Global::value] holds the [BoyiaValue] whose mValue.mObj.mPtr is the object address from the callback.
    pub object_global: *mut Global,
}

unsafe impl Send for CallbackInfo {}

/// Run `work` on the runner's thread pool; then post to the runtime thread: `before_callback`,
/// then [callback_async_result]. Returns whether the task was queued on the pool.
///
/// `on_missing_runner` runs only when the runner pointer cannot yield a runtime handle / pool.
pub fn schedule_task<W, H, M>(
    runner_ptr: *mut BoyiaRunner,
    work: W,
    callback: CallbackInfo,
    before_callback: H,
    on_missing_runner: M,
) -> bool
where
    W: FnOnce() -> AsyncBuiltinResult + Send + 'static,
    H: FnOnce(&AsyncBuiltinResult) + Send + 'static,
    M: FnOnce(),
{
    let Some((runtime_handle, thread_pool_weak)) =
        (unsafe { BoyiaRunner::get_handle_and_pool_from_ptr(runner_ptr) })
    else {
        on_missing_runner();
        return false;
    };
    let Some(thread_pool) = thread_pool_weak.upgrade() else {
        return false;
    };

    thread_pool
        .post_task(move || {
            let body = work();
            let _ = runtime_handle.post_task(move |runtime| {
                before_callback(&body);
                unsafe {
                    callback_async_result(body, callback, runtime.as_mut());
                }
                runtime.consume_micro_task();
            });
        })
        .is_ok()
}

pub fn value_to_string(value: *const BoyiaValue) -> Option<String> {
    if value.is_null() {
        return None;
    }
    let str_ref = unsafe { boyia_vm::get_string_buffer(value) };
    if str_ref.is_null() {
        return None;
    }
    let len = unsafe { (*str_ref).mLen.max(0) as usize };
    let ptr = unsafe { (*str_ref).mPtr as *const u8 };
    let slice = unsafe { std::slice::from_raw_parts(ptr, len) };
    str::from_utf8(slice).ok().map(ToOwned::to_owned)
}

pub unsafe fn make_callback_info(vm: *mut LVoid, callback_val: *const BoyiaValue) -> Option<CallbackInfo> {
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
                mSuper: K_BOYIA_NULL,
            },
        },
    };
    let object_global = {
        let rt = boyia_vm::get_runtime_from_vm(vm);
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

unsafe fn native_string_value(vm: *mut LVoid, s: &str) -> Option<BoyiaValue> {
    if s.is_empty() {
        let body = create_string_object(std::ptr::null_mut(), 0, vm);
        if body.is_null() {
            return None;
        }
        return Some(BoyiaValue {
            mNameKey: BuiltinId::kBoyiaString.as_key(),
            mValueType: ValueType::BY_CLASS,
            mValue: RealValue {
                mObj: BoyiaClass {
                    mPtr: body as LIntPtr,
                    mSuper: K_BOYIA_NULL,
                },
            },
        });
    }
    let boxed = s.as_bytes().to_vec().into_boxed_slice();
    let len = boxed.len() as LInt;
    let ptr = Box::into_raw(boxed) as *mut u8 as *mut LInt8;
    let mut value = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue { mIntVal: 0 },
    };
    create_native_string(&mut value, ptr, len, vm);
    if value.mValue.mObj.mPtr == K_BOYIA_NULL {
        return None;
    }
    Some(value)
}

unsafe fn map_put_str_key(vm: *mut LVoid, map_obj: *mut BoyiaValue, key: &str, val: &BoyiaValue) -> bool {
    let fun = (*map_obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return false;
    }
    let kb = key.as_bytes();
    let bstr = BoyiaStr {
        mPtr: kb.as_ptr() as *mut LInt8,
        mLen: kb.len() as LInt,
    };
    let key_id = gen_identifier_from_str(vm, &bstr);
    let cap = get_function_count(fun);
    if (*fun).mParamSize >= cap && !vector_params_grow_if_full(fun, vm) {
        return false;
    }
    let slot = (*fun).mParams.add((*fun).mParamSize as usize);
    value_copy(slot, val);
    (*slot).mNameKey = key_id;
    (*fun).mParamSize += 1;
    true
}

/// Build a Map `BoyiaValue` for script: `status`, and optionally `data` / `message` per rules above.
pub unsafe fn build_async_result_map(vm: *mut LVoid, r: &AsyncBuiltinResult) -> Option<BoyiaValue> {
    let raw = copy_object(BuiltinId::kBoyiaMap.as_key(), 32, vm);
    if raw.is_null() {
        return None;
    }
    let mut map_val = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_CLASS,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: raw as LIntPtr,
                mSuper: K_BOYIA_NULL,
            },
        },
    };

    let status_s = match r {
        AsyncBuiltinResult::Ok { .. } => "ok",
        AsyncBuiltinResult::Fail { .. } => "fail",
    };

    println!("build_async_result_map status: {}", status_s);
    let status_val = native_string_value(vm, status_s)?;
    if !map_put_str_key(vm, &mut map_val, "status", &status_val) {
        return None;
    }

    match r {
        AsyncBuiltinResult::Ok { data: Some(d) } => {
            let dv = native_string_value(vm, d)?;
            if !map_put_str_key(vm, &mut map_val, "data", &dv) {
                return None;
            }
        }
        AsyncBuiltinResult::Ok { data: None } => {}
        AsyncBuiltinResult::Fail { message } => {
            
            let mv = native_string_value(vm, message)?;
            if !map_put_str_key(vm, &mut map_val, "message", &mv) {
                return None;
            }
        }
    }

    Some(map_val)
}

/// Invoke `callback(result_map)` on the runtime thread; releases persistent object if any.
pub unsafe fn callback_async_result(
    result: AsyncBuiltinResult,
    callback: CallbackInfo,
    runtime: &mut BoyiaRuntime,
) {
    let vm = runtime.vm();
    let Some(map_val) = build_async_result_map(vm, &result) else {
        if !callback.object_global.is_null() {
            runtime.remove_persistent(callback.object_global);
        }
        return;
    };

    let cb_fun = callback.func_ptr as *mut BoyiaFunction;
    if cb_fun.is_null() {
        if !callback.object_global.is_null() {
            runtime.remove_persistent(callback.object_global);
        }
        return;
    }

    let obj_super = if callback.object_global.is_null() {
        K_BOYIA_NULL
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

    let mut args = [callback_value, map_val];
    if !(*cb_fun).mParams.is_null() {
        args[1].mNameKey = (*(*cb_fun).mParams).mNameKey;
    }

    let mut obj = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_CLASS,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: obj_super,
                mSuper: K_BOYIA_NULL,
            },
        },
    };
    native_call_impl(args.as_mut_ptr(), 2, &mut obj, vm);

    if !callback.object_global.is_null() {
        runtime.remove_persistent(callback.object_global);
    }
}
