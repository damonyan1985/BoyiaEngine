//! Inline cache for property/method lookup. Internal to boyia_vm only (not re-exported).
//! Port of BoyiaValue.cpp CreateInlineCache, AddPropInlineCache, AddFunInlineCache, GetInlineCache.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::types::*;
use std::mem;
use std::ptr;

/// CreateInlineCache: allocate cache, mSize = 0.
pub(crate) unsafe fn create_inline_cache() -> *mut InlineCache {
    let cache = crate::fast_malloc(mem::size_of::<InlineCache>() as LInt) as *mut InlineCache;
    if cache.is_null() {
        return ptr::null_mut();
    }
    ptr::write(
        cache,
        InlineCache {
            mItems: mem::zeroed(),
            mSize: 0,
        },
    );
    cache
}

/// AddPropInlineCache: add (klass, property_index) entry.
pub(crate) unsafe fn add_prop_inline_cache(cache: *mut InlineCache, klass: *mut BoyiaValue, index: LInt) {
    if cache.is_null() {
        return;
    }
    let size = (*cache).mSize;
    if size < MAX_INLINE_CACHE as LInt {
        let item = (*cache).mItems.as_mut_ptr().add(size as usize);
        (*item).mClass = klass;
        (*item).mIndex = index as LIntPtr;
        (*item).mType = CACHE_PROP;
        (*cache).mSize = size + 1;
    }
}

/// AddFunInlineCache: add (klass, method_value_ptr) entry.
pub(crate) unsafe fn add_fun_inline_cache(
    cache: *mut InlineCache,
    klass: *mut BoyiaValue,
    fun: *mut BoyiaValue,
) {
    if cache.is_null() {
        return;
    }
    let size = (*cache).mSize;
    if size < MAX_INLINE_CACHE as LInt {
        let item = (*cache).mItems.as_mut_ptr().add(size as usize);
        (*item).mClass = klass;
        (*item).mIndex = fun as LIntPtr;
        (*item).mType = CACHE_METHOD;
        (*cache).mSize = size + 1;
    }
}

/// GetInlineCache: lookup by object's class; return cached property slot or method value, or null.
pub(crate) unsafe fn get_inline_cache(
    cache: *mut InlineCache,
    obj: *const BoyiaValue,
) -> *mut BoyiaValue {
    if cache.is_null() || obj.is_null() {
        return ptr::null_mut();
    }
    if (*obj).mValueType != ValueType::BY_CLASS {
        return ptr::null_mut();
    }
    let fun = (*obj).mValue.mObj.mPtr as *const BoyiaFunction;
    if fun.is_null() {
        return ptr::null_mut();
    }
    let klass = (*fun).mFuncBody as *const BoyiaValue;
    let mut index: LInt = 0;
    while index < (*cache).mSize {
        let item = (*cache).mItems.as_ptr().add(index as usize);
        if (*item).mClass == klass as *mut BoyiaValue {
            return match (*item).mType {
                CACHE_PROP => (*fun).mParams.add((*item).mIndex as usize),
                CACHE_METHOD => (*item).mIndex as *mut BoyiaValue,
                _ => ptr::null_mut(),
            };
        }
        index += 1;
    }
    ptr::null_mut()
}
