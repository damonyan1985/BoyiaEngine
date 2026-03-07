//! Boyia GC. Port of BoyiaGC.cpp.
//! Mark-sweep garbage collector with optional compaction (migrate).
//! VM integration is via [GcCallbacks]; uses [boyia_vm] types (BoyiaValue, BoyiaFunction, etc.).

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use boyia_memory::{
    alloc_memory_chunk, create_memory_cache, destroy_memory_cache, fast_free, fast_malloc,
    free_memory_chunk,
};
use boyia_vm::{BoyiaFunction, BoyiaStr, BoyiaValue, BuiltinId, ValueType};
use std::ptr;

// Re-export types for ABI compatibility with C++ BoyiaGC
pub use boyia_memory::{LInt, LIntPtr, LUintPtr, LVoid};
pub type LUint8 = u8;
pub type LBool = LInt;

const MIGRATE_SIZE: usize = 6 * 1024;
const K_BOYIA_REF_PAGE_SIZE: LInt = 64 * 1024;

/// GC color bits in object header (bits 16..18 of mParamCount for Boyia objects).
pub const K_BOYIA_GC_MASK: LInt = 0x0003;
pub const K_BOYIA_GC_WHITE: LInt = 0x0;
pub const K_BOYIA_GC_GRAY: LInt = 0x1;
pub const K_BOYIA_GC_BLACK: LInt = 0x2;

/// StringBufferType (C++ enum): bits in mParamCount >> 18.
const K_NATIVE_STRING_BUFFER: LInt = 1;
const K_BOYIA_STRING_BUFFER: LInt = 0;

/// Single ref in the used-refs list (BoyiaRef in C++).
#[repr(C)]
pub struct BoyiaRef {
    pub mAddress: *mut LVoid,
    pub mType: LUint8,
    pub mNext: *mut BoyiaRef,
}

/// Head and tail of the used refs list (UsedRefs in C++).
#[repr(C)]
pub struct UsedRefs {
    pub mBegin: *mut BoyiaRef,
    pub mEnd: *mut BoyiaRef,
}

/// GC state (BoyiaGc in C++).
#[repr(C)]
pub struct BoyiaGc {
    pub mUsedRefs: UsedRefs,
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

fn migrate_flag(fun: *const BoyiaFunction) -> LInt {
    if fun.is_null() {
        return 0;
    }
    (unsafe { (*fun).mParamCount } >> 20) & 0x1
}

fn set_migrate_flag(fun: *mut BoyiaFunction) {
    if fun.is_null() {
        return;
    }
    unsafe {
        (*fun).mParamCount |= 0x1 << 20;
    }
}

/// Callbacks the VM must provide for mark/sweep and compaction.
pub trait GcCallbacks {
    /// Global table: (base pointer, element count).
    fn get_global_table(&self, vm: *mut LVoid) -> (*mut BoyiaValue, LInt);
    /// Local stack: (stack_ptr, size, op_stack_ptr, op_size). Returns next frame ptr or null.
    fn get_local_stack(
        &self,
        vm: *mut LVoid,
        frame_ptr: *mut LVoid,
    ) -> Option<(*mut BoyiaValue, LInt, *mut BoyiaValue, LInt, *mut LVoid)>;
    /// Micro task iteration: (result value ptr, obj value ptr). Returns next ptr or null.
    fn iterate_micro_task(
        &self,
        vm: *mut LVoid,
        prev: *mut LVoid,
    ) -> Option<(*mut BoyiaValue, *mut BoyiaValue, *mut LVoid)>;
    /// Native result value (single value to mark).
    fn get_native_result(&self, vm: *mut LVoid) -> *mut BoyiaValue;
    /// Native helper result value.
    fn get_native_helper_result(&self, vm: *mut LVoid) -> *mut BoyiaValue;
    /// Delete block from VM pool (BoyiaDelete).
    fn delete_from_pool(&self, addr: *mut LVoid, vm: *mut LVoid);
    /// Migrate block from current pool to to_pool; return new address.
    fn migrate_memory(&self, src: *mut LVoid, to_pool: *mut LVoid, vm: *mut LVoid) -> *mut LVoid;
    /// Create the "to" pool for compaction; return pool ptr.
    fn create_runtime_to_memory(&self, vm: *mut LVoid) -> *mut LVoid;
    /// After compaction, set runtime to use to_pool.
    fn update_runtime_memory(&self, to_pool: *mut LVoid, vm: *mut LVoid);
    /// Mark native object with color (bits 16..18).
    fn mark_native_object(&self, address: LIntPtr, color: LInt);
    /// Get native object's current GC flag.
    fn native_object_flag(&self, address: *mut LVoid) -> LInt;
    /// Delete native object (NativeDelete).
    fn native_delete(&self, address: *mut LVoid);
    /// String buffer from function body: &body->mParams[1].mValue.mStrVal.
    fn get_string_buffer_from_body(&self, body: *mut BoyiaFunction) -> *mut BoyiaStr;
    /// Free buffer (platform free for native string).
    fn free_buffer(&self, ptr: *mut LVoid);
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
pub unsafe fn gc_append_ref(address: *mut LVoid, type_: LUint8, _vm: *mut LVoid, gc: *mut BoyiaGc) {
    if gc.is_null() {
        return;
    }
    let ref_ptr = allocate_ref(gc);
    if ref_ptr.is_null() {
        return;
    }
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

// --- Mark / sweep (use GcCallbacks) ---

fn mark_value<C: GcCallbacks>(value: *mut BoyiaValue, callbacks: &C, vm: *mut LVoid) {
    if value.is_null() {
        return;
    }
    let vt = unsafe { (*value).mValueType };
    if vt == ValueType::BY_NAVCLASS {
        let addr = unsafe { (*value).mValue.mIntVal };
        callbacks.mark_native_object(addr, K_BOYIA_GC_BLACK);
        return;
    }
    if vt != ValueType::BY_CLASS && vt != ValueType::BY_PROP_FUNC {
        return;
    }
    mark_object_props(value, callbacks, vm);
}

fn mark_object_props<C: GcCallbacks>(
    value: *const BoyiaValue,
    callbacks: &C,
    vm: *mut LVoid,
) {
    let fun_ptr = if unsafe { (*value).mValueType } == ValueType::BY_CLASS {
        (unsafe { (*value).mValue.mObj.mPtr }) as *mut BoyiaFunction
    } else {
        (unsafe { (*value).mValue.mObj.mSuper }) as *mut BoyiaFunction
    };
    if fun_ptr.is_null() || !is_object_invalid(fun_ptr) {
        return;
    }
    unsafe {
        (*fun_ptr).mParamCount |= K_BOYIA_GC_GRAY << 16;
    }
    let fun = fun_ptr as *const BoyiaFunction;
    let size = unsafe { (*fun).mParamSize };
    let params = unsafe { (*fun).mParams };
    for i in 0..size {
        if !params.is_null() {
            mark_value(unsafe { params.add(i as usize) }, callbacks, vm);
        }
    }
    unsafe {
        (*fun_ptr).mParamCount |= K_BOYIA_GC_BLACK << 16;
    }
}

fn mark_value_table<C: GcCallbacks>(
    table: *mut BoyiaValue,
    size: LInt,
    callbacks: &C,
    vm: *mut LVoid,
) {
    if table.is_null() || size <= 0 {
        return;
    }
    for i in 0..(size as usize) {
        mark_value(unsafe { table.add(i) }, callbacks, vm);
    }
}

fn is_invalid_object<C: GcCallbacks>(ref_ptr: *const BoyiaRef, callbacks: &C) -> bool {
    let ty = unsafe { (*ref_ptr).mType };
    if ty == ValueType::BY_CLASS as u8 {
        let fun = unsafe { (*ref_ptr).mAddress as *const BoyiaFunction };
        return is_object_invalid(fun);
    }
    if ty != ValueType::BY_NAVCLASS as u8 {
        return false;
    }
    let flag = callbacks.native_object_flag(unsafe { (*ref_ptr).mAddress });
    if flag != K_BOYIA_GC_WHITE {
        return false;
    }
    true
}

fn delete_object<C: GcCallbacks>(
    ref_ptr: *const BoyiaRef,
    callbacks: &C,
    vm: *mut LVoid,
) {
    let ty = unsafe { (*ref_ptr).mType };
    if ty == ValueType::BY_NAVCLASS as u8 {
        callbacks.native_delete(unsafe { (*ref_ptr).mAddress });
        return;
    }
    if ty != ValueType::BY_CLASS as u8 {
        return;
    }
    let obj_body = unsafe { (*ref_ptr).mAddress as *const BoyiaFunction };
    let kclass = unsafe { (*obj_body).mFuncBody as *const BoyiaValue };
    let class_id = if kclass.is_null() {
        0
    } else {
        unsafe { (*kclass).mNameKey }
    };
    if class_id == BuiltinId::kBoyiaString.as_key() {
        let buffer = callbacks.get_string_buffer_from_body(obj_body as *mut BoyiaFunction);
        if !buffer.is_null() {
            let buf = unsafe { &*buffer };
            if is_native_string(obj_body) {
                callbacks.free_buffer(buf.mPtr as *mut LVoid);
            } else if is_boyia_string(obj_body) {
                callbacks.delete_from_pool(buf.mPtr as *mut LVoid, vm);
            }
        }
    }
    callbacks.delete_from_pool(unsafe { (*ref_ptr).mAddress }, vm);
}

fn reset_boyia_object(fun: *mut BoyiaFunction) {
    if fun.is_null() {
        return;
    }
    unsafe {
        let high = (*fun).mParamCount >> 18;
        let low = (*fun).mParamCount & 0x0000_FFFF;
        (*fun).mParamCount = (high << 18) | low;
    }
}

fn reset_memory_color<C: GcCallbacks>(gc: *mut BoyiaGc, callbacks: &C) {
    if gc.is_null() {
        return;
    }
    let mut ref_ptr = unsafe { (*gc).mUsedRefs.mBegin };
    while !ref_ptr.is_null() {
        let ty = unsafe { (*ref_ptr).mType };
        if ty == ValueType::BY_NAVCLASS as u8 {
            callbacks.mark_native_object(unsafe { (*ref_ptr).mAddress as LIntPtr }, K_BOYIA_GC_WHITE);
        } else if ty == ValueType::BY_CLASS as u8 {
            reset_boyia_object(unsafe { (*ref_ptr).mAddress as *mut BoyiaFunction });
        }
        ref_ptr = unsafe { (*ref_ptr).mNext };
    }
    let (stack_addr, size) = callbacks.get_global_table(unsafe { (*gc).mBoyiaVM });
    if stack_addr.is_null() || size <= 0 {
        return;
    }
    let stack = stack_addr as *const BoyiaValue;
    for i in 0..(size as usize) {
        let val = unsafe { &*stack.add(i) };
        if val.mValueType == ValueType::BY_CLASS {
            let ptr = unsafe { val.mValue.mObj.mPtr } as *mut BoyiaFunction;
            reset_boyia_object(ptr);
        }
    }
}

fn clear_all_garbage<C: GcCallbacks>(gc: *mut BoyiaGc, vm: *mut LVoid, callbacks: &C) {
    if gc.is_null() {
        return;
    }
    let refs = unsafe { &mut (*gc).mUsedRefs };
    let mut prev = refs.mBegin;
    while !prev.is_null() {
        if is_invalid_object(prev, callbacks) {
            delete_object(prev, callbacks, vm);
            refs.mBegin = unsafe { (*prev).mNext };
            unsafe { free_ref(prev, gc) };
            prev = refs.mBegin;
        } else {
            break;
        }
    }
    if prev.is_null() {
        refs.mEnd = ptr::null_mut();
        return;
    }
    let mut current = unsafe { (*prev).mNext };
    while !current.is_null() {
        if is_invalid_object(current, callbacks) {
            delete_object(current, callbacks, vm);
            unsafe { (*prev).mNext = (*current).mNext };
            unsafe { free_ref(current, gc) };
            current = unsafe { (*prev).mNext };
        } else {
            prev = current;
            current = unsafe { (*current).mNext };
        }
    }
    refs.mEnd = prev;
}

/// GCollectGarbage: mark from roots, then sweep. Call with VM and implementation of [GcCallbacks].
pub fn g_collect_garbage<C: GcCallbacks>(gc: *mut BoyiaGc, vm: *mut LVoid, callbacks: &C) {
    if gc.is_null() {
        return;
    }
    reset_memory_color(gc, callbacks);
    let (global_table, size) = callbacks.get_global_table(vm);
    mark_value_table(global_table, size, callbacks, vm);

    let mut ptr = vm;
    loop {
        match callbacks.get_local_stack(vm, ptr) {
            Some((stack_addr, size, op_stack_addr, op_size, next_ptr)) => {
                mark_value_table(stack_addr, size, callbacks, vm);
                mark_value_table(op_stack_addr, op_size, callbacks, vm);
                ptr = next_ptr;
            }
            None => break,
        }
    }

    ptr = vm;
    loop {
        match callbacks.iterate_micro_task(vm, ptr) {
            Some((result, obj, next_ptr)) => {
                if !result.is_null() {
                    mark_value(result, callbacks, vm);
                }
                if !obj.is_null() {
                    mark_value(obj, callbacks, vm);
                }
                ptr = next_ptr;
            }
            None => break,
        }
    }

    let result = callbacks.get_native_result(vm);
    if !result.is_null() {
        mark_value(result, callbacks, vm);
    }
    let helper = callbacks.get_native_helper_result(vm);
    if !helper.is_null() {
        mark_value(helper, callbacks, vm);
    }

    clear_all_garbage(gc, vm, callbacks);
}

// --- Compaction (migrate) ---

fn reset_migrate_address(
    value: *mut BoyiaValue,
    _migrate_index: &LInt,
    to_pool: *mut LVoid,
    fun: *const BoyiaFunction,
    gc: *mut BoyiaGc,
    callbacks: &dyn GcCallbacks,
    vm: *mut LVoid,
) {
    if value.is_null() || fun.is_null() || gc.is_null() {
        return;
    }
    let index = migrate_flag(fun);
    let new_addr = unsafe { (*gc).mMigrates[index as usize] };
    let vt = unsafe { (*value).mValueType };
    if vt == ValueType::BY_CLASS {
        unsafe {
            (*value).mValue.mObj.mPtr = new_addr as LIntPtr;
        }
        let class_id = if !unsafe { (*fun).mParams }.is_null() {
            unsafe { (*(*fun).mParams).mNameKey }
        } else {
            0
        };
        if class_id == BuiltinId::kBoyiaString.as_key() && is_boyia_string(fun) {
            let buffer = callbacks.get_string_buffer_from_body(fun as *mut BoyiaFunction);
            if !buffer.is_null() {
                let buf = unsafe { &mut *buffer };
                let old_ptr = buf.mPtr as *mut LVoid;
                let new_ptr = callbacks.migrate_memory(old_ptr, to_pool, vm);
                buf.mPtr = new_ptr as *mut boyia_vm::LInt8;
            }
        }
    } else {
        unsafe {
            (*value).mValue.mObj.mSuper = new_addr as LIntPtr;
        }
    }
}

fn migrate_object(
    value: *mut BoyiaValue,
    migrate_index: &mut LInt,
    to_pool: *mut LVoid,
    gc: *mut BoyiaGc,
    callbacks: &dyn GcCallbacks,
    vm: *mut LVoid,
) {
    let vt = unsafe { (*value).mValueType };
    let fun_ptr = if vt == ValueType::BY_CLASS {
        (unsafe { (*value).mValue.mObj.mPtr }) as *mut BoyiaFunction
    } else {
        (unsafe { (*value).mValue.mObj.mSuper }) as *mut BoyiaFunction
    };
    if fun_ptr.is_null() {
        return;
    }
    if migrate_flag(fun_ptr) == 0 {
        let new_addr = callbacks.migrate_memory(fun_ptr as *mut LVoid, to_pool, vm);
        unsafe {
            (*gc).mMigrates[*migrate_index as usize] = new_addr;
        }
        set_migrate_flag(fun_ptr);
        *migrate_index += 1;
    }
    reset_migrate_address(value, migrate_index, to_pool, fun_ptr, gc, callbacks, vm);
}

fn migrate_value(
    value: *mut BoyiaValue,
    migrate_index: &mut LInt,
    to_pool: *mut LVoid,
    gc: *mut BoyiaGc,
    callbacks: &dyn GcCallbacks,
    vm: *mut LVoid,
) {
    if value.is_null() {
        return;
    }
    let vt = unsafe { (*value).mValueType };
    let fun_ptr = if vt == ValueType::BY_CLASS {
        (unsafe { (*value).mValue.mObj.mPtr }) as *mut BoyiaFunction
    } else if vt == ValueType::BY_PROP_FUNC {
        (unsafe { (*value).mValue.mObj.mSuper }) as *mut BoyiaFunction
    } else {
        ptr::null_mut()
    };
    if !fun_ptr.is_null() && migrate_flag(fun_ptr) == 0 {
        let fun = fun_ptr as *const BoyiaFunction;
        let size = unsafe { (*fun).mParamSize };
        let params = unsafe { (*fun).mParams };
        for i in 0..size {
            if !params.is_null() {
                migrate_value(
                    unsafe { params.add(i as usize) },
                    migrate_index,
                    to_pool,
                    gc,
                    callbacks,
                    vm,
                );
            }
        }
    }
    migrate_object(value, migrate_index, to_pool, gc, callbacks, vm);
}

fn migrate_value_table(
    table: *mut BoyiaValue,
    size: LInt,
    migrate_index: &mut LInt,
    to_pool: *mut LVoid,
    gc: *mut BoyiaGc,
    callbacks: &dyn GcCallbacks,
    vm: *mut LVoid,
) {
    if table.is_null() || size <= 0 {
        return;
    }
    for i in 0..(size as usize) {
        migrate_value(
            unsafe { table.add(i) },
            migrate_index,
            to_pool,
            gc,
            callbacks,
            vm,
        );
    }
}

fn reset_gc_ref(gc: *mut BoyiaGc) {
    if gc.is_null() {
        return;
    }
    let mut ref_ptr = unsafe { (*gc).mUsedRefs.mBegin };
    while !ref_ptr.is_null() {
        if unsafe { (*ref_ptr).mType } == ValueType::BY_CLASS as u8 {
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

/// CompactMemory: migrate live objects to a new pool and switch. Call when allocation fails after a collect.
pub fn compact_memory<C: GcCallbacks>(gc: *mut BoyiaGc, callbacks: &C) {
    if gc.is_null() {
        return;
    }
    let vm = unsafe { (*gc).mBoyiaVM };
    let mut migrate_index: LInt = 0;
    unsafe {
        ptr::write_bytes((*gc).mMigrates.as_mut_ptr(), 0, MIGRATE_SIZE);
    }
    let to_pool = callbacks.create_runtime_to_memory(vm);
    if to_pool.is_null() {
        return;
    }
    let (table_addr, size) = callbacks.get_global_table(vm);
    migrate_value_table(table_addr, size, &mut migrate_index, to_pool, gc, callbacks, vm);
    let mut ptr = vm;
    loop {
        match callbacks.get_local_stack(vm, ptr) {
            Some((stack_addr, size, op_stack_addr, op_size, next_ptr)) => {
                migrate_value_table(
                    stack_addr, size, &mut migrate_index, to_pool, gc, callbacks, vm,
                );
                migrate_value_table(
                    op_stack_addr, op_size, &mut migrate_index, to_pool, gc, callbacks, vm,
                );
                ptr = next_ptr;
            }
            None => break,
        }
    }
    callbacks.update_runtime_memory(to_pool, vm);
    reset_gc_ref(gc);
}
