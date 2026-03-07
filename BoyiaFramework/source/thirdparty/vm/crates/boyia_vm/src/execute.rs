//! Instruction execution: main loop and handlers. Port of BoyiaCore.cpp ExecInstruction / ExecuteCode.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

use crate::core::{
    alloc_micro_task, copy_object, create_const_string, create_fun_val, create_micro_task_object,
    destroy_exec_state, find_global, free_micro_task, get_boyia_class_id, init_function, local_push,
    micro_task_class_key, set_int_result, set_native_result, switch_exec_state, value_copy,
    value_copy_no_name,
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
    if (*e).mFrameIndex > 0 {
        let start = (*e).mExecStack.as_ptr().add((*e).mFrameIndex as usize - 1).read().mLValSize;
        let mut idx = (*e).mStackFrame.mLValSize - 1;
        while idx > start {
            if (*e).mLocals.as_ptr().add(idx as usize).read().mNameKey == key {
                return (*e).mLocals.as_mut_ptr().add(idx as usize);
            }
            idx -= 1;
        }
    }

    /* otherwise, try global vars */
    let val = find_global(key, vm);
    if !val.is_null() {
        return val;
    }

    /* fallback: FindObjProp on current class */
    find_obj_prop(
        &(*e).mStackFrame.mClass as *const BoyiaValue,
        key,
        vm,
    )
}

/// Get pointer to BoyiaValue for REG0, REG1, or VAR operand. Returns null for constant operands.
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
        OpType::OP_VAR => get_val(op.mValue as LUintPtr, vm),
        _ => ptr::null_mut(),
    }
}

/// Next instruction by mNext offset; null if kInvalidInstruction.
#[inline]
unsafe fn next_instruction(inst: *const Instruction, vm: *mut BoyiaVM) -> *mut Instruction {
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

    (*state).mFun.mValue.mObj.mPtr = 0;
    (*state).mFun.mValue.mObj.mSuper = 0;
    (*state).mFun.mValueType = ValueType::BY_ARG;
    (*state).mWait = LFalse;

    assign_state_class(state, ptr::null());
}

/// Push scene with null inst (mLValSize=0, mPC=null). Used by native_call_impl. Match HandlePushScene(kBoyiaNull).
pub(crate) unsafe fn push_scene_null(vm: *mut BoyiaVM) {
    let e_state = (*vm).mEState;
    if e_state.is_null() {
        return;
    }
    let frame_index = (*e_state).mFrameIndex as usize;
    if frame_index >= FUNC_CALLS {
        return;
    }
    let exec_stack = (*e_state).mExecStack.as_mut_ptr();
    let frame = exec_stack.add(frame_index);
    (*frame).mLValSize = 0;
    (*frame).mPC = ptr::null_mut();
    (*frame).mContext = (*e_state).mStackFrame.mContext;
    (*frame).mResultNum = (*e_state).mStackFrame.mResultNum;
    (*frame).mLoopSize = (*e_state).mStackFrame.mLoopSize;
    (*frame).mClass = (*e_state).mStackFrame.mClass;
    (*e_state).mFrameIndex += 1;
}

/// Run callback after args copied to current state's locals. Used by native_call_impl. Dispatches BY_NAV_PROP to native or sets context and runs script.
pub(crate) unsafe fn run_callback_after_args(vm: *mut BoyiaVM, obj: *mut BoyiaValue) -> LInt {
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mFrameIndex <= 0 {
        return OpHandleResult::kOpResultEnd as i32;
    }
    let start = (*e_state).mExecStack.as_ptr().add((*e_state).mFrameIndex as usize - 1).read().mLValSize;
    let value = (*e_state).mLocals.as_mut_ptr().add(start as usize);
    let func = (*value).mValue.mObj.mPtr as *mut BoyiaFunction;
    value_copy(&mut (*e_state).mFun, value);
    let value_type = (*value).mValueType;
    if value_type == ValueType::BY_NAV_PROP {
        local_push(obj, vm as *mut LVoid);
        let nav_fun = std::mem::transmute::<_, crate::types::NativePtr>((*func).mFuncBody);
        let r = nav_fun(vm as *mut LVoid);
        return r;
    }
    assign_state_class(e_state, obj);
    let cmds = (*func).mFuncBody as *const CommandTable;
    (*e_state).mStackFrame.mContext = cmds as *mut CommandTable;
    (*e_state).mStackFrame.mPC = (*cmds).mBegin;
    exec_instruction(vm);
    OpHandleResult::kOpResultSuccess as i32
}

/// When PC is null, pop frame or switch back to mLast (MicroTask/async). Match ExecPopFunction in BoyiaCore.cpp.
unsafe fn exec_pop_function(vm: *mut BoyiaVM) {
    if vm.is_null() || (*vm).mEState.is_null() {
        return;
    }
    let e_state = (*vm).mEState;
    if !(*e_state).mStackFrame.mPC.is_null() {
        return;
    }

    // At top of frame stack: optionally switch back to caller (mLast) and destroy current state (MicroTask/async).
    if (*e_state).mFrameIndex <= 0 {
        let last = (*e_state).mLast;
        if !last.is_null() && (*last).mWait == 0 {
            let current_state = e_state;
            switch_exec_state(last, vm);
            if (*current_state).mWait == 0 {
                if !(*current_state).mTopTask.is_null() {
                    free_micro_task((*current_state).mTopTask, vm);
                }
                destroy_exec_state(current_state, vm);
            }
        } else {
            return;
        }
    }

    // Pop one frame if still in a nested call (SwitchExecState may have changed mEState).
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mFrameIndex <= 0 {
        return;
    }
    (*e_state).mFrameIndex -= 1;
    let idx = (*e_state).mFrameIndex as usize;
    (*e_state).mStackFrame.mLValSize = (*e_state).mExecStack[idx].mLValSize;
    (*e_state).mStackFrame.mPC = (*e_state).mExecStack[idx].mPC;
    (*e_state).mStackFrame.mContext = (*e_state).mExecStack[idx].mContext;
    (*e_state).mStackFrame.mLoopSize = (*e_state).mExecStack[idx].mLoopSize;
    (*e_state).mStackFrame.mClass = (*e_state).mExecStack[idx].mClass;
    if !(*e_state).mStackFrame.mPC.is_null() {
        let pc = (*e_state).mStackFrame.mPC;
        (*e_state).mStackFrame.mPC = next_instruction(pc, vm);
        exec_pop_function(vm);
    } else if !(*e_state).mLast.is_null() && (*(*e_state).mLast).mWait == 0 {
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
        CmdType::kCmdOnceJmpTrue => handle_once_jmp_true(inst, vm),
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
    let type_val = (*inst).mOPLeft.mValue as u8;
    let name_key = (*inst).mOPRight.mValue as LUintPtr;
    let val = (*vm).mGlobals.add((*vm).mGValSize as usize);
    (*vm).mGValSize += 1;
    (*val).mValueType = std::mem::transmute(type_val);
    (*val).mNameKey = name_key;
    (*val).mValue.mIntVal = 0;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_decl_local(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let type_val = (*inst).mOPLeft.mValue as u8;
    let name_key = (*inst).mOPRight.mValue as LUintPtr;
    let mut local = BoyiaValue {
        mNameKey: name_key,
        mValueType: std::mem::transmute(type_val),
        mValue: RealValue { mIntVal: 0 },
    };
    local_push(&mut local, vm as *mut LVoid);
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
    OpHandleResult::kOpResultEnd
}

unsafe fn handle_sub(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*left).mValueType != ValueType::BY_INT || (*right).mValueType != ValueType::BY_INT {
        return OpHandleResult::kOpResultEnd;
    }
    (*right).mValue.mIntVal = (*left).mValue.mIntVal - (*right).mValue.mIntVal;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_mul(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let left = get_op_value(inst, OpSide::OpLeft, vm);
    let right = get_op_value(inst, OpSide::OpRight, vm);
    if left.is_null() || right.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*left).mValueType != ValueType::BY_INT || (*right).mValueType != ValueType::BY_INT {
        return OpHandleResult::kOpResultEnd;
    }
    (*right).mValue.mIntVal *= (*left).mValue.mIntVal;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_div(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
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
    (*right).mValue.mIntVal = (*left).mValue.mIntVal / (*right).mValue.mIntVal;
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
unsafe fn handle_push_scene(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
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
        inst.offset((*inst).mOPLeft.mValue as isize) as *mut Instruction
    };
    (*frame).mContext = (*e_state).mStackFrame.mContext;
    (*frame).mResultNum = (*e_state).mStackFrame.mResultNum;
    (*frame).mLoopSize = (*e_state).mStackFrame.mLoopSize;
    (*e_state).mFrameIndex += 1;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_pop_scene(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let _ = inst;
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mFrameIndex <= 0 {
        return OpHandleResult::kOpResultEnd;
    }
    (*e_state).mFrameIndex -= 1;
    let idx = (*e_state).mFrameIndex as usize;
    (*e_state).mStackFrame.mLValSize = (*e_state).mExecStack[idx].mLValSize;
    (*e_state).mStackFrame.mPC = (*e_state).mExecStack[idx].mPC;
    (*e_state).mStackFrame.mContext = (*e_state).mExecStack[idx].mContext;
    (*e_state).mStackFrame.mLoopSize = (*e_state).mExecStack[idx].mLoopSize;
    (*e_state).mStackFrame.mClass = (*e_state).mExecStack[idx].mClass;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_push_arg(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let value = get_op_value(inst, OpSide::OpLeft, vm);
    if !value.is_null() {
        eprintln!("[handle_push_arg] value.mNameKey={}", (*value).mNameKey);
        local_push(value, vm as *mut LVoid);
        return OpHandleResult::kOpResultSuccess;
    }
    OpHandleResult::kOpResultEnd
}

unsafe fn assign_state_class(state: *mut ExecState, value: *const BoyiaValue) {
    if state.is_null() {
        return;
    }
    if value.is_null() {
        (*state).mStackFrame.mClass.mValue.mObj.mPtr = 0;
        (*state).mStackFrame.mClass.mValue.mObj.mSuper = 0;
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
        let obj_key = (*inst).mOPLeft.mValue as LUintPtr;
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
            let val = (*inst).mOPRight.mValue as *const BoyiaValue;
            if !val.is_null() {
                value_copy_no_name(left, val);
            }
        }
        OpType::OP_CONST_NUMBER => {
            (*left).mValueType = ValueType::BY_INT;
            (*left).mValue.mIntVal = (*inst).mOPRight.mValue;
        }
        OpType::OP_VAR => {
            let val = get_val((*inst).mOPRight.mValue as LUintPtr, vm);
            if val.is_null() {
                return OpHandleResult::kOpResultEnd;
            }
            if (*val).mValueType == ValueType::BY_VAR {
                println!("[handle_assignment] BY_VAR val.mNameKey={}", (*val).mNameKey);
                value_copy(left, val);
            } else {
                value_copy_no_name(left, val);

                println!("[handle_assignment] val.mNameKey={}", (*val).mNameKey);
                (*left).mNameKey = val as LUintPtr;
            }
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
        let offset = (*inst).mOPRight.mValue as isize;
        (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_once_jmp_true(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    if (*inst).mOPLeft.mType != OpType::OP_CONST_NUMBER {
        return OpHandleResult::kOpResultEnd;
    }
    if (*inst).mOPLeft.mValue == 0 {
        let offset = (*inst).mOPRight.mValue as isize;
        (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
    }
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_if_end(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let offset = (*inst).mOPLeft.mValue as isize;
    (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_jump_to(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let offset = (*inst).mOPLeft.mValue as isize;
    (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_loop_begin(_inst: *const Instruction, _vm: *mut BoyiaVM) -> OpHandleResult {
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_loop_if_true(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let value = get_op_value(inst, OpSide::OpLeft, vm);
    if value.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    if (*value).mValue.mIntVal == 0 {
        let offset = (*inst).mOPRight.mValue as isize;
        (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
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

unsafe fn handle_break(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let offset = (*inst).mOPLeft.mValue as isize;
    (*(*vm).mEState).mStackFrame.mPC = inst.offset(offset) as *mut Instruction;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_call_native(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let idx = (*inst).mOPLeft.mValue as LInt;
    let r = crate::core::native_call_by_index(vm as *mut LVoid, idx);
    op_handle_result_from_i32(r)
}

/// Find object property by key; search instance params then class chain. Match FindObjProp (no inline cache).
unsafe fn find_obj_prop(lval: *const BoyiaValue, rval: LUintPtr, _vm: *mut BoyiaVM) -> *mut BoyiaValue {
    if lval.is_null() {
        eprintln!("[find_obj_prop] lval is null");
        return ptr::null_mut();
    }
    if (*lval).mValueType != ValueType::BY_CLASS {
        eprintln!(
            "[find_obj_prop] lval not BY_CLASS (type={}), rval={}",
            (*lval).mValueType as u8, rval
        );
        return ptr::null_mut();
    }
    let fun = (*lval).mValue.mObj.mPtr as *mut BoyiaFunction;
    if fun.is_null() {
        eprintln!("[find_obj_prop] lval->mPtr (instance body) is null, rval={}", rval);
        return ptr::null_mut();
    }
    let klass = (*fun).mFuncBody as *const BoyiaValue;
    for i in 0..(*fun).mParamSize {
        if (*fun).mParams.add(i as usize).read().mNameKey == rval {
            return (*fun).mParams.add(i as usize);
        }
    }
    let mut cls = klass;
    while !cls.is_null() && (*cls).mValueType == ValueType::BY_CLASS {
        let cls_map = (*cls).mValue.mObj.mPtr as *const BoyiaFunction;
        if cls_map.is_null() {
            eprintln!(
                "[find_obj_prop] class {:?} has null mPtr (no member table), rval={}",
                cls, rval
            );
            break;
        }
        let class_key = (*cls).mNameKey;
        let n = (*cls_map).mParamSize as usize;
        for fi in 0..n {
            if (*cls_map).mParams.add(fi).read().mNameKey == rval {
                return (*cls_map).mParams.add(fi) as *mut BoyiaValue;
            }
        }
        let keys: Vec<LUintPtr> = (0..n).map(|i| (*cls_map).mParams.add(i).read().mNameKey).collect();
        eprintln!(
            "[find_obj_prop] class key={} mParamSize={} methods={:?}, lookup rval={}",
            class_key, n, keys, rval
        );
        cls = (*cls).mValue.mObj.mSuper as *const BoyiaValue;
    }
    eprintln!("[find_obj_prop] prop rval={} not found (instance+class chain)", rval);
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
        CmdType::kCmdLtRelation => (*left).mValue.mIntVal < (*right).mValue.mIntVal,
        CmdType::kCmdLeRelation => (*left).mValue.mIntVal <= (*right).mValue.mIntVal,
        CmdType::kCmdGtRelation => (*left).mValue.mIntVal > (*right).mValue.mIntVal,
        CmdType::kCmdGeRelation => (*left).mValue.mIntVal >= (*right).mValue.mIntVal,
        CmdType::kCmdEqRelation => (*left).mValue.mIntVal == (*right).mValue.mIntVal,
        CmdType::kCmdNeRelation => (*left).mValue.mIntVal != (*right).mValue.mIntVal,
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

unsafe fn handle_call_function(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let _ = inst;
    let e_state = (*vm).mEState;
    if e_state.is_null() || (*e_state).mFrameIndex <= 0 {
        eprintln!("[handle_call_function] e_state null or mFrameIndex<=0");
        return OpHandleResult::kOpResultEnd;
    }
    let start = (*e_state).mExecStack.as_ptr().add((*e_state).mFrameIndex as usize - 1).read().mLValSize;
    if start as usize >= crate::types::NUM_LOCAL_VARS {
        eprintln!("[handle_call_function] start={} >= mLocals.len()", start);
        return OpHandleResult::kOpResultEnd;
    }
    let value = (*e_state).mLocals.as_mut_ptr().add(start as usize);
    let func = (*value).mValue.mObj.mPtr as *mut BoyiaFunction;
    if func.is_null() {
        eprintln!(
            "[handle_call_function] func is null: value type={}, start={}",
            (*value).mValueType as u8, start
        );
        return OpHandleResult::kOpResultEnd;
    }
    value_copy(&mut (*e_state).mFun, value);
    let value_type = (*value).mValueType;
    if value_type == ValueType::BY_NAV_FUNC || value_type == ValueType::BY_NAV_PROP {
        local_push(&mut (*e_state).mStackFrame.mClass, vm as *mut LVoid);
        let nav_fun = std::mem::transmute::<_, crate::types::NativePtr>((*func).mFuncBody);
        (*e_state).mStackFrame.mPC = ptr::null_mut();
        let r = nav_fun(vm as *mut LVoid);
        return op_handle_result_from_i32(r);
    }
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
    }
    let cmds = (*func).mFuncBody as *const CommandTable;
    if cmds.is_null() {
        eprintln!(
            "[handle_call_function] mFuncBody is null (cmds): value_type={}",
            value_type as u8
        );
        return OpHandleResult::kOpResultEnd;
    }
    if (*cmds).mBegin.is_null() {
        eprintln!("[handle_call_function] cmds->mBegin is null");
        return OpHandleResult::kOpResultEnd;
    }
    (*e_state).mStackFrame.mContext = cmds as *mut CommandTable;
    (*e_state).mStackFrame.mPC = (*cmds).mBegin;
    OpHandleResult::kOpResultJumpFun
}

unsafe fn handle_get_prop(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let lval = get_op_value(inst, OpSide::OpLeft, vm);
    if lval.is_null() {
        eprintln!("[handle_get_prop] get_op_value(OpLeft) returned null");
        return OpHandleResult::kOpResultEnd;
    }
    let rval = (*inst).mOPRight.mValue as LUintPtr;
    let result = find_obj_prop(lval, rval, vm);
    if result.is_null() {
        eprintln!(
            "[handle_get_prop] find_obj_prop failed: lval type={}, rval(prop key)={}",
            (*lval).mValueType as u8, rval
        );
        return OpHandleResult::kOpResultEnd;
    }
    value_copy(&mut (*(*vm).mCpu).mReg0, result);
    (*(*vm).mCpu).mReg0.mNameKey = (*result).mNameKey;
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
    let value: *mut BoyiaValue = if (*left).mValueType == ValueType::BY_VAR {
        println!("[handle_assign_var] BY_BAR");
        get_val((*left).mNameKey, vm)
    } else {
        println!("[handle_assign_var] not BY_BAR");
        (*left).mNameKey as *mut BoyiaValue
    };

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
    let hash_key = (*inst).mOPLeft.mValue as LUintPtr;

    println!("[handle_create_class] called key = {}", hash_key);
    let class_val = crate::core::create_fun_val(hash_key, ValueType::BY_CLASS, vm);
    if class_val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    assign_state_class((*vm).mEState, class_val);
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_extend(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let class_val = find_global((*inst).mOPLeft.mValue as LUintPtr, vm);
    let extend_val = find_global((*inst).mOPRight.mValue as LUintPtr, vm);
    if class_val.is_null() || extend_val.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    (*class_val).mValue.mObj.mSuper = extend_val as LIntPtr;
    OpHandleResult::kOpResultSuccess
}

/// HandleFunCreate per BoyiaCore.cpp (strict 1:1).
/// Branch order: (1) in class and funType != BY_ANONYM_FUNC -> add method; (2) in class and BY_ANONYM_FUNC -> hashKey != 0: SetIntResult(hashKey); else InitFunction + if hashKey==0 patch inst and SetIntResult(mFunSize-1); (3) else CreateFunVal(hashKey, BY_FUNC).
unsafe fn handle_fun_create(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let hash_key = (*inst).mOPLeft.mValue as LUintPtr;
    let fun_type = (*inst).mOPRight.mValue as u8;
    let e_state = (*vm).mEState;

    if !e_state.is_null() && (*e_state).mStackFrame.mClass.mValue.mObj.mPtr != 0 {
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
                        mSuper: if is_prop_func { func as LIntPtr } else { 0 },
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
            (*inst_mut).mOPLeft.mValue = ((*vm).mFunSize - 1) as LIntPtr;
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
    if (*inst).mOPLeft.mValue != -1 {
        let code = (*(*vm).mVMCode).mCode;
        (*new_table).mBegin = code.offset((*inst).mOPLeft.mValue as isize);
        (*new_table).mEnd = code.offset((*inst).mOPRight.mValue as isize);
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
    let hash_key = (*inst).mOPLeft.mValue as LUintPtr;
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
    let e_state = (*vm).mEState;
    if e_state.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let func = (*e_state).mStackFrame.mClass.mValue.mObj.mPtr as *mut BoyiaFunction;
    if func.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let idx = (*func).mParamSize as usize;
    (*func).mParams.add(idx).write(BoyiaValue {
        mNameKey: (*inst).mOPLeft.mValue as LUintPtr,
        mValueType: ValueType::BY_ARG,
        mValue: RealValue { mIntVal: 0 },
    });
    (*func).mParamSize += 1;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_create_map(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let map_key = (*inst).mOPRight.mValue as LUintPtr;
    if map_key == 0 {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = copy_object(map_key, 32, vm as *mut LVoid) as *mut BoyiaFunction;
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
    (*value).mValue.mObj.mSuper = 0;
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_set_map_key(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let value = if (*inst).mOPLeft.mType == OpType::OP_REG0 {
        &(*(*vm).mCpu).mReg0
    } else {
        &(*(*vm).mCpu).mReg1
    };
    let function = (*value).mValue.mObj.mPtr as *mut BoyiaFunction;
    if function.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let idx = (*function).mParamSize as usize;
    (*function).mParams.add(idx).write(BoyiaValue {
        mNameKey: (*inst).mOPRight.mValue as LUintPtr,
        mValueType: ValueType::BY_ARG,
        mValue: RealValue { mIntVal: 0 },
    });
    (*function).mParamSize += 1;
    OpHandleResult::kOpResultSuccess
}

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
    if function.is_null() || (*function).mParamSize <= 0 {
        return OpHandleResult::kOpResultEnd;
    }
    let param = (*function).mParams.add((*function).mParamSize as usize - 1);
    value_copy_no_name(param, value);
    set_native_result(obj as *const BoyiaValue as *mut LVoid, vm as *mut LVoid);
    OpHandleResult::kOpResultSuccess
}

unsafe fn handle_create_array(inst: *const Instruction, vm: *mut BoyiaVM) -> OpHandleResult {
    let array_key = (*inst).mOPRight.mValue as LUintPtr;
    if array_key == 0 {
        return OpHandleResult::kOpResultEnd;
    }
    let fun = copy_object(array_key, 32, vm as *mut LVoid) as *mut BoyiaFunction;
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
    (*value).mValue.mObj.mSuper = 0;
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
    let str_idx = (*inst).mOPLeft.mValue as usize;
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
    (*e_state).mWait = 1;
    let fun = (*left).mValue.mObj.mPtr as *const BoyiaFunction;
    if fun.is_null() {
        return OpHandleResult::kOpResultEnd;
    }
    let task = (*fun).mParams.add(1).read().mValue.mIntVal as *mut MicroTask;
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
                            mSuper: 0,
                        },
                    },
                };
                set_native_result(&mut result as *mut BoyiaValue as *mut LVoid, vm as *mut LVoid);
            }
        }
    }
    let last = (*e_state).mLast;
    if !last.is_null() && (*last).mWait == 0 {
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

/// Execute instructions until PC is null or handler returns kOpResultEnd.
pub unsafe fn exec_instruction(vm: *mut BoyiaVM) {
    eprintln!("[exec_instruction] enter");
    if vm.is_null() || (*vm).mEState.is_null() {
        return;
    }
    let e_state = (*vm).mEState;
    eprintln!("[exec_instruction] pc={:?}", (*e_state).mStackFrame.mPC);
    if (*e_state).mStackFrame.mPC.is_null() {
        exec_pop_function(vm);
        return;
    }
    eprintln!("[exec_instruction] dispatching first inst");
    while !(*e_state).mStackFrame.mPC.is_null() {
        let pc = (*e_state).mStackFrame.mPC;
        let result = dispatch_instruction(pc, vm);
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
        execute_code(vm);
    }
}

/// Execute code for current mStackFrame.mContext (set by caller). Resets scene after.
pub unsafe fn execute_code(vm: *mut BoyiaVM) {
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
