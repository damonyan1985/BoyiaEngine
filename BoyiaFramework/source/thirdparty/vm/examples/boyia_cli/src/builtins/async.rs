//! Shared helpers for thread-pool → runtime async builtins (File, Https).

use crate::runner::BoyiaRunner;
use boyia_runtime::BoyiaRuntime;
use boyia_vm::{
    create_native_string, get_runtime_from_vm, get_string_buffer, native_call_impl, BoyiaClass,
    BoyiaFunction, BoyiaValue, Global, RealValue, Runtime, ValueType, LInt, LIntPtr, LUintPtr, LVoid,
};
use std::str;

/// Persistent callback + object handle for posting a string result back onto the runtime thread.
#[derive(Clone, Copy)]
pub struct CallbackInfo {
    pub name_key: LUintPtr,
    pub value_type: ValueType,
    pub func_ptr: LIntPtr,
    /// Persistent Global node for the object; [Global::value] holds the [BoyiaValue] whose mValue.mObj.mPtr is the object address from the callback.
    pub object_global: *mut Global,
}

unsafe impl Send for CallbackInfo {}

/// Run `work` on the runner's thread pool; then post to the runtime thread: `before_callback_string`
/// (e.g. logging), then [callback_string]. Returns whether the task was queued on the pool.
///
/// `on_missing_runner` runs only when the runner pointer cannot yield a runtime handle / pool
/// (same moment `File.read` used to log a diagnostic).
pub fn schedule_task<W, H, M>(
    runner_ptr: *mut BoyiaRunner,
    work: W,
    callback: CallbackInfo,
    before_callback_string: H,
    on_missing_runner: M,
) -> bool
where
    W: FnOnce() -> String + Send + 'static,
    H: FnOnce(&str) + Send + 'static,
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
                before_callback_string(&body);
                unsafe {
                    callback_string(body, callback, runtime.as_mut());
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
    let str_ref = unsafe { get_string_buffer(value) };
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

/// Invoke `callback(result_string)` on the runtime thread; releases persistent object if any.
pub unsafe fn callback_string(result: String, callback: CallbackInfo, runtime: &mut BoyiaRuntime) {
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
