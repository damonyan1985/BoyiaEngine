//! Type definitions and constants for Boyia VM
//! Enums match BoyiaCore.cpp / BoyiaValue.h; constants are derived from enums for compatibility.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]

use std::ffi::c_void;
use std::os::raw::{c_char, c_int};

// ---------------------------------------------------------------------------
// Numeric constants (BoyiaCore.cpp #define)
// ---------------------------------------------------------------------------
pub const NUM_FUNC: usize = 1024;
pub const NUM_FUNC_PARAMS: usize = 32;
pub const NUM_GLOBAL_VARS: usize = 512;
pub const NUM_LOCAL_VARS: usize = 512;
pub const NUM_RESULT: usize = 128;
pub const FUNC_CALLS: usize = 32;
pub const LOOP_NEST: usize = 32;
pub const CODE_CAPACITY: usize = 1024 * 32;
pub const CONST_CAPACITY: usize = 1024;
pub const ENTRY_CAPACITY: usize = 1024;
pub const MICRO_TASK_CAPACITY: usize = 1024;
pub const EXEC_STATE_CAPACITY: usize = 64;
/// Max inline cache entries per instruction (BoyiaValue.h MAX_INLINE_CACHE).
pub const MAX_INLINE_CACHE: usize = 5;

// Type definitions (PlatformLib.h) - needed for constants below
pub type LInt8 = c_char;
pub type LInt = c_int;
pub type LUint8 = u8;
pub type LUintPtr = usize;
pub type LIntPtr = isize;
pub type LVoid = c_void;
pub type LBool = LInt;

// ---------------------------------------------------------------------------
// Runtime trait (VM creator: native lookup and call)
// ---------------------------------------------------------------------------

/// Runtime interface: native function lookup and call. Stored in [BoyiaVM] as [BoyiaVM::mCreator].
pub trait Runtime {
    /// Find native function index by name key; -1 if not found.
    fn find_native_func(&self, key: LUintPtr) -> LInt;
    /// Call native function by index; returns op result (e.g. kOpResultSuccess).
    fn call_native_function(&self, idx: LInt) -> LInt;
    /// Get or assign id for a string key (e.g. compile token name).
    fn gen_identifier(&mut self, key: &str) -> LUintPtr;
    /// Get or assign id for a string from [BoyiaStr] (e.g. Map key, builtins).
    fn gen_ident_by_str(&mut self, s: *const BoyiaStr) -> LUintPtr;
}

// ---------------------------------------------------------------------------
// Enums from BoyiaValue.h / BoyiaCore.cpp (same layout as C++)
// ---------------------------------------------------------------------------

/// ValueType: type of a BoyiaValue (runtime value type). Replaces KeyWord for mValueType / create_fun_val. Same repr(u8) as C.
#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum ValueType {
    BY_ARG = 0,
    BY_CHAR,
    BY_INT,
    BY_STRING,
    BY_FUNC,
    BY_PROP_FUNC,
    BY_ASYNC,
    BY_ASYNC_PROP,
    BY_NAV_FUNC,
    BY_NAV_PROP,
    BY_ANONYM_FUNC,
    BY_AWAIT,
    BY_NAVCLASS,
    BY_CLASS,
    BY_EXTEND,
    BY_VAR,
    BY_PROP,
    BY_IF,
    BY_ELIF,
    BY_ELSE,
    BY_DO,
    BY_WHILE,
    BY_FOR,
    BY_BREAK,
    BY_RETURN,
    BY_END,
}

/// OpHandleResult (BoyiaValue.h)
#[repr(i32)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum OpHandleResult {
    kOpResultEnd = 0,
    kOpResultSuccess = 1,
    kOpResultJumpFun = 2,
}

/// BuiltinId (BoyiaValue.h enum BuiltinId). Reserved name keys for this, super, and builtin classes.
/// GetVal uses kBoyiaThis/kBoyiaSuper; CreateGlobalClass/CopyObject use String/Array/Map/MicroTask.
#[repr(u32)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum BuiltinId {
    kBoyiaThis = 1,
    kBoyiaSuper,
    kBoyiaString,
    kBoyiaArray,
    kBoyiaMap,
    kBoyiaMicroTask,
}

impl BuiltinId {
    #[inline]
    pub fn as_key(self) -> LUintPtr {
        self as u32 as LUintPtr
    }
}

/// TokenType (BoyiaCore.cpp). NONE = 0 for clear/reset.
#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum TokenType {
    NONE = 0,
    DELIMITER = 1,
    IDENTIFIER,
    NUMBER,
    KEYWORD,
    TEMP,
    VARIABLE,
    STRING_VALUE,
}

/// Token value: 0 = NONE (or KEYWORD BY_ARG in C), 1..25 = keywords, 26..52 = delimiter/logic/math/bracket. Same u8 layout as C.
#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum TokenValue {
    /// 0: no value (IDENTIFIER/NUMBER) or KEYWORD BY_ARG in C
    NONE = 0,
    // KeyWord 1..=25 (same order as KeyWord, skip BY_ARG=0)
    BY_CHAR = 1,
    BY_INT,
    BY_STRING,
    BY_FUNC,
    BY_PROP_FUNC,
    BY_ASYNC,
    BY_ASYNC_PROP,
    BY_NAV_FUNC,
    BY_NAV_PROP,
    BY_ANONYM_FUNC,
    BY_AWAIT,
    BY_NAVCLASS,
    BY_CLASS,
    BY_EXTEND,
    BY_VAR,
    BY_PROP,
    BY_IF,
    BY_ELIF,
    BY_ELSE,
    BY_DO,
    BY_WHILE,
    BY_FOR,
    BY_BREAK,
    BY_RETURN,
    BY_END,
    // TokenLogicValue (26..=34)
    AND = 26,
    OR,
    NOT,
    LT,
    LE,
    GT,
    GE,
    EQ,
    NE,
    // TokenMathValue (35..=41)
    ADD = 35,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    ASSIGN,
    // TokenDelimiValue (42..=46)
    SEMI = 42,
    COMMA,
    QUOTE,
    DOT,
    COLON,
    // TokenBracketValue (47..=52)
    LPTR = 47,
    RPTR,
    ARRAY_BEGIN,
    ARRAY_END,
    BLOCK_START,
    BLOCK_END,
}

impl From<ValueType> for TokenValue {
    fn from(k: ValueType) -> Self {
        if k as u8 == 0 {
            TokenValue::NONE
        } else {
            unsafe { std::mem::transmute(k as u8) }
        }
    }
}

impl PartialEq<ValueType> for TokenValue {
    fn eq(&self, other: &ValueType) -> bool {
        *self as u8 == *other as u8
    }
}
impl PartialEq<TokenValue> for ValueType {
    fn eq(&self, other: &TokenValue) -> bool {
        *self as u8 == *other as u8
    }
}

impl PartialEq<u8> for TokenValue {
    fn eq(&self, other: &u8) -> bool {
        *self as u8 == *other
    }
}
impl PartialEq<TokenValue> for u8 {
    fn eq(&self, other: &TokenValue) -> bool {
        *self == *other as u8
    }
}

impl PartialEq<u8> for ValueType {
    fn eq(&self, other: &u8) -> bool {
        *self as u8 == *other
    }
}
impl PartialEq<ValueType> for u8 {
    fn eq(&self, other: &ValueType) -> bool {
        *self == *other as u8
    }
}

/// OpType (BoyiaCore.cpp)
#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum OpType {
    OP_NONE = 0,
    OP_CONST_NUMBER,
    OP_CONST_STRING,
    OP_REG0,
    OP_REG1,
    OP_VAR,
}

/// CmdType (BoyiaCore.cpp)
#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum CmdType {
    kCmdJmpTrue = 0,
    kCmdIfEnd,
    kCmdElif,
    kCmdElse,
    kCmdElEnd,
    kCmdPushObj,
    kCmdAdd,
    kCmdSub,
    kCmdMul,
    kCmdDiv,
    kCmdMod,
    kCmdAndLogic,
    kCmdOrLogic,
    kCmdGtRelation,
    kCmdLtRelation,
    kCmdGeRelation,
    kCmdLeRelation,
    kCmdNotRelation,
    kCmdEqRelation,
    kCmdNeRelation,
    kCmdPush,
    kCmdPop,
    kCmdPushScene,
    kCmdPopScene,
    kCmdPushArg,
    kCmdPushParams,
    kCmdCallFunction,
    kCmdCallNative,
    kCmdDeclGlobal,
    kCmdDeclLocal,
    kCmdCreateClass,
    kCmdCreateFunction,
    kCmdClassExtend,
    kCmdDeclConstStr,
    kCmdExecCreate,
    kCmdParamCreate,
    kCmdPropCreate,
    kCmdFunCreate,
    kCmdAssign,
    kCmdAssignVar,
    kCmdLoopTrue,
    kCmdJmpTo,
    kCmdGetProp,
    kCmdConstStr,
    kCmdLoop,
    kCmdBreak,
    kCmdReturn,
    kCmdCreateMap,
    kCmdSetMapKey,
    kCmdSetMapValue,
    kCmdCreateArray,
    kCmdAddArrayItem,
    kCmdAwait,
    kCmdSetAnonym,
    kCmdOnceJmpTrue,
    kCmdEnd,
}

/// OpInstType (BoyiaCore.cpp)
#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum OpInstType {
    OpLeft = 0,
    OpRight,
}

// Use enums directly: ValueType::BY_VAR, CmdType::kCmdAdd, etc.

pub const kInvalidInstruction: LIntPtr = -1;
pub const LFalse: LInt = 0;
pub const LTrue: LInt = 1;

/// Handler return type: use OpHandleResult for dispatch.
pub type OPHandler = unsafe extern "C" fn(*mut Instruction, *mut BoyiaVM) -> OpHandleResult;

// BoyiaValue types matching BoyiaValue.h
#[repr(C)]
#[derive(Clone, Copy)]
pub struct BoyiaStr {
    pub mPtr: *mut LInt8,
    pub mLen: LInt,
}

/// Token: name (slice into source), type, value. Matches BoyiaCore.cpp BoyiaToken.
#[repr(C)]
#[derive(Clone, Copy)]
pub struct BoyiaToken {
    pub mTokenName: BoyiaStr,
    pub mTokenType: TokenType,
    pub mTokenValue: TokenValue,
}

/// Compile state: matches BoyiaCore.cpp CompileState.
#[repr(C)]
pub struct CompileState {
    pub mProg: *mut LInt8,
    pub mLineNum: LInt,
    pub mColumnNum: LInt,
    pub mToken: BoyiaToken,
    pub mVm: *mut BoyiaVM,
    pub mCmds: CommandTable,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct BoyiaClass {
    pub mPtr: LIntPtr,
    pub mSuper: LIntPtr,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub union RealValue {
    pub mIntVal: LIntPtr,
    pub mObj: BoyiaClass,
    pub mStrVal: BoyiaStr,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct BoyiaValue {
    pub mNameKey: LUintPtr,
    pub mValueType: ValueType,
    pub mValue: RealValue,
}

#[repr(C)]
pub struct BoyiaFunction {
    pub mFuncBody: LIntPtr,
    pub mParams: *mut BoyiaValue,
    pub mParamSize: LInt,
    pub mParamCount: LInt,
}

/// Inline cache entry type (BoyiaValue.cpp InlineCacheType).
pub const CACHE_PROP: LInt = 1;
pub const CACHE_METHOD: LInt = 2;

#[repr(C)]
pub struct InlineCacheItem {
    pub mClass: *mut BoyiaValue,
    pub mIndex: LIntPtr,
    pub mType: LInt,
}

#[repr(C)]
pub struct InlineCache {
    pub mItems: [InlineCacheItem; MAX_INLINE_CACHE],
    pub mSize: LInt,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct OpCommand {
    pub mType: OpType,
    pub mValue: LIntPtr,
}

impl OpCommand {
    /// No operand (OP_NONE, 0). Used when the instruction has no left/right operand.
    #[inline]
    pub const fn none() -> Self {
        Self {
            mType: OpType::OP_NONE,
            mValue: 0,
        }
    }

    /// Constant number operand (OP_CONST_NUMBER, value).
    #[inline]
    pub const fn const_number(value: LIntPtr) -> Self {
        Self {
            mType: OpType::OP_CONST_NUMBER,
            mValue: value,
        }
    }

    /// Register 0 operand (OP_REG0, 0).
    #[inline]
    pub const fn reg0() -> Self {
        Self {
            mType: OpType::OP_REG0,
            mValue: 0,
        }
    }

    /// Register 1 operand (OP_REG1, 0).
    #[inline]
    pub const fn reg1() -> Self {
        Self {
            mType: OpType::OP_REG1,
            mValue: 0,
        }
    }

    /// Variable by key (OP_VAR, key).
    #[inline]
    pub const fn op_var(key: LIntPtr) -> Self {
        Self {
            mType: OpType::OP_VAR,
            mValue: key,
        }
    }
}

#[repr(C)]
pub struct Instruction {
    pub mOPCode: CmdType,
    pub mOPLeft: OpCommand,
    pub mOPRight: OpCommand,
    pub mCache: *mut InlineCache,
    pub mNext: LIntPtr,
}

#[repr(C)]
pub struct CommandTable {
    pub mBegin: *mut Instruction,
    pub mEnd: *mut Instruction,
}

#[repr(C)]
pub struct StackFrame {
    pub mPC: *mut Instruction,
    pub mLValSize: LInt,
    pub mLoopSize: LInt,
    pub mResultNum: LInt,
    pub mContext: *mut CommandTable,
    pub mClass: BoyiaValue,
}

#[repr(C)]
pub struct VMCpu {
    pub mReg0: BoyiaValue,
    pub mReg1: BoyiaValue,
}

#[repr(C)]
pub struct VMCode {
    pub mCode: *mut Instruction,
    pub mSize: LInt,
}

#[repr(C)]
pub struct VMStrTable {
    pub mTable: [BoyiaStr; CONST_CAPACITY],
    pub mSize: LInt,
}

#[repr(C)]
pub struct VMEntryTable {
    pub mTable: [LInt; ENTRY_CAPACITY],
    pub mSize: LInt,
}

#[repr(C)]
pub struct MicroTask {
    pub mResult: BoyiaValue,
    pub mObjRef: BoyiaValue,
    pub mAsyncEs: *mut ExecState,
    pub mNext: *mut MicroTask,
    pub mAllocLink: MicroTaskLink,
}

#[repr(C)]
pub struct MicroTaskLink {
    pub mLinkNext: *mut MicroTask,
    pub mLinkPrev: *mut MicroTask,
}

#[repr(C)]
pub struct MicroTaskQueue {
    pub mUsedTasks: MicroTaskList,
    pub mAllocTasks: MicroTaskList,
    pub mTaskCache: *mut c_void, // MemoryCache pointer
}

#[repr(C)]
pub struct MicroTaskList {
    pub mHead: *mut MicroTask,
    pub mEnd: *mut MicroTask,
}

#[repr(C)]
pub struct ExecState {
    pub mStackFrame: StackFrame,
    pub mFrameIndex: LInt,
    pub mLocals: [BoyiaValue; NUM_LOCAL_VARS],
    pub mOpStack: [BoyiaValue; NUM_RESULT],
    pub mLoopStack: [LIntPtr; LOOP_NEST],
    pub mFun: BoyiaValue,
    pub mTopTask: *mut MicroTask,
    pub mLast: *mut ExecState,
    pub mPrev: *mut ExecState,
    pub mNext: *mut ExecState,
    pub mExecStack: [StackFrame; FUNC_CALLS],
    pub mWait: LBool,
}

// Main VM structure. mCreator is *mut dyn Runtime (fat pointer).
#[repr(C)]
pub struct BoyiaVM {
    pub mFunTable: *mut BoyiaFunction,
    pub mGlobals: *mut BoyiaValue,
    pub mGValSize: LInt,
    pub mOpStack: *mut BoyiaValue,
    pub mLoopStack: *mut LIntPtr,
    pub mLocals: *mut BoyiaValue,
    pub mExecStack: *mut StackFrame,
    pub mCpu: *mut VMCpu,
    pub mESLink: *mut ExecState,
    pub mEState: *mut ExecState,
    pub mEStateCache: *mut c_void,
    pub mVMCode: *mut VMCode,
    pub mStrTable: *mut VMStrTable,
    pub mEntry: *mut VMEntryTable,
    pub mHandlers: *mut OPHandler,
    pub mTaskQueue: *mut MicroTaskQueue,
    pub mFunSize: LInt,
    pub mCreator: *mut dyn Runtime,
}

// Native function pointer type
pub type NativePtr = unsafe extern "C" fn(*mut LVoid) -> LInt;

#[repr(C)]
pub struct NativeFunction {
    pub mNameKey: LUintPtr,
    pub mAddr: NativePtr,
}
