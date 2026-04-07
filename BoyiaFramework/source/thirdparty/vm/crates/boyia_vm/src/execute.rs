//! Instruction execution: main loop and handlers. Port of BoyiaCore.cpp ExecInstruction / ExecuteCode.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

use crate::core::{
    alloc_micro_task, clone_anonym_boyia_function_for_push_arg, compare_value, copy_object,
    create_const_string, create_exec_state, create_fun_val, create_micro_task_object,
    destroy_exec_state, find_global, free_micro_task, get_boyia_class_id, get_boyia_number,
    get_local_value, get_runtime_from_vm, init_function, is_boyia_number, local_push,
    micro_task_class_key, set_int_result, set_native_result, string_add, switch_exec_state,
    value_copy, value_copy_no_name, value_copy_with_key, vector_params_grow_if_full,
};
use crate::inlinecache::{
    add_fun_inline_cache, add_prop_inline_cache, create_inline_cache, get_inline_cache,
};
use crate::types::*;
use std::alloc::{alloc, Layout};
use std::ptr;

#[inline]
fn op_handle_result_from_i32(r: LInt) -> OpHandleResult {
    match r as i32 {
        0 => OpHandleResult::kOpResultEnd,
        2 => OpHandleResult::kOpResultJumpFun,
        _ => OpHandleResult::kOpResultSuccess,
    }
}

/// Operand side for get_op_value (OpLeft / OpRight).
#[derive(Clone, Copy)]
pub enum OpSide {
    OpLeft,
    OpRight,
}

/// Find the value of a variable. Strictly matches GetVal in BoyiaCore.cpp.
/// Order: (1) obj scope (this/super per BuiltinId), (2) local variables, (3) global vars, (4) FindObjProp on mClass.
#[inline]
unsafe fn get_val(key: LUintPtr, vm: *mut BoyiaVM) -> *mut BoyiaValue {
    println!("[get_val] key={}", key);
    if vm.is_null() || (*vm).mEState.is_null() {
        return ptr::null_mut();
    }
    let e = (*vm).mEState;

    /* First, see if has obj scope (BuiltinId: kBoyiaThis=1, kBoyiaSuper=2) */
    if key == BuiltinId::kBoyiaThis.as_key() {
        return &mut (*e).mStackFrame.mClass as *mut BoyiaValue;
    }
    if key == BuiltinId::kBoyiaSuper.as_key() {
        let super_ptr = (*e).mStackFrame.mClass.mValue.mObj.mSuper;
        return super_ptr as *mut BoyiaValue;
    }

    /* second, see if it's a local variable (idx > start: first element is the function itself) */
    // if (*e).mFrameIndex > 0 {
    //     let start = (*e).mExecStack.as_ptr().add((*e).mFrameIndex as usize - 1).read().mLValSize;
    //     let mut idx = (*e).mStackFrame.mLValSize - 1;
    //     while idx > start {
    //         if (*e).mLocals.as_ptr().add(idx as usize).read().mNameKey == key {
    //             println!("get local value success");
    //             return (*e).mLocals.as_mut_ptr().add(idx as usize);
    //         }
    //         idx -= 1;
    //     }
    // }

    /* otherwise, try global vars */
    let val = find_global(key, vm);
    if !val.is_null() {
        return val;
    }

    /* fallback: FindObjProp on current class (no instruction => no inline cache) */
    find_obj_prop(
        &(*e).mStackFrame.mClass as *const BoyiaValue,
        key,
        ptr::null_mut(),
        vm,
    )
}

/// `mLocals[start + frame_offset]` for current frame; `start` is previous frame's mLValSize. Offset 0 is callee function slot (not used for OP_LOCAL loads). Params start at offset 1.
#[inline]
unsafe fn get_local_ptr_by_frame_offset(vm: *mut BoyiaVM, frame_offset: LIntPtr) -> *mut BoyiaValue {
    let e = (*vm).mEState;
    if e.is_null() || (*e).mFrameIndex <= 0 {
        return ptr::null_mut();
    }
    let fi = (*e).mFrameIndex as usize - 1;
    let start = (*e).mExecStack[fi].mLValSize as isize;
    let idx = start + frame_offset as isize;
    if idx < 0 {
        return ptr::null_mut();
    }
    let idx = idx as usize;
    if idx >= (*e).mStackFrame.mLValSize as usize || idx >= NUM_LOCAL_VARS {
        return ptr::null_mut();
    }
    (*e).mLocals.as_mut_ptr().add(idx)
}

#[inline]
unsafe fn get_capture_ptr(vm: *mut BoyiaVM, capture_idx: LIntPtr) -> *mut BoyiaValue {
    if vm.is_null() {
        return ptr::null_mut();
    }
    let val = get_local_value(0, vm as *mut LVoid) as *mut BoyiaValue;
    if val.is_null() {
        return ptr::null_mut();
    }
    let fun = (*val).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() || (*fun).mParams.is_null() {
        return ptr::null_mut();
    }
    let cc = (*fun).mCaptureCount as LIntPtr;
    println!("get capture count: {}", cc);
    if capture_idx < 0 || capture_idx >= cc {
        return ptr::null_mut();
    }
    let base = (*fun).mParamSize as usize;
    (*fun).mParams.add(base + capture_idx as usize)
}

unsafe fn find_local_by_name_key_in_current_frame(vm: *mut BoyiaVM, key: LUintPtr) -> *mut BoyiaValue {
    if vm.is_null() || (*vm).mEState.is_null() {
        return ptr::null_mut();
    }
    let e = (*vm).mEState;
    if (*e).mFrameIndex <= 0 {
        return ptr::null_mut();
    }
    let fi = (*e).mFrameIndex as usize - 1;
    let start = (*e).mExecStack[fi].mLValSize as usize;
    let end = (*e).mStackFrame.mLValSize as usize;
    if end <= start {
        return ptr::null_mut();
    }
    let mut idx = end;
    while idx > start {
        idx -= 1;
        let slot = (*e).mLocals.as_mut_ptr().add(idx);
        if (*slot).mNameKey == key {
            return slot;
        }
    }
    ptr::null_mut()
}

/// Get pointer to BoyiaValue for REG0, REG1, VAR, or LOCAL operand. Returns null for constant operands.
#[inline]
unsafe fn get_op_value(inst: *const Instruction, side: OpSide, vm: *mut BoyiaVM) -> *mut BoyiaValue {
    if inst.is_null() || vm.is_null() || (*vm).mCpu.is_null() {
        return ptr::null_mut();
    }
    let op = match side {
        OpSide::OpLeft => &(*inst).mOPLeft,
        OpSide::OpRight => &(*inst).mOPRight,
    };
    match op.mType {
        OpType::OP_REG0 => &mut (*(*vm).mCpu).mReg0 as *mut BoyiaValue,
        OpType::OP_REG1 => &mut (*(*vm).mCpu).mReg1 as *mut BoyiaValue,
        // OpType::OP_VAR => get_val(op.mValue as LUintPtr, vm),
        // OpType::OP_LOCAL => get_local_ptr_by_frame_offset(vm, op.mValue),
        OpType::OP_CAPTURE => get_capture_ptr(vm, op.int_value()),
        _ => ptr::null_mut(),
    }
}

/// Next instruction by mNext offset; null if kInvalidInstruction. Used by core::consume_micro_task.
#[inline]
pub(crate) unsafe fn next_instruction(inst: *const Instruction, vm: *mut BoyiaVM) -> *mut Instruction {
    if inst.is_null() || vm.is_null() || (*vm).mVMCode.is_null() || (*(*vm).mVMCode).mCode.is_null() {
        return ptr::null_mut();
    }
    if (*inst).mNext == kInvalidInstruction {
        return ptr::null_mut();
    }
    let code = (*(*vm).mVMCode).mCode;
    code.offset((*inst).mNext as isize)
}

/// Reset scene of global execute state. Strict 1:1 match of ResetScene in BoyiaCore.cpp.
pub(crate) unsafe fn reset_scene(state: *mut ExecState) {
    if state.is_null() {
        return;
    }
    (*state).mStackFrame.mLValSize = 0;
    (*state).mStackFrame.mLoopSize = 0;
    (*state).mStackFrame.mResultNum = 0;
    (*state).mStackFrame.mContext = ptr::null_mut();
    (*state).mStackFrame.mPC = ptr::null_mut();
    (*state).mLast = ptr::null_mut();
    (*state).mTopTask = ptr::null_mut();

    (*state).mFrameIndex = 0;

    (*state).mFun.mValue.mObj.mPtr = K_BOYIA_NULL;
    (*state).mFun.mValue.mObj.mSuper = K_BOYIA_NULL;
    (*state).mFun.mValueType = ValueType::BY_ARG;
    (*state).mWait = LFalse;

    assign_state_class(state, ptr::null());
}

/// ExecPopFunction: strictly matches BoyiaCore.cpp. if mPC then return; if mFrameIndex<=0 && mLast && !mLast->mWait then SwitchExecState(mLast), maybe DestroyExecState; if mFrameIndex>0 then HandlePopScene(kBoyiaNull, vm), then if mPC then mPC=NextInstruction, ExecPopFunction else if mLast&&!mLast->mWait then ExecPopFunction.
unsafe fn exec_pop_function(vm: *mut BoyiaVM) {
    if vm.is_null() || (*vm).mEState.is_null() {
        return;
    }
    let e_state = (*vm).mEState;
    if !(*e_state).mStackFrame.mPC.is_null() {
        return;
    }

    // 指令为空，则判断是否处于函数范围中，是则pop，从而取得调用之前的运行环境
    if (*e_state).mFrameIndex <= 0 {
        let last = (*e_state).mLast;
        if !last.is_null() && (*last).mWait == LFalse {
            let current_state = e_state;
            switch_exec_state(last, vm);
            if (*current_state).mWait == LFalse {
                if !(*current_state).mTopTask.is_null() {
                    free_micro_task((*current_state).mTopTask, vm);
                }
                destroy_exec_state(current_state, vm);
            }
        }
    }

    // SwitchExecState后mFrameIndex可能不为0
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mFrameIndex <= 0 {
        return;
    }
    let _ = handle_pop_scene(ptr::null(), vm);
    let e_state = (*vm).mEState;
    if e_state.is_null() {
        return;
    }
    if !(*e_state).mStackFrame.mPC.is_null() {
        let pc = (*e_state).mStackFrame.mPC;
        (*e_state).mStackFrame.mPC = next_instruction(pc, vm);
        exec_pop_function(vm);
    } else if !(*e_state).mLast.is_null() && (*(*e_state).mLast).mWait == LFalse {
        exec_pop_function(vm);
    }
}

/// Single handler dispatch: returns OpHandleResult.
unsafe fn dispatch_instruction(inst: *mut Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    if inst.is_null() || vm.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let opcode = (*inst).mOPCode;
    eprintln!("[dispatch_instruction] mOPCode={:?}", opcode);
    match opcode {
        CmdType::kCmdDeclGlobal => handle_decl_global(inst, vm),
        CmdType::kCmdDeclLocal => handle_decl_local(inst, vm),
        CmdType::kCmdPopLocals => handle_pop_locals(inst, vm),
        CmdType::kCmdPush => handle_push(inst, vm),
        CmdType::kCmdPop => handle_pop(inst, vm),
        CmdType::kCmdAdd => handle_add(inst, vm),
        CmdType::kCmdSub => handle_sub(inst, vm),
        CmdType::kCmdMul => handle_mul(inst, vm),
        CmdType::kCmdDiv => handle_div(inst, vm),
        CmdType::kCmdMod => handle_mod(inst, vm),
        CmdType::kCmdPushScene => handle_push_scene(inst, vm),
        CmdType::kCmdPopScene => handle_pop_scene(inst, vm),
        CmdType::kCmdPushArg => handle_push_arg(inst, vm),
        CmdType::kCmdPushObj => handle_push_obj(inst, vm),
        CmdType::kCmdAssign => handle_assignment(inst, vm),
        CmdType::kCmdJmpTrue => handle_jump_to_if_true(inst, vm),
        CmdType::kCmdOnceJmpTrue => handle_once_jmp_true(inst as *mut Instruction, vm),
        CmdType::kCmdIfEnd => handle_if_end(inst, vm),
        CmdType::kCmdJmpTo => handle_jump_to(inst, vm),
        CmdType::kCmdLoop => handle_loop_begin(inst, vm),
        CmdType::kCmdLoopTrue => handle_loop_if_true(inst, vm),
        CmdType::kCmdReturn => handle_return(inst, vm),
        CmdType::kCmdBreak => handle_break(inst, vm),
        CmdType::kCmdCallNative => handle_call_native(inst, vm),
        // C++: handlers[kCmdElse] = kBoyiaNull, handlers[kCmdElEnd] = kBoyiaNull — no handler, just advance PC.
        CmdType::kCmdElif | CmdType::kCmdElse | CmdType::kCmdElEnd => OpHandleResult::kOpResultSuccess,
        CmdType::kCmdEnd => handle_cmd_end(inst, vm),
        CmdType::kCmdAndLogic | CmdType::kCmdOrLogic => handle_logic(inst, vm),
        CmdType::kCmdGtRelation
        | CmdType::kCmdLtRelation
        | CmdType::kCmdGeRelation
        | CmdType::kCmdLeRelation
        | CmdType::kCmdNotRelation
        | CmdType::kCmdEqRelation
        | CmdType::kCmdNeRelation => handle_relational(inst, vm),
        CmdType::kCmdPushParams => handle_push_params(inst, vm),
        CmdType::kCmdCallFunction => handle_call_function(inst, vm),
        CmdType::kCmdCreateClass => handle_create_class(inst, vm),
        CmdType::kCmdClassExtend => handle_extend(inst, vm),
        CmdType::kCmdExecCreate => handle_create_executor(inst, vm),
        CmdType::kCmdCreateFunction => handle_fun_create(inst, vm),
        CmdType::kCmdParamCreate => handle_create_param(inst, vm),
        CmdType::kCmdPropCreate => handle_create_prop(inst, vm),
        CmdType::kCmdFunCreate => handle_fun_create(inst, vm), // alias for kCmdCreateFunction in some paths
        CmdType::kCmdAssignVar => handle_assign_var(inst, vm),
        CmdType::kCmdGetProp => handle_get_prop(inst, vm),
        CmdType::kCmdConstStr => handle_const_str(inst, vm),
        CmdType::kCmdCreateMap => handle_create_map(inst, vm),
        CmdType::kCmdSetMapKey => handle_set_map_key(inst, vm),
        CmdType::kCmdSetMapValue => handle_set_map_value(inst, vm),
        CmdType::kCmdCreateArray => handle_create_array(inst, vm),
        CmdType::kCmdAddArrayItem => handle_add_array_item(inst, vm),
        CmdType::kCmdAwait => handle_await(inst, vm),
        CmdType::kCmdSetAnonym => handle_set_anonym(inst, vm),
        CmdType::kCmdDeclConstStr => OpHandleResult::kOpResultSuccess,
    }
}

// ---------- Handlers (match BoyiaCore.cpp) ----------

unsafe fn handle_decl_global(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    if vm.is_null() || (*vm).mGlobals.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let type_val = (*inst).mOPLeft.int_value() as u8;
    let name_key = (*inst).mOPRight.int_value() as LUintPtr;
    let val = (*vm).mGlobals.add((*vm).mGValSize as usize);
    (*vm).mGValSize += 1;
    (*val).mValueType = std::mem::transmute(type_val);
    (*val).mNameKey = name_key;
    (*val).mValue.mIntVal = 0;
    OpHandleResult::kOpResultSuccess
}

/// HandleDeclLocal: strictly matches BoyiaCore.cpp. BoyiaValue local; local.mValueType = inst->mOPLeft.mValue; local.mNameKey = (LUintPtr)inst->mOPRight.mValue; LocalPush(&local, vm).
unsafe fn handle_decl_local(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let mut local = BoyiaValue {
        mNameKey: 0,
        mValueType: std::mem::transmute((*inst).mOPLeft.int_value() as u8),
        mValue: RealValue { mIntVal: 0 },
    };
    local.mNameKey = (*inst).mOPRight.int_value() as LUintPtr;
    local_push(&mut local, vm as *mut LVoid);
    OpHandleResult::kOpResultSuccess
}

/// Pop N persistent local slots (block exit); matches BoyiaCore.cpp HandlePopLocals / kCmdPopLocals.
unsafe fn handle_pop_locals(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    if vm.is_null() || (*vm).mEState.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let e = (*vm).mEState;
    if (*e).mFrameIndex <= 0 {
        return OpHandleResult::kOpResultEnd;
    }
    let n = (*inst).mOPLeft.int_value();
    if n <= 0 {
        return OpHandleResult::kOpResultSuccess;
    }
    let fi = (*e).mFrameIndex as usize - 1;
    let start = (*e).mExecStack[fi].mLValSize;
    let new_size = (*e).mStackFrame.mLValSize - n as LInt;
    if new_size < start + 1 {
        return OpHandleResult::kOpResultEnd;
    }
    (*e).mStackFrame.mLValSize = new_size;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_push(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let value = if (*inst).mOPLeft.mType == OpType::OP_REG0 {
        &(*(*vm).mCpu).mReg0 as *const BoyiaValue as *mut BoyiaValue
    } else {
        &(*(*vm).mCpu).mReg1 as *const BoyiaValue as *mut BoyiaValue
    };
    let e_state = (*vm).mEState;
    if e_state.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let result_num = (*e_state).mStackFrame.mResultNum as usize;
    if result_num >= NUM_RESULT {
        return OpHandleResult::kOpResultEnd;
    }
    value_copy((*e_state).mOpStack.as_mut_ptr().add(result_num), value);
    (*e_state).mStackFrame.mResultNum += 1;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_pop(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mStackFrame.mResultNum <= 0 {
        return OpHandleResult::kOpResultEnd;
    }
    if (*inst).mOPLeft.mType != OpType::OP_REG0 && (*inst).mOPLeft.mType != OpType::OP_REG1 {
        (*e_state).mStackFrame.mResultNum -= 1;
        return OpHandleResult::kOpResultSuccess;
    }
    (*e_state).mStackFrame.mResultNum -= 1;
    let result_num = (*e_state).mStackFrame.mResultNum as usize;
    let src = (*e_state).mOpStack.as_ptr().add(result_num);
    let dest = if (*inst).mOPLeft.mType == OpType::OP_REG0 {
        &mut (*(*vm).mCpu).mReg0
    } else {
        &mut (*(*vm).mCpu).mReg1
    };
    value_copy(dest, src);
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_add(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*left).mValueType == ValueType::BY_INT && (*right).mValueType == ValueType::BY_INT {
        (*right).mValue.mIntVal += (*left).mValue.mIntVal;
        return OpHandleResult::kOpResultSuccess;
    }
    if (*left).mValueType != ValueType::BY_CLASS && (*right).mValueType != ValueType::BY_CLASS {
        if !is_boyia_number(left) || !is_boyia_number(right) {
            return OpHandleResult::kOpResultEnd;
        }
        let lv = get_boyia_number(left);
        let rv = get_boyia_number(right);
        (*right).mValue.mRealVal = lv + rv;
        (*right).mValueType = ValueType::BY_REAL;
        return OpHandleResult::kOpResultSuccess;
    }
    let string_key = BuiltinId::kBoyiaString.as_key();
    if get_boyia_class_id(left) == string_key || get_boyia_class_id(right) == string_key {
        string_add(left, right, vm);
        return OpHandleResult::kOpResultSuccess;
    }
    OpHandleResult::kOpResultEnd
}

unsafe fn handle_sub(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if !is_boyia_number(left) || !is_boyia_number(right) {
        return OpHandleResult::kOpResultEnd;
    }
    if (*left).mValueType == ValueType::BY_INT && (*right).mValueType == ValueType::BY_INT {
        (*right).mValue.mIntVal = (*left).mValue.mIntVal - (*right).mValue.mIntVal;
        return OpHandleResult::kOpResultSuccess;
    }
    let lv = get_boyia_number(left);
    let rv = get_boyia_number(right);
    (*right).mValue.mRealVal = lv - rv;
    (*right).mValueType = ValueType::BY_REAL;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_mul(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if !is_boyia_number(left) || !is_boyia_number(right) {
        return OpHandleResult::kOpResultEnd;
    }
    if (*left).mValueType == ValueType::BY_INT && (*right).mValueType == ValueType::BY_INT {
        (*right).mValue.mIntVal *= (*left).mValue.mIntVal;
        return OpHandleResult::kOpResultSuccess;
    }
    let lv = get_boyia_number(left);
    let rv = get_boyia_number(right);
    (*right).mValue.mRealVal = lv * rv;
    (*right).mValueType = ValueType::BY_REAL;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_div(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if !is_boyia_number(left) || !is_boyia_number(right) {
        return OpHandleResult::kOpResultEnd;
    }
    if ((*right).mValueType == ValueType::BY_INT && (*right).mValue.mIntVal == 0)
        || ((*right).mValueType == ValueType::BY_REAL && (*right).mValue.mRealVal == 0.0)
    {
        return OpHandleResult::kOpResultEnd;
    }
    if (*left).mValueType == ValueType::BY_INT && (*right).mValueType == ValueType::BY_INT {
        (*right).mValue.mIntVal = (*left).mValue.mIntVal / (*right).mValue.mIntVal;
        return OpHandleResult::kOpResultSuccess;
    }
    let lv = get_boyia_number(left);
    let rv = get_boyia_number(right);
    (*right).mValue.mRealVal = lv / rv;
    (*right).mValueType = ValueType::BY_REAL;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_mod(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*left).mValueType != ValueType::BY_INT || (*right).mValueType != ValueType::BY_INT {
        return OpHandleResult::kOpResultEnd;
    }
    if (*right).mValue.mIntVal == 0 {
        return OpHandleResult::kOpResultEnd;
    }
    (*right).mValue.mIntVal = (*left).mValue.mIntVal % (*right).mValue.mIntVal;
    OpHandleResult::kOpResultSuccess
}

/// HandlePushScene per BoyiaCore.cpp: set mLValSize, mPC, mContext, mResultNum, mLoopSize on mExecStack[mFrameIndex]; mClass is set by HandlePushObj only.
/// When inst is null (HandlePushScene(kBoyiaNull, vm)), mLValSize=0 and mPC=null.
pub(crate) unsafe fn handle_push_scene(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let e_state = (*vm).mEState;
    if e_state.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let frame_index = (*e_state).mFrameIndex as usize;
    if frame_index >= FUNC_CALLS {
        return OpHandleResult::kOpResultEnd;
    }
    let exec_stack = (*vm).mExecStack;
    if exec_stack.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let frame = exec_stack.add(frame_index);
    (*frame).mLValSize = if inst.is_null() {
        0
    } else {
        (*e_state).mStackFrame.mLValSize - (*(*vm).mCpu).mReg1.mValue.mIntVal as LInt
    };
    (*frame).mPC = if inst.is_null() {
        ptr::null_mut()
    } else {
        inst.offset((*inst).mOPLeft.int_value() as isize) as *mut Instruction
    };
    (*frame).mContext = (*e_state).mStackFrame.mContext;
    (*frame).mResultNum = (*e_state).mStackFrame.mResultNum;
    (*frame).mLoopSize = (*e_state).mStackFrame.mLoopSize;
    (*e_state).mFrameIndex += 1;
    OpHandleResult::kOpResultSuccess
}

/// HandlePopScene per BoyiaCore.cpp: if mFrameIndex > 0, when `inst` is null (ExecPopFunction path) and
/// callee slot 0 is `BY_ANONYM_FUNC`, clear ptr and `delete_data` the PushArg-cloned `BoyiaFunction` + `mParams`;
/// then --mFrameIndex and restore mLValSize, mPC, mContext, mLoopSize, AssignStateClass.
unsafe fn handle_pop_scene(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mFrameIndex <= 0 {
        return OpHandleResult::kOpResultEnd;
    }
    if inst.is_null() {
        let callee = get_local_value(0, vm as *mut LVoid) as *mut BoyiaValue;
        if !callee.is_null()
            && (*callee).mValueType == ValueType::BY_ANONYM_FUNC
            && (*callee).mValue.mObj.mPtr != K_BOYIA_NULL
        {
            let fun = (*callee).mValue.mObj.mPtr as *mut BoyiaFunction;
            let params = (*fun).mParams;
            (*callee).mValue.mObj.mPtr = K_BOYIA_NULL;
            let creator = (*vm).mCreator;
            if !creator.is_null() {
                (*creator).delete_data(fun as *mut LVoid);
                if !params.is_null() {
                    (*creator).delete_data(params as *mut LVoid);
                }
            }
        }
    }
    (*e_state).mFrameIndex -= 1;
    let idx = (*e_state).mFrameIndex as usize;
    (*e_state).mStackFrame.mLValSize = (*e_state).mExecStack[idx].mLValSize;
    (*e_state).mStackFrame.mPC = (*e_state).mExecStack[idx].mPC;
    (*e_state).mStackFrame.mContext = (*e_state).mExecStack[idx].mContext;
    (*e_state).mStackFrame.mLoopSize = (*e_state).mExecStack[idx].mLoopSize;
    assign_state_class(e_state, &(*e_state).mExecStack[idx].mClass as *const BoyiaValue);
    OpHandleResult::kOpResultSuccess
}

/// Append copies of current-frame locals `mLocals[start+1 .. mLValSize)` into `fun`'s capture tail
/// (`mParams[mParamSize + mCaptureCount]`), incrementing `mCaptureCount` only. Slot `start` is the callee pointer.
unsafe fn capture_current_frame_locals_into_function(
    vm: *mut BoyiaVM,
    fun: *mut BoyiaFunction,
) -> OpHandleResult {
    if vm.is_null() || (*vm).mEState.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if fun.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    if (*fun).mParams.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let e_state = (*vm).mEState;
    if (*e_state).mFrameIndex <= 0 || (*vm).mExecStack.is_null() || (*vm).mLocals.is_null() {
        return OpHandleResult::kOpResultSuccess;
    }
    let frame_idx = (*e_state).mFrameIndex as usize - 1;
    let start = (*vm).mExecStack.add(frame_idx).read().mLValSize;
    if start < 0 || start as usize >= NUM_LOCAL_VARS {
        return OpHandleResult::kOpResultSuccess;
    }
    let frame_end = (*e_state).mStackFrame.mLValSize;
    if frame_end <= start + 1 {
        return OpHandleResult::kOpResultSuccess;
    }
    let mut idx = start + 1;
    while idx < frame_end {
        while (*fun).mParamSize + (*fun).mCaptureCount >= crate::core::get_function_count(fun) {
            if !vector_params_grow_if_full(fun, vm as *mut LVoid) {
                return OpHandleResult::kOpResultEnd;
            }
        }
        let write_idx = (*fun).mParamSize + (*fun).mCaptureCount;
        value_copy_no_name(
            (*fun).mParams.add(write_idx as usize),
            (*vm).mLocals.add(idx as usize),
        );
        (*fun).mCaptureCount += 1;
        idx += 1;
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_push_arg(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let value = get_op_value(inst, OpSide::OpLeft, vm);
    if value.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*value).mValueType == ValueType::BY_ANONYM_FUNC {
        let src_fun = (*value).mValue.mObj.mPtr as *mut BoyiaFunction;
        if src_fun.is_null() {
            return OpHandleResult::kOpResultEnd;
        }
        /* mCaptureCount == 0: template from mFunTable; clone then capture (BoyiaCore.cpp HandlePushArg). */
        if (*src_fun).mCaptureCount == 0 {
            println!("handle_push_arg mCaptureCount==0");
            let fun = clone_anonym_boyia_function_for_push_arg(src_fun, vm);
            if fun.is_null() {
                return OpHandleResult::kOpResultEnd;
            }
            
            let r = capture_current_frame_locals_into_function(vm, fun);
            if r != OpHandleResult::kOpResultSuccess {
                return r;
            }
            (*value).mValue.mObj.mPtr = fun as LIntPtr;
            let rt = get_runtime_from_vm(vm as *mut LVoid);
            if !rt.is_null() {
                let _ = (*rt).persistent_object(value as *const BoyiaValue);
            }
        }
    }
    eprintln!("[handle_push_arg] value.mNameKey={}", (*value).mNameKey);
    local_push(value, vm as *mut LVoid);
    OpHandleResult::kOpResultSuccess
}

pub(crate) unsafe fn assign_state_class(state: *mut ExecState, value: *const BoyiaValue) {
    if state.is_null() {
        return;
    }
    if value.is_null() {
        (*state).mStackFrame.mClass.mValue.mObj.mPtr = K_BOYIA_NULL;
        (*state).mStackFrame.mClass.mValue.mObj.mSuper = K_BOYIA_NULL;
    } else {
        value_copy_no_name(&mut (*state).mStackFrame.mClass, value);
    }
    (*state).mStackFrame.mClass.mValueType = ValueType::BY_CLASS;
}

/// HandlePushObj per BoyiaCore.cpp: mExecStack[mFrameIndex].mClass = mStackFrame.mClass; then if OP_VAR and not kBoyiaSuper, AssignStateClass(Reg0) else AssignStateClass(null).
unsafe fn handle_push_obj(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*vm).mExecStack.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let frame_index = (*e_state).mFrameIndex as usize;
    (*((*vm).mExecStack.add(frame_index))).mClass = (*e_state).mStackFrame.mClass;
    if (*inst).mOPLeft.mType == OpType::OP_VAR {
        let obj_key = (*inst).mOPLeft.int_value() as LUintPtr;
        if obj_key != BuiltinId::kBoyiaSuper.as_key() {
            assign_state_class(e_state, &(*(*vm).mCpu).mReg0 as *const BoyiaValue);
        }
    } else {
        assign_state_class(e_state, ptr::null());
    }
    OpHandleResult::kOpResultSuccess
}

/// HandleAssignment(Instruction* inst, BoyiaVM* vm) per BoyiaCore.cpp.
/// Assign right operand to left; no copy to Reg0 at end (C++ does not).
unsafe fn handle_assignment(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    if left.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    match (*inst).mOPRight.mType {
        OpType::OP_CONST_STRING => {
            let val = (*inst).mOPRight.int_value() as *const BoyiaValue;
            if !val.is_null() {
                value_copy_no_name(left, val);
            }
        }
        OpType::OP_CONST_NUMBER => {
            (*left).mValueType = ValueType::BY_INT;
            (*left).mValue.mIntVal = (*inst).mOPRight.int_value();
        }
        OpType::OP_CONST_REAL => {
            (*left).mValueType = ValueType::BY_REAL;
            (*left).mValue.mRealVal = (*inst).mOPRight.real_value();
        }
        OpType::OP_VAR => {
            let val = get_val((*inst).mOPRight.int_value() as LUintPtr, vm);
            if val.is_null() {
                return OpHandleResult::kOpResultEnd;
            }
            value_copy_with_key(left, val);
        }
        OpType::OP_LOCAL => {
            let val = get_local_ptr_by_frame_offset(vm, (*inst).mOPRight.int_value());
            if val.is_null() {
                return OpHandleResult::kOpResultEnd;
            }
            value_copy_with_key(left, val);
        }
        OpType::OP_CAPTURE => {
            println!("get op capture");
            let val = get_capture_ptr(vm, (*inst).mOPRight.int_value());
            if val.is_null() {
                println!("get op capture1");
                return OpHandleResult::kOpResultEnd;
            }
            value_copy_with_key(left, val);
        }
        OpType::OP_REG0 => {
            value_copy_no_name(left, &(*(*vm).mCpu).mReg0);
        }
        OpType::OP_REG1 => {
            value_copy_no_name(left, &(*(*vm).mCpu).mReg1);
        }
        _ => {}
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_jump_to_if_true(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let value = get_op_value(inst, OpSide::OpLeft, vm);
    if value.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*value).mValue.mIntVal == 0 {
        let offset = (*inst).mOPRight.int_value() as isize;
        (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_once_jmp_true(inst: *mut Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    if (*inst).mOPLeft.mType != OpType::OP_CONST_NUMBER {
        return OpHandleResult::kOpResultEnd;
    }
    if (*inst).mOPLeft.int_value() == 0 {
        let offset = (*inst).mOPRight.int_value() as isize;
        (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
    } else {
        (*inst).mOPLeft.set_int_value(LFalse as LIntPtr);
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_if_end(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let offset = (*inst).mOPLeft.int_value() as isize;
    (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
    OpHandleResult::kOpResultSuccess
}

/// HandleJumpTo per BoyiaCore.cpp: if (inst->mOPLeft.mType == OP_CONST_NUMBER)
/// mPC = inst - inst->mOPLeft.mValue; return kOpResultSuccess.
unsafe fn handle_jump_to(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    if (*inst).mOPLeft.mType == OpType::OP_CONST_NUMBER {
        let offset = (*inst).mOPLeft.int_value() as isize;
        (*(*vm).mEState).mStackFrame.mPC = inst.offset(-offset) as *mut Instruction;
    }
    OpHandleResult::kOpResultSuccess
}

/// HandleLoopBegin per BoyiaCore.cpp: push left => loop stack;
/// mLoopStack[mLoopSize++] = (LIntPtr)(inst + inst->mOPLeft.mValue).
unsafe fn handle_loop_begin(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let e_state = (*vm).mEState;
    let loop_size = (*e_state).mStackFrame.mLoopSize as usize;
    let target = inst.offset((*inst).mOPLeft.int_value() as isize);
    (*vm).mLoopStack.add(loop_size).write(target as LIntPtr);
    (*e_state).mStackFrame.mLoopSize += 1;
    OpHandleResult::kOpResultSuccess
}

/// HandleLoopIfTrue per BoyiaCore.cpp: value = &mCpu->mReg0; if !value->mValue.mIntVal then
/// PC = inst + mOPRight.mValue, mLoopSize--, return; if (inst->mOPLeft.mValue) PC = inst + mOPLeft.mValue; return.
unsafe fn handle_loop_if_true(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let value = &(*(*vm).mCpu).mReg0;
    if value.mValue.mIntVal == 0 {
        (*(*vm).mEState).mStackFrame.mPC =
            inst.offset((*inst).mOPRight.int_value() as isize) as *mut Instruction;
        (*(*vm).mEState).mStackFrame.mLoopSize -= 1;
        return OpHandleResult::kOpResultSuccess;
    }
    if (*inst).mOPLeft.int_value() != 0 {
        (*(*vm).mEState).mStackFrame.mPC =
            inst.offset((*inst).mOPLeft.int_value() as isize) as *mut Instruction;
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_return(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let _ = inst;
    let e_state = (*vm).mEState;
    if e_state.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let ctx = (*e_state).mStackFrame.mContext;
    (*e_state).mStackFrame.mPC = if ctx.is_null() {
        ptr::null_mut()
    } else {
        (*ctx).mEnd
    };
    OpHandleResult::kOpResultSuccess
}

/// HandleBreak per BoyiaCore.cpp: mPC = (Instruction*)mLoopStack[--mStackFrame.mLoopSize]; return kOpResultSuccess.
unsafe fn handle_break(_inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let e_state = (*vm).mEState;
    (*e_state).mStackFrame.mLoopSize -= 1;
    let idx = (*e_state).mStackFrame.mLoopSize as usize;
    let target = (*vm).mLoopStack.add(idx).read() as *mut Instruction;
    (*e_state).mStackFrame.mPC = target;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_call_native(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let idx = (*inst).mOPLeft.int_value() as LInt;
    let r = crate::core::native_call_by_index(vm as *mut LVoid, idx);
    op_handle_result_from_i32(r)
}

/// Find object property by key. Strictly matches FindObjProp in BoyiaCore.cpp:
/// 1) find in instance params (obj.prop) -> AddPropInlineCache(cache, klass, idx); return fun->mParams+idx
/// 2) find in class chain (obj.method) -> AddFunInlineCache(cache, klass, result); return result
/// 3) not found -> return null.
/// When inst is non-null, cache is created/filled on hit.
unsafe fn find_obj_prop(
    lval: *const BoyiaValue,
    rval: LUintPtr,
    inst: *mut crate::types::Instruction,
    _vm: *mut BoyiaVM,
) -> *mut BoyiaValue {
    if lval.is_null() || (*lval).mValueType != ValueType::BY_CLASS {
        return ptr::null_mut();
    }

    let fun = (*lval).mValue.mObj.mPtr as *const BoyiaFunction;
    let klass = (*fun).mFuncBody as *mut BoyiaValue;

    // find props, such as obj.prop1.
    let mut idx: LInt = 0;
    while idx < (*fun).mParamSize {
        if (*fun).mParams.add(idx as usize).read().mNameKey == rval {
            if !inst.is_null() {
                let cache = if (*inst).mCache.is_null() {
                    let c = create_inline_cache();
                    (*inst).mCache = c;
                    c
                } else {
                    (*inst).mCache
                };
                add_prop_inline_cache(cache, klass, idx);
            }
            return (*fun).mParams.add(idx as usize);
        }
        idx += 1;
    }

    // find function, such as obj.func1
    let mut cls = klass as *const BoyiaValue;
    while !cls.is_null() && (*cls).mValueType == ValueType::BY_CLASS {
        let cls_map = (*cls).mValue.mObj.mPtr as *const BoyiaFunction;
        let param_size = if cls_map.is_null() {
            0
        } else {
            (*cls_map).mParamSize
        };
        let mut fun_idx: LInt = 0;
        while fun_idx < param_size {
            if (*cls_map).mParams.add(fun_idx as usize).read().mNameKey == rval {
                let result = (*cls_map).mParams.add(fun_idx as usize) as *mut BoyiaValue;
                if !inst.is_null() {
                    let cache = if (*inst).mCache.is_null() {
                        let c = create_inline_cache();
                        (*inst).mCache = c;
                        c
                    } else {
                        (*inst).mCache
                    };
                    println!("[find_obj_prop] add inline cache");
                    add_fun_inline_cache(cache, klass, result);
                }
                return result;
            }
            fun_idx += 1;
        }
        cls = (*cls).mValue.mObj.mSuper as *const BoyiaValue;
    }

    ptr::null_mut()
}

unsafe fn handle_relational(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let result = match (*inst).mOPCode {
        CmdType::kCmdNotRelation => (*right).mValue.mIntVal != 0,
        CmdType::kCmdLtRelation => get_boyia_number(left) < get_boyia_number(right),
        CmdType::kCmdLeRelation => get_boyia_number(left) <= get_boyia_number(right),
        CmdType::kCmdGtRelation => get_boyia_number(left) > get_boyia_number(right),
        CmdType::kCmdGeRelation => get_boyia_number(left) >= get_boyia_number(right),
        CmdType::kCmdEqRelation => compare_value(left, right),
        CmdType::kCmdNeRelation => !compare_value(left, right),
        _ => return OpHandleResult::kOpResultEnd,
    };
    let out = if result {
        OpHandleResult::kOpResultSuccess as i32
    } else {
        OpHandleResult::kOpResultEnd as i32
    };
    (*right).mValueType = ValueType::BY_INT;
    (*right).mValue.mIntVal = out as LIntPtr;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_logic(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let result = match (*inst).mOPCode {
        CmdType::kCmdAndLogic => (*left).mValue.mIntVal != 0 && (*right).mValue.mIntVal != 0,
        CmdType::kCmdOrLogic => (*left).mValue.mIntVal != 0 || (*right).mValue.mIntVal != 0,
        _ => return OpHandleResult::kOpResultEnd,
    };
    let out = if result {
        OpHandleResult::kOpResultSuccess as i32
    } else {
        OpHandleResult::kOpResultEnd as i32
    };
    (*right).mValueType = ValueType::BY_INT;
    (*right).mValue.mIntVal = out as LIntPtr;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_push_params(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let _ = inst;
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mFrameIndex <= 0 {
        return OpHandleResult::kOpResultSuccess;
    }
    let start = (*e_state).mExecStack.as_ptr().add((*e_state).mFrameIndex as usize - 1).read().mLValSize;
    let value = (*e_state).mLocals.as_ptr().add(start as usize) as *mut BoyiaValue;
    let vt = (*value).mValueType;
    // Match C++ HandlePushParams: only assign param name (mNameKey) to existing argument slots; do not overwrite value.
    if vt == ValueType::BY_FUNC
        || vt == ValueType::BY_PROP_FUNC
        || vt == ValueType::BY_ASYNC_PROP
        || vt == ValueType::BY_ANONYM_FUNC
    {
        let func = (*value).mValue.mObj.mPtr as *const BoyiaFunction;
        if (*func).mParamSize <= 0 {
            return OpHandleResult::kOpResultSuccess;
        }
        let mut idx = start + 1;
        let end = idx + (*func).mParamSize;
        while idx < end {
            let v_key = (*func).mParams.add((idx - start - 1) as usize).read().mNameKey;
            let slot = (*e_state).mLocals.as_mut_ptr().add(idx as usize);
            (*slot).mNameKey = v_key;
            idx += 1;
        }
    }
    OpHandleResult::kOpResultSuccess
}

/// HandleCallAsyncFunction per BoyiaCore.cpp: create exec state, switch, push scene, copy params, AssignStateClass.
unsafe fn handle_call_async_function(vm: *mut BoyiaVM) {
    println!("call handle_call_async_function0");
    let current = (*vm).mEState;
    if current.is_null() || (*current).mFrameIndex <= 0 {
        return;
    }

    println!("call handle_call_async_function1");
    let start = (*current)
        .mExecStack
        .as_ptr()
        .add((*current).mFrameIndex as usize - 1)
        .read()
        .mLValSize;
    let start_usize = start as usize;
    if start_usize >= crate::types::NUM_LOCAL_VARS {
        return;
    }
    let value = (*current).mLocals.as_ptr().add(start_usize);
    let state = create_exec_state(vm);
    if state.is_null() {
        return;
    }

    println!("call handle_call_async_function2");
    (*state).mLast = current;
    switch_exec_state(state, vm);
    let _ = handle_push_scene(ptr::null(), vm);
    let func = (*value).mValue.mObj.mPtr as *const BoyiaFunction;
    let param_size = (*func).mParamSize as usize;

    let end = start_usize + param_size + 1;
    for idx in start_usize..end {
        value_copy(
            (*state).mLocals.as_mut_ptr().add(idx - start_usize),
            (*current).mLocals.as_ptr().add(idx),
        );
    }
    (*state).mStackFrame.mLValSize = (*func).mParamSize + 1;
    let obj_body = (*value).mValue.mObj.mSuper as *const BoyiaFunction;
    let val = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_CLASS,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: (*value).mValue.mObj.mSuper,
                mSuper: (*obj_body).mFuncBody,
            },
        },
    };

    println!("call handle_call_async_function3");
    assign_state_class((*vm).mEState, &val as *const BoyiaValue);
}

/// HandleCallFunction per BoyiaCore.cpp: localstack first value is function ptr; BY_NAV_FUNC/BY_NAV_PROP => LocalPush + navFun, return; BY_PROP_FUNC/BY_ANONYM_FUNC => AssignStateClass; BY_ASYNC_PROP => HandleCallAsyncFunction; BY_ASYNC => no-op; then set cmds/PC and return kOpResultJumpFun.
unsafe fn handle_call_function(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    println!("call handle_call_function0");
    let _ = inst;
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mFrameIndex <= 0 {
        return OpHandleResult::kOpResultEnd;
    }
    println!("call handle_call_function1");
    let start = (*vm).mExecStack.add((*e_state).mFrameIndex as usize - 1).read().mLValSize;
    if start as usize >= crate::types::NUM_LOCAL_VARS {
        return OpHandleResult::kOpResultEnd;
    }

    println!("call handle_call_function2");
    let value = (*vm).mLocals.add(start as usize);
    let func = (*value).mValue.mObj.mPtr as *mut BoyiaFunction;
    if func.is_null() {
        return OpHandleResult::kOpResultEnd;
    }

    println!("call handle_call_function3");
    value_copy(&mut (*e_state).mFun, value);
    let value_type = (*value).mValueType;
    // 内置类产生的对象，调用其方法
    if value_type == ValueType::BY_NAV_FUNC || value_type == ValueType::BY_NAV_PROP {
        local_push(&mut (*e_state).mStackFrame.mClass, vm as *mut LVoid);
        let nav_fun = std::mem::transmute::<_, crate::types::NativePtr>((*func).mFuncBody);
        (*e_state).mStackFrame.mPC = ptr::null_mut();
        return nav_fun(vm as *mut LVoid);
    }

    println!("call handle_call_function4");
    if value_type == ValueType::BY_PROP_FUNC || value_type == ValueType::BY_ANONYM_FUNC {
        let obj_body = (*value).mValue.mObj.mSuper as *const BoyiaFunction;
        let val = BoyiaValue {
            mNameKey: 0,
            mValueType: ValueType::BY_CLASS,
            mValue: RealValue {
                mObj: BoyiaClass {
                    mPtr: (*value).mValue.mObj.mSuper,
                    mSuper: (*obj_body).mFuncBody,
                },
            },
        };
        assign_state_class(e_state, &val as *const BoyiaValue);
    } else if value_type == ValueType::BY_ASYNC_PROP {
        println!("call handle_call_async_function");
        handle_call_async_function(vm);
    } else if value_type == ValueType::BY_ASYNC {
        // no-op
    }
    let cmds = (*func).mFuncBody as *mut CommandTable;
    if cmds.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*cmds).mBegin.is_null() {
        return OpHandleResult::kOpResultEnd;
    }

    let e_state_new = (*vm).mEState;
    (*e_state_new).mStackFrame.mContext = cmds;
    (*e_state_new).mStackFrame.mPC = (*cmds).mBegin;
    OpHandleResult::kOpResultJumpFun
}

/// HandleGetProp: strictly matches BoyiaCore.cpp. GetOpValue(Left); GetInlineCache(inst->mCache, lVal); FindObjProp; ValueCopyWithKey.
unsafe fn handle_get_prop(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let lval = get_op_value(inst, OpSide::OpLeft, vm);
    if lval.is_null() {
        return OpHandleResult::kOpResultEnd;
    }

    let result = get_inline_cache((*inst).mCache, lval);
    if !result.is_null() {
        value_copy_with_key(&mut (*(*vm).mCpu).mReg0, result);
        return OpHandleResult::kOpResultSuccess;
    }

    let rval = (*inst).mOPRight.int_value() as LUintPtr;
    let result = find_obj_prop(lval, rval, inst as *mut Instruction, vm);
    if result.is_null() {
        eprintln!(
            "[handle_get_prop] find_obj_prop failed: lval type={}, rval(prop key)={}",
            (*lval).mValueType as u8, rval
        );
        return OpHandleResult::kOpResultEnd;
    }
    value_copy_with_key(&mut (*(*vm).mCpu).mReg0, result);
    OpHandleResult::kOpResultSuccess
}

/// HandleAssignVar(Instruction* inst, BoyiaVM* vm) per BoyiaCore.cpp.
/// Resolve destination: if left->mValueType == BY_VAR then FindVal(left->mNameKey), else value = (BoyiaValue*)left->mNameKey.
/// ValueCopyNoName(value, result); ValueCopy(Reg0, value); return kOpResultSuccess.
unsafe fn handle_assign_var(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let result = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || result.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    // let value: *mut BoyiaValue = if (*left).mValueType == ValueType::BY_VAR {
    //     println!("[handle_assign_var] BY_BAR");
    //     get_val((*left).mNameKey, vm)
    // } else {
    //     println!("[handle_assign_var] not BY_BAR");
    //     (*left).mNameKey as *mut BoyiaValue
    // };

    let value: *mut BoyiaValue = (*left).mNameKey as *mut BoyiaValue;

    println!("[handle_assign_var] value.mNameKey={}", (*value).mNameKey);
    if value.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    value_copy_no_name(value, result);
    value_copy(&mut (*(*vm).mCpu).mReg0, value);
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_create_class(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    if (*inst).mOPLeft.mType == OpType::OP_NONE {
        assign_state_class((*vm).mEState, ptr::null());
        return OpHandleResult::kOpResultSuccess;
    }
    let hash_key = (*inst).mOPLeft.int_value() as LUintPtr;

    println!("[handle_create_class] called key = {}", hash_key);
    let class_val = crate::core::create_fun_val(hash_key, ValueType::BY_CLASS, vm);
    if class_val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    assign_state_class((*vm).mEState, class_val);
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_extend(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let class_val = find_global((*inst).mOPLeft.int_value() as LUintPtr, vm);
    let extend_val = find_global((*inst).mOPRight.int_value() as LUintPtr, vm);
    if class_val.is_null() || extend_val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    (*class_val).mValue.mObj.mSuper = extend_val as LIntPtr;
    OpHandleResult::kOpResultSuccess
}

/// HandleFunCreate per BoyiaCore.cpp (strict 1:1).
/// Branch order: (1) in class and funType != BY_ANONYM_FUNC -> add method; (2) in class and BY_ANONYM_FUNC -> hashKey != 0: SetIntResult(hashKey); else InitFunction + if hashKey==0 patch inst and SetIntResult(mFunSize-1); (3) else CreateFunVal(hashKey, BY_FUNC).
unsafe fn handle_fun_create(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let hash_key = (*inst).mOPLeft.int_value() as LUintPtr;
    let fun_type = (*inst).mOPRight.int_value() as u8;
    let e_state = (*vm).mEState;

    if !e_state.is_null() && (*e_state).mStackFrame.mClass.mValue.mObj.mPtr != K_BOYIA_NULL {
        // Inside a class (or function with mClass set).
        if fun_type != ValueType::BY_ANONYM_FUNC as u8 {
            // Class method: add to class params, point to new slot, InitFunction (C++ lines 1716–1727).
            let func = (*e_state).mStackFrame.mClass.mValue.mObj.mPtr as *mut BoyiaFunction;
            let class_key = (*e_state).mStackFrame.mClass.mNameKey;
            let idx = (*func).mParamSize as usize;
            let slot = (*vm).mFunTable.add((*vm).mFunSize as usize);
            let is_prop_func = fun_type == ValueType::BY_PROP_FUNC as u8
                || fun_type == ValueType::BY_ASYNC_PROP as u8
                || fun_type == ValueType::BY_NAV_PROP as u8
                || fun_type == ValueType::BY_ANONYM_FUNC as u8;
            (*func).mParams.add(idx).write(BoyiaValue {
                mNameKey: hash_key,
                mValueType: std::mem::transmute(fun_type),
                mValue: RealValue {
                    mObj: crate::types::BoyiaClass {
                        mPtr: slot as LIntPtr,
                        mSuper: if is_prop_func {
                            func as LIntPtr
                        } else {
                            K_BOYIA_NULL
                        },
                    },
                },
            });
            (*func).mParamSize += 1;
            init_function(slot, vm);
            eprintln!(
                "[class_func] add method key={} to class key={}, class mParamSize={}",
                hash_key, class_key, (*func).mParamSize
            );
            return OpHandleResult::kOpResultSuccess;
        }

        // funType == BY_ANONYM_FUNC (C++ lines 1729–1741).
        if hash_key != 0 {
            let _ = set_int_result(hash_key as LInt, vm as *mut LVoid);
            return OpHandleResult::kOpResultSuccess;
        }
        init_function((*vm).mFunTable.add((*vm).mFunSize as usize), vm);
        if hash_key == 0 {
            let inst_mut = inst as *mut Instruction;
            (*inst_mut).mOPLeft.mType = OpType::OP_CONST_NUMBER;
            (*inst_mut).mOPLeft.set_int_value(((*vm).mFunSize - 1) as LIntPtr);
            let _ = set_int_result((*vm).mFunSize - 1, vm as *mut LVoid);
            return OpHandleResult::kOpResultSuccess;
        }
    } else {
        // Not in class: top-level function (C++ line 1743).
        let _ = create_fun_val(hash_key, ValueType::BY_FUNC, vm);
    }

    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_create_executor(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let layout = Layout::new::<CommandTable>();
    let new_table = alloc(layout) as *mut CommandTable;
    if new_table.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*inst).mOPLeft.int_value() != -1 {
        let code = (*(*vm).mVMCode).mCode;
        (*new_table).mBegin = code.offset((*inst).mOPLeft.int_value() as isize);
        (*new_table).mEnd = code.offset((*inst).mOPRight.int_value() as isize);
    } else {
        (*new_table).mBegin = ptr::null_mut();
        (*new_table).mEnd = ptr::null_mut();
    }
    if (*vm).mFunSize > 0 {
        let fun = (*vm).mFunTable.add((*vm).mFunSize as usize - 1);
        (*fun).mFuncBody = new_table as LIntPtr;
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_create_param(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let hash_key = (*inst).mOPLeft.int_value() as LUintPtr;
    if (*vm).mFunSize <= 0 {
        return OpHandleResult::kOpResultEnd;
    }
    let function = (*vm).mFunTable.add((*vm).mFunSize as usize - 1);
    let idx = (*function).mParamSize as usize;
    let value = (*function).mParams.add(idx);
    (*value).mNameKey = hash_key;
    (*value).mValueType = ValueType::BY_ARG;
    (*value).mValue.mIntVal = 0;
    (*function).mParamSize += 1;
    OpHandleResult::kOpResultSuccess
}

/// Add property slot to current class (mStackFrame.mClass). Match HandleCreateProp in BoyiaCore.cpp.
unsafe fn handle_create_prop(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let func = (*(*vm).mEState).mStackFrame.mClass.mValue.mObj.mPtr as *mut BoyiaFunction;
    let param = (*func).mParams.add((*func).mParamSize as usize);
    (*param).mNameKey = (*inst).mOPLeft.int_value() as LUintPtr;
    (*param).mValue.mIntVal = 0;
    (*func).mParamSize += 1;
    OpHandleResult::kOpResultSuccess
}

/// HandleCreateMap per BoyiaCore.cpp: CreatMapObject(vm); value = mOPLeft ? mReg0 : mReg1; set BY_CLASS, mPtr, mSuper = K_BOYIA_NULL.
unsafe fn handle_create_map(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    // CreatMapObject(vm) = CopyObject(kBoyiaMap, 32, vm) per BoyiaValue.cpp
    let fun = copy_object(BuiltinId::kBoyiaMap.as_key(), 32, vm as *mut LVoid) as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let value = if (*inst).mOPLeft.mType == OpType::OP_REG0 {
        &mut (*(*vm).mCpu).mReg0
    } else {
        &mut (*(*vm).mCpu).mReg1
    };
    (*value).mValueType = ValueType::BY_CLASS;
    (*value).mValue.mObj.mPtr = fun as LIntPtr;
    (*value).mValue.mObj.mSuper = K_BOYIA_NULL;
    OpHandleResult::kOpResultSuccess
}

/// HandleSetMapKey per BoyiaCore.cpp: value = mOPLeft ? mReg0 : mReg1; function = value->mObj.mPtr; param = &function->mParams[function->mParamSize++]; param->mNameKey = mOPRight.mValue.
unsafe fn handle_set_map_key(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let value = if (*inst).mOPLeft.mType == OpType::OP_REG0 {
        &(*(*vm).mCpu).mReg0
    } else {
        &(*(*vm).mCpu).mReg1
    };
    let function = (*value).mValue.mObj.mPtr as *mut BoyiaFunction;
    let param = (*function).mParams.add((*function).mParamSize as usize);
    (*function).mParamSize += 1;
    (*param).mNameKey = (*inst).mOPRight.int_value() as LUintPtr;
    OpHandleResult::kOpResultSuccess
}

/// HandleSetMapValue per BoyiaCore.cpp: obj = mOPRight ? mReg0 : mReg1; value = mOPLeft ? mReg0 : mReg1; param = function->mParams[mParamSize-1]; ValueCopyNoName(param, value); SetNativeResult(obj, vm).
unsafe fn handle_set_map_value(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let obj = if (*inst).mOPRight.mType == OpType::OP_REG0 {
        &(*(*vm).mCpu).mReg0
    } else {
        &(*(*vm).mCpu).mReg1
    };
    let value = if (*inst).mOPLeft.mType == OpType::OP_REG0 {
        &(*(*vm).mCpu).mReg0
    } else {
        &(*(*vm).mCpu).mReg1
    };
    let function = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    let param = (*function).mParams.add((*function).mParamSize as usize - 1);
    value_copy_no_name(param, value);
    set_native_result(obj as *const BoyiaValue as *mut LVoid, vm as *mut LVoid);
    OpHandleResult::kOpResultSuccess
}

/// HandleCreateArray per BoyiaCore.cpp: CreateArrayObject(vm); value = mOPLeft ? mReg0 : mReg1; set BY_CLASS, mPtr, mSuper = K_BOYIA_NULL.
unsafe fn handle_create_array(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    // CreateArrayObject(vm) = CopyObject(kBoyiaArray, 32, vm) per BoyiaValue.cpp
    let fun = copy_object(BuiltinId::kBoyiaArray.as_key(), 32, vm as *mut LVoid) as *mut BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let value = if (*inst).mOPLeft.mType == OpType::OP_REG0 {
        &mut (*(*vm).mCpu).mReg0
    } else {
        &mut (*(*vm).mCpu).mReg1
    };
    (*value).mValueType = ValueType::BY_CLASS;
    (*value).mValue.mObj.mPtr = fun as LIntPtr;
    (*value).mValue.mObj.mSuper = K_BOYIA_NULL;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_add_array_item(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let obj = if (*inst).mOPRight.mType == OpType::OP_REG0 {
        &(*(*vm).mCpu).mReg0
    } else {
        &(*(*vm).mCpu).mReg1
    };
    let value = if (*inst).mOPLeft.mType == OpType::OP_REG0 {
        &(*(*vm).mCpu).mReg0
    } else {
        &(*(*vm).mCpu).mReg1
    };
    let function = (*obj).mValue.mObj.mPtr as *mut BoyiaFunction;
    if function.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let idx = (*function).mParamSize as usize;
    value_copy((*function).mParams.add(idx), value);
    (*function).mParamSize += 1;
    set_native_result(obj as *const BoyiaValue as *mut LVoid, vm as *mut LVoid);
    OpHandleResult::kOpResultSuccess
}

/// HandleConstString(Instruction* inst, BoyiaVM* vm) per BoyiaCore.cpp.
/// str = &mStrTable->mTable[inst->mOPLeft.mValue]; CreateConstString(&mReg0, str->mPtr, str->mLen, vm).
unsafe fn handle_const_str(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    if (*vm).mStrTable.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let str_table = &*(*vm).mStrTable;
    let str_idx = (*inst).mOPLeft.int_value() as usize;
    if str_idx >= str_table.mSize as usize {
        return OpHandleResult::kOpResultEnd;
    }
    let s = &str_table.mTable[str_idx];
    let reg0 = &mut (*(*vm).mCpu).mReg0 as *mut BoyiaValue;
    if create_const_string(reg0, s.mPtr, s.mLen, vm as *mut LVoid) {
        OpHandleResult::kOpResultSuccess
    } else {
        OpHandleResult::kOpResultEnd
    }
}

unsafe fn handle_await(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    if left.is_null() {
        return handle_return(inst, vm);
    }
    let e_state = (*vm).mEState;
    if e_state.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let class_id = get_boyia_class_id(left);
    if class_id != micro_task_class_key(vm as *mut LVoid) {
        return handle_return(inst, vm);
    }
    (*e_state).mWait = LTrue;
    let fun = (*left).mValue.mObj.mPtr as *const BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let task = (*fun).mParams.add(1).read().mValue.mIntVal as *mut MicroTask;
    println!("call handle_await");
    (*task).mAsyncEs = e_state;
    if (*e_state).mTopTask.is_null() {
        let top_task = alloc_micro_task(vm);
        if !top_task.is_null() {
            (*e_state).mTopTask = top_task;
            (*top_task).mAsyncEs = ptr::null_mut();
            let task_obj = create_micro_task_object(vm as *mut LVoid);
            if !task_obj.is_null() {
                (*task_obj).mParams.add(1).write(BoyiaValue {
                    mNameKey: 0,
                    mValueType: ValueType::BY_INT,
                    mValue: RealValue {
                        mIntVal: top_task as LIntPtr,
                    },
                });
                let mut result = BoyiaValue {
                    mNameKey: 0,
                    mValueType: ValueType::BY_CLASS,
                    mValue: RealValue {
                        mObj: BoyiaClass {
                            mPtr: task_obj as LIntPtr,
                            mSuper: K_BOYIA_NULL,
                        },
                    },
                };
                set_native_result(&mut result as *mut BoyiaValue as *mut LVoid, vm as *mut LVoid);
            }
        }
    }
    let last = (*e_state).mLast;
    if !last.is_null() && (*last).mWait == LFalse {
        switch_exec_state(last, vm);
        (*(*vm).mEState).mStackFrame.mPC = ptr::null_mut();
        return OpHandleResult::kOpResultSuccess;
    }
    OpHandleResult::kOpResultEnd
}

unsafe fn handle_set_anonym(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    if left.is_null() || (*vm).mFunTable.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let idx = (*left).mValue.mIntVal as usize;
    let func = (*vm).mFunTable.add(idx);
    let e_state = (*vm).mEState;
    if e_state.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let mut result = BoyiaValue {
        mNameKey: 0,
        mValueType: ValueType::BY_ANONYM_FUNC,
        mValue: RealValue {
            mObj: BoyiaClass {
                mPtr: func as LIntPtr,
                mSuper: (*e_state).mStackFrame.mClass.mValue.mObj.mPtr,
            },
        },
    };
    set_native_result(&mut result as *mut BoyiaValue as *mut LVoid, vm as *mut LVoid);
    OpHandleResult::kOpResultSuccess
}

/// No-op; kCmdEnd is the enum end sentinel in C++, not dispatched.
unsafe fn handle_cmd_end(_inst: *const Instruction, _vm: *mut BoyiaVM) -> OpHandleResult {
    OpHandleResult::kOpResultSuccess
}

// ---------- Main loop ----------

/// Execute instructions until PC is null or handler returns kOpResultEnd. Internal only; use execute_code / execute_global_code.
pub(crate) unsafe fn exec_instruction(vm: *mut BoyiaVM) {
    eprintln!("[exec_instruction] enter");
    if vm.is_null() || (*vm).mEState.is_null() {
        return;
    }
    let mut e_state = (*vm).mEState;
    eprintln!("[exec_instruction] pc={:?}", (*e_state).mStackFrame.mPC);
    if (*e_state).mStackFrame.mPC.is_null() {
        exec_pop_function(vm);
        return;
    }
    eprintln!("[exec_instruction] dispatching first inst");
    while !(*e_state).mStackFrame.mPC.is_null() {
        let pc = (*e_state).mStackFrame.mPC;
        let result = dispatch_instruction(pc, vm);
        e_state = (*vm).mEState;
        if result == OpHandleResult::kOpResultEnd {
            break;
        }

        if !(*e_state).mStackFrame.mPC.is_null() && result == OpHandleResult::kOpResultJumpFun {
            continue;
        }

        if !(*e_state).mStackFrame.mPC.is_null() {
            (*e_state).mStackFrame.mPC = next_instruction((*e_state).mStackFrame.mPC, vm);
        }
        exec_pop_function(vm);
        e_state = (*vm).mEState;
    }
}

/// Execute global code: for each entry, set mContext and run execute_code. Match ExecuteGlobalCode in BoyiaCore.cpp.
pub unsafe fn execute_global_code(vm: *mut LVoid) {
    eprintln!("[execute_global_code] enter");
    if vm.is_null() {
        return;
    }
    let vm = vm as *mut BoyiaVM;
    let vmcode = (*vm).mVMCode;
    let entry_ptr = (*vm).mEntry;
    if (*vm).mEState.is_null() || entry_ptr.is_null() || vmcode.is_null() || (*vmcode).mCode.is_null() {
        eprintln!("[execute_global_code] early return null");
        return;
    }
    reset_scene((*vm).mEState);
    let entry = &*entry_ptr;
    let code_base = (*vmcode).mCode;
    let size = entry.mSize as usize;
    eprintln!("[execute_global_code] entry size={}", size);
    let mut cmds = crate::types::CommandTable {
        mBegin: ptr::null_mut(),
        mEnd: ptr::null_mut(),
    };
    for i in 0..size {
        let entry_offset = *entry.mTable.as_ptr().add(i);
        cmds.mBegin = code_base.offset(entry_offset as isize);
        cmds.mEnd = ptr::null_mut();
        (*(*vm).mEState).mStackFrame.mContext = &mut cmds;
        execute_code(vm as *mut LVoid);
    }
}

/// Execute code for current mStackFrame.mContext (set by caller). Resets scene after.
pub unsafe fn execute_code(vm: *mut LVoid) {
    let vm = vm as *mut BoyiaVM;
    eprintln!("[execute_code] enter");
    if vm.is_null() || (*vm).mEState.is_null() {
        return;
    }
    let e_state = (*vm).mEState;
    let ctx = (*e_state).mStackFrame.mContext;
    eprintln!("[execute_code] ctx={:?}", ctx);
    if ctx.is_null() {
        return;
    }
    let begin = (*ctx).mBegin;
    eprintln!("[execute_code] mBegin={:?}", begin);
    (*e_state).mStackFrame.mPC = begin;
    eprintln!("[execute_code] calling exec_instruction");
    exec_instruction(vm);
    reset_scene((*vm).mEState);
}
