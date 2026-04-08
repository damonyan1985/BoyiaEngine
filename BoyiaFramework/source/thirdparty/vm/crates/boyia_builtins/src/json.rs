//! Json builtin: `parse` (JSON string → Map / Array / String / number / null), `toString` (Boyia value → JSON string).
//! Aligns with `BoyiaLib.cpp` `jsonParseWithCJSON` / `toJsonString`; Map keys round-trip via [Runtime::name_for_identifier].

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::gen_builtin_class_function;
use boyia_vm::{
    copy_object, create_global_class, create_native_string, create_string_object, gen_identifier_from_str,
    get_boyia_class_id, get_function_count, get_local_size, get_local_value, get_runtime_from_vm,
    get_string_buffer, name_for_identifier, set_native_result, value_copy, vector_params_grow_if_full,
    BoyiaClass, BoyiaFunction, BoyiaStr, BoyiaValue, BuiltinId, K_BOYIA_NULL, LIntPtr, NativePtr,
    RealValue, ValueType, LInt, LInt8, LUintPtr, LVoid, OpHandleResult,
};
use serde_json::{Map as JsonMap, Number, Value as JsonValue};

unsafe fn boyia_str_to_slice<'a>(v: *const BoyiaValue) -> Option<&'a [u8]> {
    if v.is_null() {
        return None;
    }
    let buf = get_string_buffer(v);
    if buf.is_null() {
        return None;
    }
    let len = (*buf).mLen.max(0) as usize;
    let ptr = (*buf).mPtr as *const u8;
    Some(std::slice::from_raw_parts(ptr, len))
}

unsafe fn alloc_string_value(vm: *mut LVoid, s: &str) -> Option<BoyiaValue> {
    let rt = get_runtime_from_vm(vm);
    if rt.is_null() {
        return None;
    }
    let bytes = s.as_bytes();
    let len = bytes.len() as LInt;
    if len <= 0 {
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
    let p = (*rt).new_data(len) as *mut u8;
    if p.is_null() {
        return None;
    }
    std::ptr::copy_nonoverlapping(bytes.as_ptr(), p, bytes.len());
    let body = create_string_object(p as *mut LInt8, len, vm);
    if body.is_null() {
        (*rt).delete_data(p as *mut LVoid);
        return None;
    }
    Some(BoyiaValue {
        mNameKey: BuiltinId::kBoyiaString.as_key(),
        mValueType: ValueType::BY_CLASS,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: body as LIntPtr,
                mSuper: K_BOYIA_NULL,
            },
        },
    })
}

unsafe fn map_put(vm: *mut LVoid, map_obj: *mut BoyiaValue, key: &str, val: &BoyiaValue) -> Result<(), ()> {
    let fun = (*map_obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return Err(());
    }
    let kb = key.as_bytes();
    let bstr = BoyiaStr {
        mPtr: kb.as_ptr() as *mut LInt8,
        mLen: kb.len() as LInt,
    };
    let key_id = gen_identifier_from_str(vm, &bstr);
    let cap = get_function_count(fun);
    if (*fun).mParamSize >= cap && !vector_params_grow_if_full(fun, vm) {
        return Err(());
    }
    let slot = (*fun).mParams.add((*fun).mParamSize as usize);
    value_copy(slot, val);
    (*slot).mNameKey = key_id;
    (*fun).mParamSize += 1;
    Ok(())
}

unsafe fn array_add(vm: *mut LVoid, arr_obj: *mut BoyiaValue, val: &BoyiaValue) -> Result<(), ()> {
    let fun = (*arr_obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        return Err(());
    }
    let cap = get_function_count(fun);
    if (*fun).mParamSize >= cap && !vector_params_grow_if_full(fun, vm) {
        return Err(());
    }
    if (*fun).mParams.is_null() {
        return Err(());
    }
    let dst = (*fun).mParams.add((*fun).mParamSize as usize);
    value_copy(dst, val);
    (*dst).mNameKey = 0;
    (*fun).mParamSize += 1;
    Ok(())
}

unsafe fn serde_to_boyia(vm: *mut LVoid, j: &JsonValue) -> Result<BoyiaValue, ()> {
    Ok(match j {
        JsonValue::Null => BoyiaValue {
            mNameKey: 0,
            mValueType: ValueType::BY_INT,
            mValue: RealValue {
                mIntVal: K_BOYIA_NULL,
            },
        },
        JsonValue::Bool(b) => BoyiaValue {
            mNameKey: 0,
            mValueType: ValueType::BY_INT,
            mValue: RealValue {
                mIntVal: if *b { 1 } else { 0 },
            },
        },
        JsonValue::Number(n) => {
            if let Some(i) = n.as_i64() {
                if i >= isize::MIN as i64 && i <= isize::MAX as i64 {
                    BoyiaValue {
                        mNameKey: 0,
                        mValueType: ValueType::BY_INT,
                        mValue: RealValue {
                            mIntVal: i as LIntPtr,
                        },
                    }
                } else {
                    BoyiaValue {
                        mNameKey: 0,
                        mValueType: ValueType::BY_REAL,
                        mValue: RealValue {
                            mRealVal: n.as_f64().unwrap_or(0.0),
                        },
                    }
                }
            } else {
                BoyiaValue {
                    mNameKey: 0,
                    mValueType: ValueType::BY_REAL,
                    mValue: RealValue {
                        mRealVal: n.as_f64().unwrap_or(0.0),
                    },
                }
            }
        }
        JsonValue::String(s) => alloc_string_value(vm, s).ok_or(())?,
        JsonValue::Array(a) => {
            let map_key = BuiltinId::kBoyiaArray.as_key();
            let raw = copy_object(map_key, 32, vm);
            if raw.is_null() {
                return Err(());
            }
            let mut out = BoyiaValue {
                mNameKey: 0,
                mValueType: ValueType::BY_CLASS,
                mValue: RealValue {
                    mObj: BoyiaClass {
                        mPtr: raw as LIntPtr,
                        mSuper: K_BOYIA_NULL,
                    },
                },
            };
            for item in a {
                let elem = serde_to_boyia(vm, item)?;
                array_add(vm, &mut out, &elem)?;
            }
            out
        }
        JsonValue::Object(o) => {
            let map_key = BuiltinId::kBoyiaMap.as_key();
            let raw = copy_object(map_key, 32, vm);
            if raw.is_null() {
                return Err(());
            }
            let mut out = BoyiaValue {
                mNameKey: 0,
                mValueType: ValueType::BY_CLASS,
                mValue: RealValue {
                    mObj: BoyiaClass {
                        mPtr: raw as LIntPtr,
                        mSuper: K_BOYIA_NULL,
                    },
                },
            };
            for (k, v) in o {
                let elem = serde_to_boyia(vm, v)?;
                map_put(vm, &mut out, k, &elem)?;
            }
            out
        }
    })
}

fn json_number_from_int(i: LIntPtr) -> JsonValue {
    JsonValue::Number(Number::from(i))
}

fn json_number_from_real(f: f64) -> JsonValue {
    JsonValue::Number(Number::from_f64(f).unwrap_or_else(|| Number::from(0)))
}

unsafe fn boyia_to_serde(vm: *mut LVoid, v: *const BoyiaValue) -> Result<JsonValue, ()> {
    if v.is_null() {
        return Err(());
    }
    match (*v).mValueType {
        ValueType::BY_INT => {
            if (*v).mValue.mIntVal == K_BOYIA_NULL {
                Ok(JsonValue::Null)
            } else {
                Ok(json_number_from_int((*v).mValue.mIntVal))
            }
        }
        ValueType::BY_REAL => Ok(json_number_from_real((*v).mValue.mRealVal)),
        ValueType::BY_CLASS => {
            let cid = get_boyia_class_id(v);
            if cid == BuiltinId::kBoyiaString.as_key() {
                let slice = boyia_str_to_slice(v).ok_or(())?;
                let s = std::str::from_utf8(slice).map_err(|_| ())?;
                Ok(JsonValue::String(s.to_string()))
            } else if cid == BuiltinId::kBoyiaArray.as_key() {
                let fun = (*v).mValue.mObj.mPtr as *const BoyiaFunction;
                if fun.is_null() || (*fun).mParams.is_null() {
                    return Err(());
                }
                let mut vec = Vec::new();
                for i in 0..(*fun).mParamSize {
                    let prop = (*fun).mParams.add(i as usize);
                    if matches!(
                        (*prop).mValueType,
                        ValueType::BY_NAV_FUNC | ValueType::BY_FUNC | ValueType::BY_PROP_FUNC
                    ) {
                        continue;
                    }
                    vec.push(boyia_to_serde(vm, prop)?);
                }
                Ok(JsonValue::Array(vec))
            } else if cid == BuiltinId::kBoyiaMap.as_key() {
                let fun = (*v).mValue.mObj.mPtr as *const BoyiaFunction;
                if fun.is_null() || (*fun).mParams.is_null() {
                    return Err(());
                }
                let mut m = JsonMap::new();
                for i in 0..(*fun).mParamSize {
                    let prop = (*fun).mParams.add(i as usize);
                    if matches!(
                        (*prop).mValueType,
                        ValueType::BY_NAV_FUNC | ValueType::BY_FUNC | ValueType::BY_PROP_FUNC
                    ) {
                        continue;
                    }
                    let key = name_for_identifier(vm, (*prop).mNameKey).ok_or(())?;
                    m.insert(key, boyia_to_serde(vm, prop)?);
                }
                Ok(JsonValue::Object(m))
            } else {
                Err(())
            }
        }
        _ => Err(()),
    }
}

unsafe fn json_parse_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    if size < 3 {
        return OpHandleResult::kOpResultEnd;
    }
    let json_val = get_local_value(1, vm) as *const BoyiaValue;
    let Some(slice) = boyia_str_to_slice(json_val) else {
        return OpHandleResult::kOpResultEnd;
    };
    let text = match std::str::from_utf8(slice) {
        Ok(t) => t,
        Err(_) => return OpHandleResult::kOpResultEnd,
    };
    let parsed: JsonValue = match serde_json::from_str(text) {
        Ok(j) => j,
        Err(_) => {
            return OpHandleResult::kOpResultEnd;
        }
    };
    let mut out = match serde_to_boyia(vm, &parsed) {
        Ok(o) => o,
        Err(_) => return OpHandleResult::kOpResultEnd,
    };
    set_native_result(&mut out as *mut BoyiaValue as *mut LVoid, vm);
    OpHandleResult::kOpResultSuccess
}

unsafe fn json_to_string_impl(vm: *mut LVoid) -> OpHandleResult {
    let size = get_local_size(vm);
    if size < 3 {
        return OpHandleResult::kOpResultEnd;
    }
    let obj = get_local_value(1, vm) as *const BoyiaValue;
    let sj = match boyia_to_serde(vm, obj) {
        Ok(j) => j,
        Err(_) => return OpHandleResult::kOpResultEnd,
    };
    let s = match serde_json::to_string(&sj) {
        Ok(s) => s,
        Err(_) => return OpHandleResult::kOpResultEnd,
    };
    let boxed = s.into_bytes().into_boxed_slice();
    let len = boxed.len() as LInt;
    let ptr = Box::into_raw(boxed) as *mut u8 as *mut LInt8;
    let mut val = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_INT,
        mValue: RealValue { mIntVal: 0 },
    };
    create_native_string(&mut val, ptr, len, vm);
    set_native_result(&mut val as *mut BoyiaValue as *mut LVoid, vm);
    OpHandleResult::kOpResultSuccess
}

/// Register global class `Json` with `parse` and `toString` (static-style methods on class body).
pub fn builtin_json_class<F>(vm: *mut LVoid, gen_id: &mut F)
where
    F: FnMut(&str) -> LUintPtr,
{
    if vm.is_null() {
        return;
    }
    let json_key = gen_id("Json");
    let class_ref = unsafe { create_global_class(json_key, vm) } as *mut BoyiaValue;
    if class_ref.is_null() {
        return;
    }
    unsafe {
        (*class_ref).mValue.mObj.mSuper = K_BOYIA_NULL;
        let class_body = (*class_ref).mValue.mObj.mPtr as *mut BoyiaFunction;
        if class_body.is_null() || (*class_body).mParams.is_null() {
            return;
        }
        gen_builtin_class_function(gen_id("parse"), json_parse_impl as NativePtr, class_body, vm);
        gen_builtin_class_function(gen_id("toString"), json_to_string_impl as NativePtr, class_body, vm);
    }
}
