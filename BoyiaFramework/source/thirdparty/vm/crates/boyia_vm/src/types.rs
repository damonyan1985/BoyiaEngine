//! Type definitions and constants for Boyia VM
//! Enums match BoyiaCore.cpp / BoyiaValue.h; constants are derived from enums for compatibility.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]

// Platform types: shared with `boyia_memory` via `boyia_types` (no circular deps). Prefer `boyia_vm::…` in app code.
pub use boyia_types::{
    K_BOYIA_NULL, LBool, LByte, LFalse, LInt, LInt8, LIntPtr, LTrue, LUint8, LUintPtr, LVoid,
};

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

// ---------------------------------------------------------------------------
// Runtime trait (VM creator: native lookup and call)
// ---------------------------------------------------------------------------

/// Runtime interface: VM creator, native lookup/call, and optional memory allocation. Stored in the VM as mCreator.
pub trait Runtime {
    /// Find native function index by name key; -1 if not found.
    fn find_native_func(&self, key: LUintPtr) -> LInt;
    /// Call native function by index; returns op result (e.g. kOpResultSuccess).
    fn call_native_function(&self, idx: LInt) -> LInt;
    /// Get or assign id for a string key (e.g. compile token name).
    fn gen_identifier(&mut self, key: &str) -> LUintPtr;
    /// Get or assign id for a string from [BoyiaStr] (e.g. Map key, builtins).
    fn gen_ident_by_str(&mut self, s: *const BoyiaStr) -> LUintPtr;

    /// Allocate a block from the runtime memory pool. Implemented by [BoyiaRuntime]; other runtimes may return null.
    fn new_data(&self, size: LInt) -> *mut LVoid;

    /// Free a block previously returned by [Runtime::new_data]. Implemented by [BoyiaRuntime]; no-op for other runtimes.
    fn delete_data(&self, data: *mut LVoid);

    /// Current memory pool (for GC migrate). Same as BoyiaRuntime::m_memoryPool in C++.
    fn memory_pool(&self) -> *mut LVoid;

    /// Create a new pool for compaction (CreateRuntimeToMemory). Returns to_pool.
    fn create_runtime_to_memory(&self, _vm: *mut LVoid) -> *mut LVoid;

    /// Switch runtime to use to_pool after compaction (UpdateRuntimeMemory / changeMemoryPool).
    fn update_runtime_memory(&mut self, _to_pool: *mut LVoid, _vm: *mut LVoid);

    /// Register a heap object with the GC (GCAppendRef). GC and VM are obtained from [Runtime::gc_ptr] / [Runtime::vm_ptr]. Implemented by [BoyiaRuntime]; no-op for other runtimes.
    fn gc_append_ref(&self, _address: *mut LVoid, _type_: ValueType);

    /// GC state pointer (for [boyia_gc::gc_append_ref]).
    fn gc_ptr(&self) -> *mut LVoid;
    /// VM pointer (for GC / runtime).
    fn vm_ptr(&self) -> *mut LVoid;

    /// Store a copy of the given [BoyiaValue] in the runtime's persistent object list and return a pointer to the list node ([Global]).
    /// The runtime keeps the value until it is dropped. Used to prevent objects from being collected.
    fn persistent_object(&mut self, value: *const BoyiaValue) -> *mut crate::Global;

    /// Iterate over the persistent object list, calling `f` with each stored value pointer ([*mut BoyiaValue]).
    /// [BoyiaRuntime]: for `BY_ANONYM_FUNC`, if `mPtr == 0` removes the node; if `mPtr != 0` calls `f` only on each capture
    /// in `BoyiaFunction::mParams[mParamSize .. mParamSize + mCaptureCount)` (not on the wrapper `BoyiaValue`); other types: `f(value_ptr)` only. Requires `&mut self` for removals.
    fn iterate_persistent(&mut self, f: &mut dyn FnMut(*mut BoyiaValue));

    /// Remove the node at `ptr` from the runtime's persistent object list. [BoyiaRuntime] calls [crate::GlobalList::remove]; other runtimes may no-op.
    fn remove_persistent(&mut self, ptr: *mut crate::Global);
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
pub(crate) enum TokenType {
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
pub(crate) enum TokenValue {
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
pub(crate) enum OpType {
    OP_NONE = 0,
    OP_CONST_NUMBER,
    OP_CONST_STRING,
    OP_REG0,
    OP_REG1,
    OP_VAR,
    /// Frame slot offset from [ExecState::mExecStack][frame-1].mLValSize: 1..=param_count are params, then locals.
    /// Rust VM only; not in C++ BoyiaCore opcode set.
    OP_LOCAL,
    /// Closure capture slot index in current callee's `BoyiaFunction::mParams` capture tail.
    OP_CAPTURE,
}

/// CmdType (BoyiaCore.cpp)
#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub(crate) enum CmdType {
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
    kCmdPopLocals,
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

/// Handler return type: use OpHandleResult for dispatch.
pub(crate) type OPHandler = unsafe fn(*mut Instruction, *mut BoyiaVM) -> OpHandleResult;

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
pub(crate) struct BoyiaToken {
    pub mTokenName: BoyiaStr,
    pub mTokenType: TokenType,
    pub mTokenValue: TokenValue,
}

/// One block’s local variable names (compile-time only), for [OpType::OP_LOCAL] resolution.
#[derive(Default)]
pub(crate) struct LocalScope {
    pub(crate) mLocals: Vec<LUintPtr>,
}

impl LocalScope {
    pub(crate) fn add_local(&mut self, key: LUintPtr) {
        self.mLocals.push(key);
    }
}

/// One function being compiled: formal parameters + a stack of [LocalScope] (outer blocks first, innermost last).
/// `push_local_scope` / `pop_local_scope` are driven by `{` / `}` in `compile::block_statement` while a function is active.
/// Runtime VM `mLocals[start+1..]` = [mParams] in order, then locals from each [LocalScope] in stack order.
pub(crate) struct FunctionScope {
    pub(crate) mIsAnonym: bool,
    pub(crate) mParams: Vec<LUintPtr>,
    /// Captured outer variable name keys used by nested anonymous functions.
    pub(crate) mCaptures: Vec<LUintPtr>,
    pub(crate) mLocalScopes: Vec<LocalScope>,
}

impl FunctionScope {
    pub(crate) fn new() -> Self {
        Self::new_with_anonym(false)
    }

    pub(crate) fn new_with_anonym(is_anonym: bool) -> Self {
        Self {
            mIsAnonym: is_anonym,
            mParams: Vec::new(),
            mCaptures: Vec::new(),
            mLocalScopes: Vec::new(),
        }
    }

    pub(crate) fn add_param(&mut self, key: LUintPtr) {
        self.mParams.push(key);
    }

    pub(crate) fn push_local_scope(&mut self) {
        self.mLocalScopes.push(LocalScope::default());
    }

    pub(crate) fn pop_local_scope(&mut self) -> Option<LocalScope> {
        self.mLocalScopes.pop()
    }

    pub(crate) fn add_local(&mut self, key: LUintPtr) {
        if let Some(top) = self.mLocalScopes.last_mut() {
            top.add_local(key);
        }
    }

    pub(crate) fn add_capture(&mut self, key: LUintPtr) {
        self.mCaptures.push(key);
    }

    /// Frame offset for `mLocals[start + offset]`; offset 0 = callee; first param = 1.
    /// Locals: concatenation of each [LocalScope] in order; shadowing resolves inner scope first.
    pub(crate) fn resolve_local_frame_offset(&self, key: LUintPtr) -> Option<LIntPtr> {
        let pc = self.mParams.len();
        for si in (0..self.mLocalScopes.len()).rev() {
            if let Some(j) = self.mLocalScopes[si].mLocals.iter().rposition(|&k| k == key) {
                let mut prefix = 0usize;
                for s in 0..si {
                    prefix += self.mLocalScopes[s].mLocals.len();
                }
                return Some((1 + pc + prefix + j) as LIntPtr);
            }
        }
        if let Some(i) = self.mParams.iter().rposition(|&k| k == key) {
            return Some((1 + i) as LIntPtr);
        }
        None
    }
}

impl Default for FunctionScope {
    fn default() -> Self {
        Self::new()
    }
}

/// Compile state: matches BoyiaCore.cpp CompileState (plus Rust-only [FunctionScope] stack for nested functions / [OpType::OP_LOCAL]).
pub(crate) struct CompileState {
    pub mProg: *mut LInt8,
    pub mLineNum: LInt,
    pub mColumnNum: LInt,
    pub mToken: BoyiaToken,
    pub mVm: *mut BoyiaVM,
    pub mCmds: CommandTable,
    /// Innermost = currently compiling function (same role as former `m_scope_stack`).
    pub mFunctionScopes: Vec<FunctionScope>,
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
    pub mCaptureCount: LInt,
    pub mParamCount: LInt,
}

/// Filled by [crate::core::get_callee_and_captures_from_locals]: callee and closure capture region in `mParams`.
#[repr(C)]
#[derive(Clone, Copy)]
pub struct CalleeCapturesInfo {
    pub callee: *mut BoyiaFunction,
    /// First capture cell: `callee->mParams + callee->mParamSize` when `capture_count > 0`.
    pub captures: *mut BoyiaValue,
    pub capture_count: LInt,
}

/// Inline cache entry type (BoyiaValue.cpp InlineCacheType).
pub const CACHE_PROP: LInt = 1;
pub const CACHE_METHOD: LInt = 2;

#[repr(C)]
pub(crate) struct InlineCacheItem {
    pub mClass: *mut BoyiaValue,
    pub mIndex: LIntPtr,
    pub mType: LInt,
}

#[repr(C)]
pub(crate) struct InlineCache {
    pub mItems: [InlineCacheItem; MAX_INLINE_CACHE],
    pub mSize: LInt,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub(crate) struct OpCommand {
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

    /// Local/param by frame offset (OP_LOCAL, offset from frame start slot in mLocals).
    #[inline]
    pub const fn op_local(frame_offset: LIntPtr) -> Self {
        Self {
            mType: OpType::OP_LOCAL,
            mValue: frame_offset,
        }
    }

    /// Closure capture by slot index (OP_CAPTURE, capture index).
    #[inline]
    pub const fn op_capture(capture_idx: LIntPtr) -> Self {
        Self {
            mType: OpType::OP_CAPTURE,
            mValue: capture_idx,
        }
    }
}

#[repr(C)]
pub(crate) struct Instruction {
    pub mOPCode: CmdType,
    pub mOPLeft: OpCommand,
    pub mOPRight: OpCommand,
    pub mCache: *mut InlineCache,
    pub mNext: LIntPtr,
}

#[repr(C)]
pub(crate) struct CommandTable {
    pub mBegin: *mut Instruction,
    pub mEnd: *mut Instruction,
}

#[repr(C)]
pub(crate) struct StackFrame {
    pub mPC: *mut Instruction,
    pub mLValSize: LInt,
    pub mLoopSize: LInt,
    pub mResultNum: LInt,
    pub mContext: *mut CommandTable,
    pub mClass: BoyiaValue,
}

#[repr(C)]
pub(crate) struct VMCpu {
    pub mReg0: BoyiaValue,
    pub mReg1: BoyiaValue,
}

#[repr(C)]
pub(crate) struct VMCode {
    pub mCode: *mut Instruction,
    pub mSize: LInt,
}

#[repr(C)]
pub(crate) struct VMStrTable {
    pub mTable: [BoyiaStr; CONST_CAPACITY],
    pub mSize: LInt,
}

#[repr(C)]
pub(crate) struct VMEntryTable {
    pub mTable: [LInt; ENTRY_CAPACITY],
    pub mSize: LInt,
}

#[repr(C)]
pub(crate) struct MicroTask {
    pub mResult: BoyiaValue,
    pub mObjRef: BoyiaValue,
    pub mAsyncEs: *mut ExecState,
    pub mNext: *mut MicroTask,
    pub mAllocLink: MicroTaskLink,
}

#[repr(C)]
pub(crate) struct MicroTaskLink {
    pub mLinkNext: *mut MicroTask,
    pub mLinkPrev: *mut MicroTask,
}

#[repr(C)]
pub(crate) struct MicroTaskQueue {
    pub mUsedTasks: MicroTaskList,
    pub mAllocTasks: MicroTaskList,
    pub mTaskCache: *mut LVoid, // MemoryCache pointer
}

#[repr(C)]
pub(crate) struct MicroTaskList {
    pub mHead: *mut MicroTask,
    pub mEnd: *mut MicroTask,
}

#[repr(C)]
pub(crate) struct ExecState {
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

// Main VM structure. mCreator is *mut dyn Runtime (fat pointer). Not exposed in public API.
#[repr(C)]
pub(crate) struct BoyiaVM {
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
    pub mEStateCache: *mut LVoid,
    pub mVMCode: *mut VMCode,
    pub mStrTable: *mut VMStrTable,
    pub mEntry: *mut VMEntryTable,
    pub mHandlers: *mut OPHandler,
    pub mTaskQueue: *mut MicroTaskQueue,
    pub mFunSize: LInt,
    pub mCreator: *mut dyn Runtime,
}

// Native function pointer type (return OpHandleResult for VM dispatch).
pub type NativePtr = unsafe fn(*mut LVoid) -> OpHandleResult;

#[repr(C)]
pub struct NativeFunction {
    pub mNameKey: LUintPtr,
    pub mAddr: NativePtr,
}
