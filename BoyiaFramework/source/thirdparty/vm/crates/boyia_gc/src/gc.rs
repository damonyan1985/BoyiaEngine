//! Boyia GC implementation. Strict port of BoyiaGC.cpp.
//! Mark-sweep with optional compaction. Uses boyia_vm and boyia_memory APIs directly (no GcCallbacks).

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use boyia_memory::{
    alloc_memory_chunk, create_memory_cache, destroy_memory_cache, fast_free, fast_malloc,
    free_memory_chunk, migrate_memory,
};
use boyia_vm::{
    get_function_count, get_global_table, get_local_stack, get_native_helper_result, get_native_result,
    get_runtime_from_vm, get_string_buffer_from_body, iterate_micro_task, BoyiaFunction, BoyiaValue,
    BuiltinId, LInt, LIntPtr, LVoid, Runtime, ValueType, LUint8
};
use std::ptr;

const MIGRATE_SIZE: usize = 6 * 1024;
const K_BOYIA_REF_PAGE_SIZE: LInt = 64 * 1024;

/// GC color bits in object header (bits 16..18 of mParamCount for Boyia objects).
const K_BOYIA_GC_MASK: LInt = 0x0003;
const K_BOYIA_GC_WHITE: LInt = 0x0;
const K_BOYIA_GC_GRAY: LInt = 0x1;
const K_BOYIA_GC_BLACK: LInt = 0x2;

/// StringBufferType (C++ enum): bits in mParamCount >> 18.
const K_NATIVE_STRING_BUFFER: LInt = 1;
const K_BOYIA_STRING_BUFFER: LInt = 0;

/// Single ref in the used-refs list (BoyiaRef in C++).
#[repr(C)]
pub(crate) struct BoyiaRef {
    pub mAddress: *mut LVoid,
    pub mType: ValueType,
    pub mNext: *mut BoyiaRef,
}

/// Head and tail of the used refs list (UsedRefs in C++).
#[repr(C)]
pub(crate) struct UsedRefs {
    pub mBegin: *mut BoyiaRef,
    pub mEnd: *mut BoyiaRef,
}

/// GC state (BoyiaGc in C++).
#[repr(C)]
pub struct BoyiaGc {
    pub(crate) mUsedRefs: UsedRefs,
    pub mRefCache: *mut LVoid,
    pub mBoyiaVM: *mut LVoid,
    pub mMigrates: [*mut LVoid; MIGRATE_SIZE],
}

fn is_object_invalid(fun: *const BoyiaFunction) -> bool {
    if fun.is_null() {
        return true;
    }
    let count = unsafe { (*fun).mParamCount };
    ((count >> 16) & K_BOYIA_GC_MASK) == K_BOYIA_GC_WHITE
}

fn is_native_string(fun: *const BoyiaFunction) -> bool {
    if fun.is_null() {
        return false;
    }
    ((unsafe { (*fun).mParamCount } >> 18) & K_BOYIA_GC_MASK) == K_NATIVE_STRING_BUFFER
}

fn is_boyia_string(fun: *const BoyiaFunction) -> bool {
    if fun.is_null() {
        return false;
    }
    ((unsafe { (*fun).mParamCount } >> 18) & K_BOYIA_GC_MASK) == K_BOYIA_STRING_BUFFER
}

#[allow(dead_code)]
fn migrate_flag(fun: *const BoyiaFunction) -> LInt {
    if fun.is_null() {
        return 0;
    }
    (unsafe { (*fun).mParamCount } >> 20) & 0x1
}

#[allow(dead_code)]
fn set_migrate_flag(fun: *mut BoyiaFunction) {
    if fun.is_null() {
        return;
    }
    unsafe {
        (*fun).mParamCount |= 0x1 << 20;
    }
}

// Native/platform hooks (extern in C++; stubs by default).
#[inline]
unsafe fn mark_native_object(_addr: LIntPtr, _color: LInt) {}

#[inline]
fn native_object_flag(_addr: *mut LVoid) -> LInt {
    K_BOYIA_GC_WHITE
}

#[inline]
unsafe fn native_delete(_addr: *mut LVoid) {}

#[inline]
unsafe fn free_buffer(_ptr: *mut LUint8, _size: LInt) {
    let slice = ptr::slice_from_raw_parts_mut(_ptr, _size as usize);
    let _ = Box::from_raw(slice);
}

unsafe fn allocate_ref(gc: *mut BoyiaGc) -> *mut BoyiaRef {
    let cache = (*gc).mRefCache;
    if cache.is_null() {
        return ptr::null_mut();
    }
    let p = alloc_memory_chunk(cache) as *mut BoyiaRef;
    if !p.is_null() {
        (*p).mNext = ptr::null_mut();
    }
    p
}

unsafe fn free_ref(ref_ptr: *mut BoyiaRef, gc: *mut BoyiaGc) {
    if ref_ptr.is_null() || (*gc).mRefCache.is_null() {
        return;
    }
    free_memory_chunk(ref_ptr as *mut LVoid, (*gc).mRefCache);
}

/// CreateGC: allocate GC state and ref cache. Returns gc pointer; store in runtime.
pub unsafe fn create_gc(vm: *mut LVoid) -> *mut BoyiaGc {
    let gc = fast_malloc(std::mem::size_of::<BoyiaGc>() as LInt) as *mut BoyiaGc;
    if gc.is_null() {
        return ptr::null_mut();
    }
    ptr::write(
        gc,
        BoyiaGc {
            mUsedRefs: UsedRefs {
                mBegin: ptr::null_mut(),
                mEnd: ptr::null_mut(),
            },
            mRefCache: ptr::null_mut(),
            mBoyiaVM: vm,
            mMigrates: [ptr::null_mut(); MIGRATE_SIZE],
        },
    );
    (*gc).mRefCache = create_memory_cache(std::mem::size_of::<BoyiaRef>() as LInt, K_BOYIA_REF_PAGE_SIZE);
    if (*gc).mRefCache.is_null() {
        fast_free(gc as *mut LVoid);
        return ptr::null_mut();
    }
    gc
}

/// DestroyGC: destroy ref cache and free GC. Call when shutting down VM.
pub unsafe fn destroy_gc(gc: *mut BoyiaGc) {
    if gc.is_null() {
        return;
    }
    if !(*gc).mRefCache.is_null() {
        destroy_memory_cache((*gc).mRefCache);
        (*gc).mRefCache = ptr::null_mut();
    }
    fast_free(gc as *mut LVoid);
}

/// GCAppendRef: register a heap object with the GC (BY_CLASS or BY_NAVCLASS).
/// GC and VM are obtained from the given [Runtime].
pub fn gc_append_ref(address: *mut LVoid, type_: ValueType, runtime: &dyn Runtime) {
    let gc = runtime.gc_ptr() as *mut BoyiaGc;
    let _vm = runtime.vm_ptr();
    if gc.is_null() {
        return;
    }
    let ref_ptr = unsafe { allocate_ref(gc) };
    if ref_ptr.is_null() {
        return;
    }
    unsafe {
        (*ref_ptr).mAddress = address;
        (*ref_ptr).mType = type_;
        (*ref_ptr).mNext = ptr::null_mut();

        let refs = &mut (*gc).mUsedRefs;
        if !refs.mBegin.is_null() {
            (*refs.mEnd).mNext = ref_ptr;
        } else {
            refs.mBegin = ref_ptr;
        }
        refs.mEnd = ref_ptr;
    }
}

// --- Mark / sweep (match BoyiaGC.cpp: MarkValue, MarkObjectProps, etc.) ---

unsafe fn mark_value(value: *mut BoyiaValue) {
    if value.is_null() {
        return;
    }
    let vt = (*value).mValueType;
    if vt == ValueType::BY_NAVCLASS {
        let addr = (*value).mValue.mIntVal;
        mark_native_object(addr, K_BOYIA_GC_BLACK);
        return;
    }
    if vt != ValueType::BY_CLASS && vt != ValueType::BY_PROP_FUNC {
        return;
    }
    mark_object_props(value);
}

unsafe fn mark_object_props(value: *const BoyiaValue) {
    let fun_ptr = if (*value).mValueType == ValueType::BY_CLASS {
        (*value).mValue.mObj.mPtr as *mut BoyiaFunction
    } else {
        (*value).mValue.mObj.mSuper as *mut BoyiaFunction
    };
    if fun_ptr.is_null() || !is_object_invalid(fun_ptr) {
        return;
    }
    (*fun_ptr).mParamCount |= K_BOYIA_GC_GRAY << 16;
    let fun = fun_ptr as *const BoyiaFunction;
    let size = (*fun).mParamSize;
    let params = (*fun).mParams;
    for i in 0..size {
        if !params.is_null() {
            mark_value(params.add(i as usize));
        }
    }
    (*fun_ptr).mParamCount |= K_BOYIA_GC_BLACK << 16;
}

unsafe fn mark_value_table(table: *mut BoyiaValue, size: LInt) {
    if table.is_null() || size <= 0 {
        return;
    }
    for i in 0..(size as usize) {
        mark_value(table.add(i));
    }
}

fn is_invalid_object(ref_ptr: *const BoyiaRef) -> bool {
    let ty = unsafe { (*ref_ptr).mType };
    if ty == ValueType::BY_CLASS {
        let fun = unsafe { (*ref_ptr).mAddress as *const BoyiaFunction };
        return is_object_invalid(fun);
    }
    if ty != ValueType::BY_NAVCLASS {
        return false;
    }
    let flag = native_object_flag(unsafe { (*ref_ptr).mAddress });
    if flag != K_BOYIA_GC_WHITE {
        return false;
    }
    true
}

unsafe fn delete_object(ref_ptr: *const BoyiaRef, vm: *mut LVoid) {
    let ty = (*ref_ptr).mType;
    if ty == ValueType::BY_NAVCLASS {
        native_delete((*ref_ptr).mAddress);
        return;
    }
    if ty != ValueType::BY_CLASS {
        return;
    }
    let obj_body = (*ref_ptr).mAddress as *const BoyiaFunction;
    let kclass = (*obj_body).mFuncBody as *const BoyiaValue;
    let class_id = if kclass.is_null() {
        0
    } else {
        (*kclass).mNameKey
    };
    if class_id == BuiltinId::kBoyiaString.as_key() {
        let buffer = get_string_buffer_from_body(obj_body as *mut BoyiaFunction);
        if !buffer.is_null() {
            let buf = &*buffer;
            if is_native_string(obj_body) {
                free_buffer(buf.mPtr as *mut LUint8, buf.mLen);
            } else if is_boyia_string(obj_body) {
                let rt = get_runtime_from_vm(vm);
                if !rt.is_null() {
                    (*rt).delete_data(buf.mPtr as *mut LVoid);
                }
            }
        }
    }
    let rt = get_runtime_from_vm(vm);
    if !rt.is_null() {
        (*rt).delete_data((*ref_ptr).mAddress);
    }
}

fn reset_boyia_object(fun: *mut BoyiaFunction) {
    if fun.is_null() {
        return;
    }
    unsafe {
        let high = (*fun).mParamCount >> 18;
        let low = get_function_count(fun);
        (*fun).mParamCount = (high << 18) | low;
    }
}

unsafe fn reset_memory_color(gc: *mut BoyiaGc) {
    if gc.is_null() {
        return;
    }
    let vm = (*gc).mBoyiaVM;
    let mut ref_ptr = (*gc).mUsedRefs.mBegin;
    while !ref_ptr.is_null() {
        let ty = (*ref_ptr).mType;
        if ty == ValueType::BY_NAVCLASS {
            mark_native_object((*ref_ptr).mAddress as LIntPtr, K_BOYIA_GC_WHITE);
        } else if ty == ValueType::BY_CLASS {
            reset_boyia_object((*ref_ptr).mAddress as *mut BoyiaFunction);
        }
        ref_ptr = (*ref_ptr).mNext;
    }
    let mut table_addr: LIntPtr = 0;
    let mut size: LInt = 0;
    get_global_table(&mut table_addr, &mut size, vm);
    if table_addr == 0 || size <= 0 {
        return;
    }
    let stack = table_addr as *const BoyiaValue;
    for i in 0..(size as usize) {
        let val = &*stack.add(i);
        if val.mValueType == ValueType::BY_CLASS {
            let ptr = val.mValue.mObj.mPtr as *mut BoyiaFunction;
            reset_boyia_object(ptr);
        }
    }
}

unsafe fn clear_all_garbage(gc: *mut BoyiaGc, vm: *mut LVoid) {
    if gc.is_null() {
        return;
    }
    let refs = &mut (*gc).mUsedRefs;
    let mut prev = refs.mBegin;
    while !prev.is_null() {
        if is_invalid_object(prev) {
            delete_object(prev, vm);
            refs.mBegin = (*prev).mNext;
            free_ref(prev, gc);
            prev = refs.mBegin;
        } else {
            break;
        }
    }
    if prev.is_null() {
        refs.mEnd = ptr::null_mut();
        return;
    }
    let mut current = (*prev).mNext;
    while !current.is_null() {
        if is_invalid_object(current) {
            delete_object(current, vm);
            (*prev).mNext = (*current).mNext;
            free_ref(current, gc);
            current = (*prev).mNext;
        } else {
            prev = current;
            current = (*current).mNext;
        }
    }
    refs.mEnd = prev;
}

/// Mark persistent roots from the runtime [boyia_vm::GlobalList] via [mark_value]. Anonymous entries only mark capture cells, not the wrapper value. Called from [gc_collect_garbage].
unsafe fn mark_persistent(vm: *mut LVoid) {
    let rt = get_runtime_from_vm(vm);
    if rt.is_null() {
        return;
    }
    (*rt).iterate_persistent(&mut |value: *mut BoyiaValue| {
        if !value.is_null() {
            mark_value(value);
        }
    });
}

/// GCollectGarbage: reset color, mark from roots (global, local stack, micro task, native result, persistent), then sweep. Match GCollectGarbage in BoyiaGC.cpp.
pub unsafe fn gc_collect_garbage(gc: *mut BoyiaGc, vm: *mut LVoid) {
    if gc.is_null() {
        return;
    }
    reset_memory_color(gc);
    let mut table_addr: LIntPtr = 0;
    let mut size: LInt = 0;
    get_global_table(&mut table_addr, &mut size, vm);
    mark_value_table(table_addr as *mut BoyiaValue, size);

    let mut stack_addr: LIntPtr = 0;
    let mut op_stack_addr: LIntPtr = 0;
    let mut op_size: LInt = 0;
    let mut ptr = vm;
    loop {
        let next = get_local_stack(
            &mut stack_addr,
            &mut size,
            &mut op_stack_addr,
            &mut op_size,
            vm,
            ptr,
        );
        mark_value_table(stack_addr as *mut BoyiaValue, size);
        mark_value_table(op_stack_addr as *mut BoyiaValue, op_size);
        if next.is_null() {
            break;
        }
        ptr = next;
    }

    let mut result = ptr::null_mut::<BoyiaValue>();
    let mut obj = ptr::null_mut::<BoyiaValue>();
    ptr = vm;
    loop {
        ptr = iterate_micro_task(&mut obj, &mut result, vm, ptr);
        if !result.is_null() {
            mark_value(result);
        }
        if !obj.is_null() {
            mark_value(obj);
        }
        if ptr.is_null() {
            break;
        }
    }

    let result_val = get_native_result(vm);
    if !result_val.is_null() {
        mark_value(result_val);
    }
    let helper_val = get_native_helper_result(vm);
    if !helper_val.is_null() {
        mark_value(helper_val);
    }

    mark_persistent(vm);

    clear_all_garbage(gc, vm);
}

// --- Compaction (migrate) ---

#[allow(dead_code)]
unsafe fn reset_migrate_address(
    value: *mut BoyiaValue,
    _migrate_index: &LInt,
    to_pool: *mut LVoid,
    fun: *const BoyiaFunction,
    gc: *mut BoyiaGc,
    vm: *mut LVoid,
) {
    if value.is_null() || fun.is_null() || gc.is_null() {
        return;
    }
    let index = migrate_flag(fun);
    let new_addr = (*gc).mMigrates[index as usize];
    let vt = (*value).mValueType;
    if vt == ValueType::BY_CLASS {
        (*value).mValue.mObj.mPtr = new_addr as LIntPtr;
        let class_id = if !(*fun).mParams.is_null() {
            (*(*fun).mParams).mNameKey
        } else {
            0
        };
        if class_id == BuiltinId::kBoyiaString.as_key() && is_boyia_string(fun) {
            let buffer = get_string_buffer_from_body(fun as *mut BoyiaFunction);
            if !buffer.is_null() {
                let buf = &mut *buffer;
                let from_pool = {
                    let rt = get_runtime_from_vm(vm);
                    if rt.is_null() {
                        return;
                    }
                    (*rt).memory_pool()
                };
                let new_ptr = migrate_memory(buf.mPtr as *mut LVoid, from_pool, to_pool);
                buf.mPtr = new_ptr as *mut boyia_vm::LInt8;
            }
        }
    } else {
        (*value).mValue.mObj.mSuper = new_addr as LIntPtr;
    }
}

#[allow(dead_code)]
unsafe fn migrate_object(
    value: *mut BoyiaValue,
    migrate_index: &mut LInt,
    to_pool: *mut LVoid,
    gc: *mut BoyiaGc,
    vm: *mut LVoid,
) {
    let vt = (*value).mValueType;
    let fun_ptr = if vt == ValueType::BY_CLASS {
        (*value).mValue.mObj.mPtr as *mut BoyiaFunction
    } else {
        (*value).mValue.mObj.mSuper as *mut BoyiaFunction
    };
    if fun_ptr.is_null() {
        return;
    }
    if migrate_flag(fun_ptr) == 0 {
        let rt = get_runtime_from_vm(vm);
        let from_pool = if rt.is_null() {
            ptr::null_mut()
        } else {
            (*rt).memory_pool()
        };
        let new_addr = migrate_memory(fun_ptr as *mut LVoid, from_pool, to_pool);
        (*gc).mMigrates[*migrate_index as usize] = new_addr;
        set_migrate_flag(fun_ptr);
        *migrate_index += 1;
    }
    reset_migrate_address(value, migrate_index, to_pool, fun_ptr, gc, vm);
}

#[allow(dead_code)]
unsafe fn migrate_value(
    value: *mut BoyiaValue,
    migrate_index: &mut LInt,
    to_pool: *mut LVoid,
    gc: *mut BoyiaGc,
    vm: *mut LVoid,
) {
    if value.is_null() {
        return;
    }
    let vt = (*value).mValueType;
    let fun_ptr = if vt == ValueType::BY_CLASS {
        (*value).mValue.mObj.mPtr as *mut BoyiaFunction
    } else if vt == ValueType::BY_PROP_FUNC {
        (*value).mValue.mObj.mSuper as *mut BoyiaFunction
    } else {
        ptr::null_mut()
    };
    if !fun_ptr.is_null() && migrate_flag(fun_ptr) == 0 {
        let fun = fun_ptr as *const BoyiaFunction;
        let size = (*fun).mParamSize;
        let params = (*fun).mParams;
        for i in 0..size {
            if !params.is_null() {
                migrate_value(params.add(i as usize), migrate_index, to_pool, gc, vm);
            }
        }
    }
    migrate_object(value, migrate_index, to_pool, gc, vm);
}

#[allow(dead_code)]
unsafe fn migrate_value_table(
    table: *mut BoyiaValue,
    size: LInt,
    migrate_index: &mut LInt,
    to_pool: *mut LVoid,
    gc: *mut BoyiaGc,
    vm: *mut LVoid,
) {
    if table.is_null() || size <= 0 {
        return;
    }
    for i in 0..(size as usize) {
        migrate_value(table.add(i), migrate_index, to_pool, gc, vm);
    }
}

fn reset_gc_ref(gc: *mut BoyiaGc) {
    if gc.is_null() {
        return;
    }
    let mut ref_ptr = unsafe { (*gc).mUsedRefs.mBegin };
    while !ref_ptr.is_null() {
        if unsafe { (*ref_ptr).mType } == ValueType::BY_CLASS {
            let fun = unsafe { (*ref_ptr).mAddress as *const BoyiaFunction };
            if migrate_flag(fun) != 0 {
                let index = migrate_flag(fun);
                unsafe {
                    (*ref_ptr).mAddress = (*gc).mMigrates[index as usize];
                }
            }
        }
        ref_ptr = unsafe { (*ref_ptr).mNext };
    }
}

/// CompactMemory: migrate live objects to a new pool and switch. Match CompactMemory in BoyiaGC.cpp.
#[allow(dead_code)]
pub(crate) unsafe fn compact_memory(gc: *mut BoyiaGc) {
    if gc.is_null() {
        return;
    }
    let vm = (*gc).mBoyiaVM;
    let rt = get_runtime_from_vm(vm);
    if rt.is_null() {
        return;
    }
    let mut migrate_index: LInt = 0;
    ptr::write_bytes((*gc).mMigrates.as_mut_ptr(), 0, MIGRATE_SIZE * std::mem::size_of::<*mut LVoid>());
    let to_pool = (*rt).create_runtime_to_memory(vm);
    if to_pool.is_null() {
        return;
    }
    let mut table_addr: LIntPtr = 0;
    let mut size: LInt = 0;
    get_global_table(&mut table_addr, &mut size, vm);
    migrate_value_table(
        table_addr as *mut BoyiaValue,
        size,
        &mut migrate_index,
        to_pool,
        gc,
        vm,
    );
    let mut stack_addr: LIntPtr = 0;
    let mut op_stack_addr: LIntPtr = 0;
    let mut op_size: LInt = 0;
    let mut ptr = vm;
    loop {
        let next = get_local_stack(
            &mut stack_addr,
            &mut size,
            &mut op_stack_addr,
            &mut op_size,
            vm,
            ptr,
        );
        migrate_value_table(
            stack_addr as *mut BoyiaValue,
            size,
            &mut migrate_index,
            to_pool,
            gc,
            vm,
        );
        migrate_value_table(
            op_stack_addr as *mut BoyiaValue,
            op_size,
            &mut migrate_index,
            to_pool,
            gc,
            vm,
        );
        if next.is_null() {
            break;
        }
        ptr = next;
    }
    (*rt).update_runtime_memory(to_pool, vm);
    reset_gc_ref(gc);
}
