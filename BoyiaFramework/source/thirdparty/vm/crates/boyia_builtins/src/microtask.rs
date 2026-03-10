//! MicroTask builtin class: task prop; init, resolve (prop).

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::{gen_builtin_class_function, gen_builtin_class_prop_function, K_BOYIA_NULL};
use boyia_vm::{
    copy_object, create_global_class, create_micro_task, get_local_size, get_local_value,
    native_call_impl, resume_micro_task, BoyiaClass, BoyiaFunction, BoyiaValue, NativePtr,
    RealValue, ValueType, LIntPtr, LUintPtr, LVoid, OpHandleResult,
};

unsafe fn micro_task_resolve_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *const BoyiaValue;
    let result = get_local_value(1, vm) as *mut BoyiaValue;
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    let task_ptr = (*fun).mParams.add(1).read().mValue.mIntVal;
    resume_micro_task(task_ptr as *mut LVoid, result, vm);
    OpHandleResult::kOpResultSuccess
}

unsafe fn micro_task_init_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    let obj = get_local_value(size - 1, vm) as *mut BoyiaValue;
    let worker = get_local_value(1, vm) as *const BoyiaValue;
    let fun = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    let task = create_micro_task(vm, obj);
    if task.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    (*fun).mParams.add(1).write(BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue {
            mIntVal: task as LIntPtr,
        },
    });
    let resolve_val = (*fun).mParams.add(0).read();
    let mut args = [*worker, resolve_val];
    let mut cb_obj = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_CLASS,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: (*worker).mValue.mObj.mSuper,
                mSuper: 0,
            },
        },
    };
    native_call_impl(args.as_mut_ptr(), 2, &mut cb_obj, vm)
}

/// Register MicroTask builtin class: task prop; init, resolve (prop).
pub fn builtin_micro_task_class<F>(vm: *mut LVoid, gen_id: &mut F)
where
    F: FnMut(&str) -> LUintPtr,
{
    eprintln!("[builtin_micro_task_class] 1");
    if vm.is_null() {
        return;
    }
    let micro_task_key = gen_id("MicroTask");
    eprintln!("[builtin_micro_task_class] 2 create_global_class");
    let class_ref = unsafe { create_global_class(micro_task_key, vm) } as *mut BoyiaValue;
    if class_ref.is_null() {
        eprintln!("[builtin_micro_task_class] class_ref null");
        return;
    }
    eprintln!("[builtin_micro_task_class] 3");
    unsafe {
        (*class_ref).mValue.mObj.mSuper = K_BOYIA_NULL;
        let class_body = (*class_ref).mValue.mObj.mPtr as *mut BoyiaFunction;
        if class_body.is_null() || (*class_body).mParams.is_null() {
            eprintln!("[builtin_micro_task_class] class_body or mParams null");
            return;
        }
        eprintln!("[builtin_micro_task_class] 4 task prop");
        let task_slot = (*class_body).mParams.add((*class_body).mParamSize as usize);
        (*task_slot).mValueType = ValueType::BY_INT;
        (*task_slot).mNameKey = gen_id("task");
        (*task_slot).mValue.mIntVal = K_BOYIA_NULL;
        (*class_body).mParamSize += 1;
        eprintln!("[builtin_micro_task_class] 5 init, resolve");
        gen_builtin_class_function(gen_id("init"), micro_task_init_impl as NativePtr, class_body, vm);
        gen_builtin_class_prop_function(gen_id("resolve"), micro_task_resolve_impl as NativePtr, class_body, vm);
    }
    eprintln!("[builtin_micro_task_class] 6 done");
}

/// Create a MicroTask instance. Match CreateMicroTaskObject.
pub unsafe fn create_micro_task_object(vm: *mut LVoid, micro_task_class_key: LUintPtr) -> *mut LVoid {
    copy_object(micro_task_class_key, 32, vm)
}
