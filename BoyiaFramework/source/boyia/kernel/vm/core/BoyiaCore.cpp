/*
* Date: 2011-11-07
* Modify: 2019-5-3
* Author: yanbo
* Description: Boyia interpreter
* Copyright (c) reserved
* As a independent module for BoyiaUI Engine
*/
#include "BoyiaCore.h"
#include "BoyiaError.h"
#include "BoyiaMemory.h"
#include "SalLog.h"
#include "SystemUtil.h"

#define SntxErrorBuild(error, cs) SntxError(error, cs->mLineNum)

#ifndef kBoyiaNull
#define kBoyiaNull 0
#endif

#define kInvalidInstruction (-1)
#define RuntimeError(key, error, vm) PrintErrorKey(key, error, GetCodeRow(vm), vm)

/* Type Define Begin */
#define NUM_FUNC ((LInt)1024)
#define NUM_FUNC_PARAMS ((LInt)32)
#define NUM_GLOBAL_VARS ((LInt)512)
#define NUM_LOCAL_VARS ((LInt)512)
#define NUM_RESULT ((LInt)128)
#define FUNC_CALLS ((LInt)32)
#define NUM_PARAMS ((LInt)32)
#define LOOP_NEST ((LInt)32)
#define CODE_CAPACITY ((LInt)1024 * 32) // Instruction Capacity
#define CONST_CAPACITY ((LInt)1024)
#define ENTRY_CAPACITY ((LInt)1024)
#define MICRO_TASK_CAPACITY ((LInt)1024)
#define EXEC_STATE_CAPACITY ((LInt)64)

#define STR2_INT(str) Str2Int(str.mPtr, str.mLen, 10)

extern LInt Str2Int(LInt8* ptr, LInt len, LInt radix);
extern LVoid GCAppendRef(LVoid* address, LUint8 type, LVoid* vm);

enum TokenType {
    DELIMITER = 1,
    IDENTIFIER,
    NUMBER,
    KEYWORD,
    TEMP,
    VARIABLE,
    STRING_VALUE,
};

enum TokenLogicValue {
    AND = BY_END + 1,
    OR,
    NOT,
    LT,
    LE,
    GT,
    GE,
    EQ,
    NE
}; // 26

// "+-*/%^=;,'.:()[]{}"
enum TokenMathValue {
    ADD = NE + 1,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    ASSIGN
}; // 33
// 标点符号
enum TokenDelimiValue {
    SEMI = ASSIGN + 1, // 分号
    COMMA, // 逗号
    QUOTE, // 引号
    DOT,   // 点
    COLON  // 冒号
}; // 38
// 小括号，中括号，大括号
enum TokenBracketValue {
    LPTR = COLON + 1,
    RPTR,
    ARRAY_BEGIN,
    ARRAY_END,
    BLOCK_START,
    BLOCK_END
};
struct BoyiaVM;
struct Instruction;
// 操作指令处理器
typedef LInt (*OPHandler)(Instruction* instruction, BoyiaVM* vm);

// 操作类型
enum OpType {
    OP_NONE = 0,
    OP_CONST_NUMBER,
    OP_CONST_STRING,
    OP_REG0,
    OP_REG1,
    OP_VAR,
};

// 指令类型
enum CmdType {
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
    kCmdTmpLocal,
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
    kCmdEnd
};

typedef struct {
    LUint8 mType;
    LIntPtr mValue;
} OpCommand;

static OpCommand COMMAND_R0 = { OP_REG0, 0 };
static OpCommand COMMAND_R1 = { OP_REG1, 0 };

enum OpInstType {
    OpLeft,
    OpRight,
};

typedef struct Instruction {
    LUint8 mOPCode;
    OpCommand mOPLeft;
    OpCommand mOPRight;
    InlineCache* mCache;
    LIntPtr mNext;
} Instruction;

typedef struct {
    Instruction* mBegin;
    Instruction* mEnd;
} CommandTable;

struct KeywordPair {
    BoyiaStr mName; /* keyword lookup table */
    LUint8 mType;
} gKeywordTable[] = {
    /* Commands must be entered lowercase */
    { D_STR("if", 2), BY_IF }, /* in this table. */
    { D_STR("elif", 4), BY_ELIF },
    { D_STR("else", 4), BY_ELSE },
    { D_STR("do", 2), BY_DO },
    { D_STR("while", 5), BY_WHILE },
    { D_STR("for", 3), BY_FOR },
    { D_STR("break", 5), BY_BREAK },
    { D_STR("fun", 3), BY_FUNC },
    { D_STR("async", 5), BY_ASYNC },
    { D_STR("await", 5), BY_AWAIT },
    { D_STR("class", 5), BY_CLASS },
    { D_STR("extends", 7), BY_EXTEND },
    { D_STR("prop", 4), BY_PROP },
    { D_STR("var", 3), BY_VAR },
    { D_STR("return", 6), BY_RETURN },
    { D_STR("", 0), 0 } /* mark end of table */
};

/* ----------------------------------------------------------- */
/* struct Token;                                               */
/* 成员说明：                                                   */
/* mTokenName:    此变量是存放单词名字                           */
/* mTokenType:    此变量是存放Token的类型                        */
/* mTokenValue:   此变量是存放Token的值                          */
/* ------------------------------------------------------------ */
typedef struct {
    BoyiaStr mTokenName;
    LUint8 mTokenType; /* contains type of token*/
    LUint8 mTokenValue; /* internal representation of token */
} BoyiaToken;

// 栈帧
typedef struct {
    Instruction* mPC; // pc, 指令计数器
    LInt mLValSize;  // 局部变量个数
    LInt mTmpLValSize; // 函数调用前, 保存当前栈帧局部变量个数, 保存执行现场后，使用tmp设置上一个栈帧的变量个数
    LInt mLoopSize; // loop层次
    CommandTable* mContext; // 函数指令集
    BoyiaValue mClass; // 函数所属对象
    LInt mResultNum;
} StackFrame;

// 虚拟寄存器模型，其中每个寄存器可以表示为4个32寄存器
// Reg的mNameKey没有任何意义，
// 但是在处理变量时变得有意义，即指向变量地址，
typedef struct {
    BoyiaValue mReg0; // result register, 结果寄存器
    BoyiaValue mReg1; // help register, 辅助运算寄存器
} VMCpu;

typedef struct {
    Instruction* mCode;
    LInt mSize;
} VMCode;

typedef struct {
    BoyiaStr mTable[CONST_CAPACITY];
    LInt mSize;
} VMStrTable;

typedef struct {
    LInt mTable[ENTRY_CAPACITY];
    LInt mSize;
} VMEntryTable;

struct ExecState;
typedef struct MicroTask {
    BoyiaValue mResult; // 微任务执行完后回调
    BoyiaValue mObjRef; // 微任务对象引用
    ExecState* mAsyncEs; // 保存当前状态
    MicroTask* mNext;
    struct {
        MicroTask* mLinkNext;
        MicroTask* mLinkPrev;
    } mAllocLink;
} MicroTask;

typedef struct {
    // 微任务队列，微任务获取到结果后，执行resolve会加入到这个队列
    struct {
        MicroTask* mHead;
        MicroTask* mEnd;
    } mUsedTasks;
    // 管理分配的微任务，方便gc时进行遍历
    struct {
        MicroTask* mHead;
        MicroTask* mEnd;
    } mAllocTasks;

    LVoid* mTaskCache;
} MicroTaskQueue;

typedef struct ExecState {
    StackFrame mStackFrame;
    LInt mFrameIndex;
    
    BoyiaValue mLocals[NUM_LOCAL_VARS];
    BoyiaValue mOpStack[NUM_RESULT];
    LIntPtr mLoopStack[LOOP_NEST];
    BoyiaValue mFun;

    MicroTask* mTopTask;
    // 保存上一个执行状态指针用于回溯
    ExecState* mPrevious;

    // mPrev与mNext单纯只是对创建的ExecState进行记录
    // 用于GC时使用
    ExecState* mPrev;
    ExecState* mNext;
    StackFrame mExecStack[FUNC_CALLS];
    LBool mWait;
} ExecState;

/* Boyia VM Define
 * Member
 * 1, mPool
 * 2, Function Area
 * 3, gBoyiaVM->mGlobals
 */
typedef struct BoyiaVM {
    BoyiaFunction* mFunTable;
    BoyiaValue* mGlobals;
    LInt mGValSize; /* count of global variable stack */

    // Use ExecState begin
    BoyiaValue* mOpStack; // 指令运算压栈
    LIntPtr* mLoopStack;
    BoyiaValue* mLocals;
    StackFrame* mExecStack;
    // Use ExecState end

    VMCpu* mCpu;
    // mESLink是以最新创建的ExecState为head的双向链表
    // 只要在GC时使用
    ExecState* mESLink;
    ExecState* mEState;
    LVoid* mEStateCache;
    
    VMCode* mVMCode;
    VMStrTable* mStrTable;
    VMEntryTable* mEntry;
    OPHandler* mHandlers;
    MicroTaskQueue* mTaskQueue;
    
    LInt mFunSize; /* index to top of function call stack */
    LVoid* mCreator; // 此处传入创建vm的外部对象
} BoyiaVM;

typedef struct {
    LInt8* mProg;
    LInt mLineNum; // Current Line num of source code
    LInt mColumnNum; // Current Column num of source code
    BoyiaToken mToken;
    BoyiaVM* mVm;
    CommandTable* mCmds;
} CompileState;

/* Global value define end */
static LVoid LocalStatement(CompileState* cs);

static LVoid IfStatement(CompileState* cs);

static LVoid PushArgStatement(CompileState* cs);

static LVoid WhileStatement(CompileState* cs);

static LVoid DoStatement(CompileState* cs);

static LVoid BlockStatement(CompileState* cs);

static LVoid FunStatement(CompileState* cs, LInt funType);

static LInt NextToken(CompileState* cs);

static LVoid EvalExpression(CompileState* cs);

static LVoid EvalAssignment(CompileState* cs);

static LVoid Atom(CompileState* cs);

static BoyiaValue* FindVal(LUintPtr key, BoyiaVM* vm);

static BoyiaValue* FindObjProp(BoyiaValue* lVal, LUintPtr rVal, Instruction* inst, BoyiaVM* vm);

// Handler Declarations Begin
static LInt HandleAssignment(Instruction* inst, BoyiaVM* vm);

static LInt HandleJumpToIfTrue(Instruction* inst, BoyiaVM* vm);

static LInt HandleIfEnd(Instruction* inst, BoyiaVM* vm);

static LInt HandlePushObj(Instruction* inst, BoyiaVM* vm);

static LInt HandleAdd(Instruction* inst, BoyiaVM* vm);

static LInt HandleSub(Instruction* inst, BoyiaVM* vm);

static LInt HandleMul(Instruction* inst, BoyiaVM* vm);

static LInt HandleDiv(Instruction* inst, BoyiaVM* vm);

static LInt HandleMod(Instruction* inst, BoyiaVM* vm);

static LInt HandleLogic(Instruction* inst, BoyiaVM* vm);

static LInt HandleRelational(Instruction* inst, BoyiaVM* vm);

static LInt HandlePush(Instruction* inst, BoyiaVM* vm);

static LInt HandlePop(Instruction* inst, BoyiaVM* vm);

static LInt HandlePushScene(Instruction* inst, BoyiaVM* vm);

static LInt HandlePopScene(Instruction* inst, BoyiaVM* vm);

static LInt HandlePushArg(Instruction* inst, BoyiaVM* vm);

static LInt HandleTempLocalSize(Instruction* inst, BoyiaVM* vm);

static LInt HandlePushParams(Instruction* inst, BoyiaVM* vm);

static LInt HandleCallFunction(Instruction* inst, BoyiaVM* vm);

static LInt HandleCallInternal(Instruction* inst, BoyiaVM* vm);

static LInt HandleLoopIfTrue(Instruction* inst, BoyiaVM* vm);

static LInt HandleJumpTo(Instruction* inst, BoyiaVM* vm);

static LInt HandleGetProp(Instruction* inst, BoyiaVM* vm);

static LInt HandleConstString(Instruction* inst, BoyiaVM* vm);

static LInt HandleAssignment(Instruction* inst, BoyiaVM* vm);

static LInt HandleAssignVar(Instruction* inst, BoyiaVM* vm);

static LInt HandleLoopBegin(Instruction* inst, BoyiaVM* vm);

static LInt HandleCreateClass(Instruction* inst, BoyiaVM* vm);

static LInt HandleExtend(Instruction* inst, BoyiaVM* vm);

static LInt HandleDeclGlobal(Instruction* inst, BoyiaVM* vm);

static LInt HandleDeclLocal(Instruction* inst, BoyiaVM* vm);

static LInt HandleFunCreate(Instruction* inst, BoyiaVM* vm);

static LInt HandleCreateExecutor(Instruction* inst, BoyiaVM* vm);

static LInt HandleCreateParam(Instruction* inst, BoyiaVM* vm);

static LInt HandleReturn(Instruction* inst, BoyiaVM* vm);

static LInt HandleBreak(Instruction* inst, BoyiaVM* vm);

static LInt HandleCreateProp(Instruction* inst, BoyiaVM* vm);

static LInt HandleCreateMap(Instruction* inst, BoyiaVM* vm);

static LInt HandleSetMapKey(Instruction* inst, BoyiaVM* vm);

static LInt HandleSetMapValue(Instruction* inst, BoyiaVM* vm);

static LInt HandleCreateArray(Instruction* inst, BoyiaVM* vm);

static LInt HandleAddArrayItem(Instruction* inst, BoyiaVM* vm);

static LInt HandleAwait(Instruction* inst, BoyiaVM* vm);

static LInt HandleSetAnonym(Instruction* inst, BoyiaVM* vm);

static LInt HandleOnceJmpTrue(Instruction* inst, BoyiaVM* vm);
// Handler Declarations End

// Eval Begin
static LVoid EvalAwait(CompileState* cs);
// Eval End

static LVoid ValueCopyNoName(BoyiaValue* dest, BoyiaValue* src);

static LBool IsObjectPropFunc(LUint8 type) {
    return type == BY_PROP_FUNC 
        || type == BY_ASYNC_PROP
        || type == BY_NAV_PROP
        || type == BY_ANONYM_FUNC;
}

static LVoid AssignStateClass(ExecState* state, BoyiaValue* value) {
    if (value) {
        ValueCopyNoName(&state->mStackFrame.mClass, value);
    } else {
        state->mStackFrame.mClass.mValue.mObj.mPtr = kBoyiaNull;
        state->mStackFrame.mClass.mValue.mObj.mSuper = kBoyiaNull;
    }
    
    state->mStackFrame.mClass.mValueType = BY_CLASS;
}

static LInt GetCodeRow(BoyiaVM* vmPtr) {
    LInt codeIndex = (LInt)(vmPtr->mEState->mStackFrame.mPC - vmPtr->mVMCode->mCode);
    BoyiaCodePosition* position = GetCodePosition(codeIndex, vmPtr);
    if (position) {
        return position->mRow;
    }

    return -1;
}

// Reset scene of global execute state
static LVoid ResetScene(ExecState* state) {
    state->mStackFrame.mLValSize = 0; /* Initialize local variable stack index */
    state->mStackFrame.mLoopSize = 0;
    state->mStackFrame.mResultNum = 0;
    state->mStackFrame.mTmpLValSize = 0;
    state->mStackFrame.mContext = kBoyiaNull;
    state->mStackFrame.mPC = kBoyiaNull;
    state->mPrevious = kBoyiaNull;
    state->mTopTask = kBoyiaNull;

    state->mFrameIndex = 0; /* Initialize the call stack index */

    state->mFun.mValue.mObj.mPtr = kBoyiaNull;
    state->mFun.mValueType = BY_ARG;
    state->mWait = LFalse;

    AssignStateClass(state, kBoyiaNull);
}

static VMCode* CreateVMCode() {
    VMCode* code = FAST_NEW(VMCode);
    code->mCode = FAST_NEW_ARRAY(Instruction, CODE_CAPACITY); //new Instruction[CODE_CAPACITY];//
    code->mSize = 0;
    return code;
}

static OPHandler* InitHandlers() {
    OPHandler* handlers = FAST_NEW_ARRAY(OPHandler, kCmdEnd);
    LMemset(handlers, 0, sizeof(OPHandler) * kCmdEnd);
    handlers[kCmdJmpTrue] = HandleJumpToIfTrue;
    handlers[kCmdIfEnd] = HandleIfEnd;
    handlers[kCmdElse] = kBoyiaNull;
    handlers[kCmdElEnd] = kBoyiaNull;
    handlers[kCmdPushObj] = HandlePushObj;
    handlers[kCmdAdd] = HandleAdd;
    handlers[kCmdSub] = HandleSub;
    handlers[kCmdMul] = HandleMul;
    handlers[kCmdDiv] = HandleDiv;
    handlers[kCmdMod] = HandleMod;
    handlers[kCmdAndLogic] = HandleLogic;
    handlers[kCmdOrLogic] = HandleLogic;
    handlers[kCmdGtRelation] = HandleRelational;
    handlers[kCmdLtRelation] = HandleRelational;
    handlers[kCmdGeRelation] = HandleRelational;
    handlers[kCmdLeRelation] = HandleRelational;
    handlers[kCmdEqRelation] = HandleRelational;
    handlers[kCmdNeRelation] = HandleRelational;
    handlers[kCmdNotRelation] = HandleRelational;
    handlers[kCmdPush] = HandlePush;
    handlers[kCmdPop] = HandlePop;
    handlers[kCmdPushScene] = HandlePushScene;
    handlers[kCmdPopScene] = HandlePopScene;
    handlers[kCmdPushArg] = HandlePushArg;
    handlers[kCmdTmpLocal] = HandleTempLocalSize;
    handlers[kCmdPushParams] = HandlePushParams;
    handlers[kCmdCallFunction] = HandleCallFunction;
    handlers[kCmdCallNative] = HandleCallInternal;
    handlers[kCmdLoopTrue] = HandleLoopIfTrue;
    handlers[kCmdJmpTo] = HandleJumpTo;
    handlers[kCmdGetProp] = HandleGetProp;
    handlers[kCmdConstStr] = HandleConstString;
    handlers[kCmdAssign] = HandleAssignment;
    handlers[kCmdAssignVar] = HandleAssignVar;
    handlers[kCmdLoop] = HandleLoopBegin;
    handlers[kCmdCreateClass] = HandleCreateClass;
    handlers[kCmdClassExtend] = HandleExtend;
    handlers[kCmdDeclGlobal] = HandleDeclGlobal;
    handlers[kCmdDeclLocal] = HandleDeclLocal;
    handlers[kCmdCreateFunction] = HandleFunCreate;
    handlers[kCmdExecCreate] = HandleCreateExecutor;
    handlers[kCmdParamCreate] = HandleCreateParam;
    handlers[kCmdReturn] = HandleReturn;
    handlers[kCmdBreak] = HandleBreak;
    handlers[kCmdPropCreate] = HandleCreateProp;
    handlers[kCmdCreateMap] = HandleCreateMap;
    handlers[kCmdSetMapKey] = HandleSetMapKey;
    handlers[kCmdSetMapValue] = HandleSetMapValue;
    handlers[kCmdCreateArray] = HandleCreateArray;
    handlers[kCmdAddArrayItem] = HandleAddArrayItem;
    handlers[kCmdAwait] = HandleAwait;
    handlers[kCmdSetAnonym] = HandleSetAnonym;
    handlers[kCmdOnceJmpTrue] = HandleOnceJmpTrue;

    return handlers;
}

static VMStrTable* CreateVMStringTable() {
    VMStrTable* table = FAST_NEW(VMStrTable);
    table->mSize = 0;
    return table;
}

static VMEntryTable* CreateVMEntryTable() {
    VMEntryTable* table = FAST_NEW(VMEntryTable);
    table->mSize = 0;
    return table;
}

static VMCpu* CreateVMCpu() {
    VMCpu* vmCpu = FAST_NEW(VMCpu);
    vmCpu->mReg0.mValueType = BY_ARG;
    vmCpu->mReg0.mValue.mIntVal = 0;

    vmCpu->mReg1.mValueType = BY_ARG;
    vmCpu->mReg1.mValue.mIntVal = 0;
    return vmCpu;
}

static MicroTask* AllocMicroTask(BoyiaVM* vm) {
    MicroTaskQueue* queue = vm->mTaskQueue;
    MicroTask* task = ALLOC_CHUNK(MicroTask, queue->mTaskCache);
    if (!task) {
        return kBoyiaNull;
    }

    task->mResult.mValueType = BY_ARG;
    task->mObjRef.mValueType = BY_ARG;
    task->mNext = kBoyiaNull;
    task->mAllocLink.mLinkNext = kBoyiaNull;

    if (queue->mAllocTasks.mHead) {
        task->mAllocLink.mLinkPrev = queue->mAllocTasks.mEnd;
    } else {
        queue->mAllocTasks.mHead = task;
        task->mAllocLink.mLinkPrev = kBoyiaNull;
    }

    queue->mAllocTasks.mEnd = task;

    return task;
}

static LVoid FreeMicroTask(MicroTask* task, BoyiaVM* vm) {
    MicroTaskQueue* queue = vm->mTaskQueue;
    task->mResult.mValueType = BY_ARG;

    MicroTask* prev = task->mAllocLink.mLinkPrev;
    MicroTask* next = task->mAllocLink.mLinkNext;

    if (next) {
        next->mAllocLink.mLinkPrev = prev;
    } else {
        queue->mAllocTasks.mEnd = prev;
    }

    if (prev) {
        prev->mAllocLink.mLinkNext = next;
    } else {
        queue->mAllocTasks.mHead = next;
    }
    
    FREE_CHUNK(task, queue->mTaskCache);
}

static MicroTaskQueue* CreateTaskQueue() {
    MicroTaskQueue* queue = FAST_NEW(MicroTaskQueue);
    queue->mUsedTasks.mHead = kBoyiaNull;
    queue->mUsedTasks.mEnd = kBoyiaNull;

    queue->mAllocTasks.mHead = kBoyiaNull;
    queue->mAllocTasks.mEnd = kBoyiaNull;

    queue->mTaskCache = CREATE_MEMCACHE(MicroTask, MICRO_TASK_CAPACITY);
    return queue;
}

LVoid* IterateMicroTask(BoyiaValue** obj, BoyiaValue** result, LVoid* vmPtr, LVoid* ptr) {
    *result = kBoyiaNull;
    *obj = kBoyiaNull;
    if (!ptr) {
        return ptr;
    }

    MicroTask* task = kBoyiaNull;
    if (vmPtr == ptr) {
        BoyiaVM* vm = (BoyiaVM*)vmPtr;
        task = vm->mTaskQueue->mAllocTasks.mHead;
    } else {
        task = (MicroTask*)ptr;
    }

    if (!task) {
        return task;
    }

    *result = &task->mResult;
    *obj = &task->mObjRef;
    return task->mAllocLink.mLinkNext;
}

static LVoid AddMicroTask(MicroTask* task, BoyiaVM* vm) {
    MicroTaskQueue* queue = vm->mTaskQueue;
    if (queue->mUsedTasks.mHead) {
        queue->mUsedTasks.mEnd->mNext = task;
        queue->mUsedTasks.mEnd = task;
    } else {
        queue->mUsedTasks.mHead = task;
        queue->mUsedTasks.mEnd = task;
    }     
}

static LVoid* CreateExecStateCache() {
    return CREATE_MEMCACHE(ExecState, EXEC_STATE_CAPACITY);
}

static ExecState* AllocExecState(BoyiaVM* vm) {
    return ALLOC_CHUNK(ExecState, vm->mEStateCache);
}

static LVoid FreeExecState(ExecState* state, BoyiaVM* vm) {
    FREE_CHUNK(state, vm->mEStateCache);
}

static ExecState* CreateExecState(BoyiaVM* vm) {
    ExecState* state = AllocExecState(vm);
    ResetScene(state);
    if (vm->mESLink) {
        vm->mESLink->mNext = state;
    }
    
    state->mNext = kBoyiaNull;
    state->mPrev = vm->mESLink;
    vm->mESLink = state;
    return state;
}

static LVoid DestroyExecState(ExecState* state, BoyiaVM* vm) {
    if (state->mNext) {
        state->mNext->mPrev = state->mPrev;
        state->mPrev->mNext = state->mNext;
    } else {
        state->mPrev->mNext = kBoyiaNull;
        vm->mESLink = state->mPrev;
    }

    state->mTopTask = kBoyiaNull;
    state->mPrevious = kBoyiaNull;
    FreeExecState(state, vm);
}

static LVoid SwitchExecState(ExecState* execState, BoyiaVM* vm) {
    vm->mEState = execState;
    vm->mLocals = execState->mLocals;
    vm->mOpStack = execState->mOpStack;
    vm->mLoopStack = execState->mLoopStack;
    vm->mExecStack = execState->mExecStack;
}

LVoid* InitVM(LVoid* creator) {
    BoyiaVM* vm = FAST_NEW(BoyiaVM);
    vm->mCreator = creator;
    vm->mESLink = kBoyiaNull;
    /* 一个页面只允许最多NUM_GLOBAL_VARS个函数 */
    vm->mGlobals = FAST_NEW_ARRAY(BoyiaValue, NUM_GLOBAL_VARS);
    vm->mFunTable = FAST_NEW_ARRAY(BoyiaFunction, NUM_FUNC);
    vm->mEStateCache = CreateExecStateCache();
    ExecState* execState = CreateExecState(vm);
    SwitchExecState(execState, vm);

    vm->mCpu = CreateVMCpu();
    vm->mVMCode = CreateVMCode();
    vm->mHandlers = InitHandlers();
    vm->mStrTable = CreateVMStringTable();
    vm->mEntry = CreateVMEntryTable();
    vm->mTaskQueue = CreateTaskQueue();

    vm->mGValSize = 0;
    vm->mFunSize = 0;

    ResetScene(vm->mEState);
    return vm;
}

LVoid* GetVMCreator(LVoid* vm) {
    return ((BoyiaVM*)vm)->mCreator;
}

LVoid DestroyVM(LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    FAST_DELETE(vmPtr->mGlobals);
    FAST_DELETE(vmPtr->mCpu);
    FAST_DELETE(vmPtr->mEntry);
    FAST_DELETE(vmPtr->mEState);
    FAST_DELETE(vmPtr->mFunTable);
    FAST_DELETE(vmPtr->mHandlers);
    FAST_DELETE(vmPtr->mStrTable);
    FAST_DELETE(vmPtr->mVMCode);
    
    FAST_DELETE(vm);
}

static Instruction* AllocateInstruction(BoyiaVM* vm) {
    BOYIA_LOG("AllocateInstruction size=%d", vm->mVMCode->mSize);
    return vm->mVMCode->mCode + vm->mVMCode->mSize++;
}

static Instruction* PutInstruction(
    OpCommand* left,
    OpCommand* right,
    LUint8 op,
    CompileState* cs) {
    Instruction* newIns = AllocateInstruction(cs->mVm);
    // Init member
    newIns->mOPLeft.mType = OP_NONE;
    newIns->mOPLeft.mValue = 0;
    newIns->mOPRight.mType = OP_NONE;
    newIns->mOPRight.mValue = 0;

    if (left) {
        newIns->mOPLeft.mType = left->mType;
        newIns->mOPLeft.mValue = left->mValue;
    }

    if (right) {
        newIns->mOPRight.mType = right->mType;
        newIns->mOPRight.mValue = right->mValue;
    }
    
    newIns->mOPCode = op;
    newIns->mNext = kInvalidInstruction;
    newIns->mCache = kBoyiaNull;
    CommandTable* cmds = cs->mCmds;
    Instruction* inst = cmds->mEnd;
    if (!inst) {
        cmds->mBegin = newIns;
    } else {
        inst->mNext = cs->mVm->mVMCode->mSize - 1;
    }

    cmds->mEnd = newIns;
    SetCodePosition(
        cs->mVm->mVMCode->mSize - 1, 
        cs->mLineNum, 
        cs->mColumnNum, 
        cs->mVm);
    return newIns;
}

static Instruction* NextInstruction(Instruction* instruction, BoyiaVM* vm) {
    if (instruction->mNext == kInvalidInstruction) {
        return kBoyiaNull;
    }

    return vm->mVMCode->mCode + instruction->mNext;
}

static LVoid ExecPopFunction(BoyiaVM* vm) {
    if (vm->mEState->mStackFrame.mPC) {
        return;
    }

    // 指令为空，则判断是否处于函数范围中，是则pop，
    // 从而取得调用之前的运行环境, 即之前指向的pc指令
    // 如果不为空，就获取下一条指令，递归调用本函数
    if (vm->mEState->mFrameIndex <= 0
        && vm->mEState->mPrevious
        && !vm->mEState->mPrevious->mWait) {
        ExecState* currentState = vm->mEState;
        SwitchExecState(currentState->mPrevious, vm);
        if (!currentState->mWait) {
            if (currentState->mTopTask) {
                FreeMicroTask(currentState->mTopTask, vm);
            }
            DestroyExecState(currentState, vm);
        }
    }

    // SwitchExecState后mFrameIndex可能不为0
    if (vm->mEState->mFrameIndex > 0) {
        HandlePopScene(kBoyiaNull, vm);
        if (vm->mEState->mStackFrame.mPC) {
            vm->mEState->mStackFrame.mPC = NextInstruction(vm->mEState->mStackFrame.mPC, vm); // vm->mEState->mPC->mNext;
            ExecPopFunction(vm);
        } else if (vm->mEState->mPrevious && !vm->mEState->mPrevious->mWait) {
            ExecPopFunction(vm);
        }
    }
}

static LVoid ExecInstruction(BoyiaVM* vm) {
    // 通过指令寄存器进行计算
    if (!vm->mEState->mStackFrame.mPC) {
        ExecPopFunction(vm);
        return;
    }

    while (vm->mEState->mStackFrame.mPC) {
        OPHandler handler = vm->mHandlers[vm->mEState->mStackFrame.mPC->mOPCode];
        if (handler) {
            LInt result = handler(vm->mEState->mStackFrame.mPC, vm);
            if (result == kOpResultEnd) { // 指令运行出错跳出循环
                break;
            } else if (vm->mEState->mStackFrame.mPC && result == kOpResultJumpFun) { // 函数跳转
                continue;
            } // 指令计算结果为1即为正常情况
        }

        if (vm->mEState->mStackFrame.mPC) {
            vm->mEState->mStackFrame.mPC = NextInstruction(vm->mEState->mStackFrame.mPC, vm);
        }

        ExecPopFunction(vm);
    }
}

static LVoid NewLine(CompileState* cs) {
    ++cs->mLineNum;
    cs->mColumnNum = 0;
}

static LVoid AddColumn(CompileState* cs, LInt number) {
    cs->mProg += number;
    cs->mLineNum += number;
}

static LVoid Putback(CompileState* cs) {
    //cs->mProg -= cs->mToken.mTokenName.mLen;
    AddColumn(cs, -cs->mToken.mTokenName.mLen);
}

static LUint8 LookUp(BoyiaStr* name) {
    LInt i = 0;
    for (; gKeywordTable[i].mName.mLen; ++i) {
        if (MStrcmp(&gKeywordTable[i].mName, name))
            return gKeywordTable[i].mType;
    }

    return 0;
}

static BoyiaFunction* CopyFunction(BoyiaValue* clsVal, LInt count, BoyiaVM* vm) {
    BoyiaFunction* newFunc = NEW(BoyiaFunction, vm);
    // copy function
    BOYIA_LOG("HandleCallInternal CreateObject %d", 5);

    BoyiaFunction* func = (BoyiaFunction*)clsVal->mValue.mObj.mPtr;
    BOYIA_LOG("HandleCallInternal CreateObject %d", 6);
    newFunc->mParams = NEW_ARRAY(BoyiaValue, count, vm);
    newFunc->mParamSize = 0;
    newFunc->mFuncBody = func->mFuncBody;
    newFunc->mParamCount = count;

    // 不拷贝方法，只拷贝属性
    while (clsVal) {
        BoyiaFunction* func = (BoyiaFunction*)clsVal->mValue.mObj.mPtr;
        LInt idx = func->mParamSize;
        while (idx--) {
            LUint8 type = func->mParams[idx].mValueType;
            if (type == BY_FUNC || type == BY_NAV_FUNC) {
                continue;
            }
            
            BoyiaValue* prop = newFunc->mParams + newFunc->mParamSize++;
            ValueCopy(prop, func->mParams + idx);
            if (IsObjectPropFunc(type)) {
                prop->mValue.mObj.mSuper = (LIntPtr)newFunc;
            }
        }

        clsVal = (BoyiaValue*)clsVal->mValue.mObj.mSuper;
    }

    return newFunc;
}

LInt CreateObject(LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    BOYIA_LOG("HandleCallInternal CreateObject %d", 1);
    BoyiaValue* value = (BoyiaValue*)GetLocalValue(0, vm);
    if (!value || value->mValueType != BY_CLASS) {
        return kOpResultEnd;
    }

    BOYIA_LOG("HandleCallInternal CreateObject %d", 2);
    // 获取CLASS的内部实现
    BOYIA_LOG("HandleCallInternal CreateObject %d", 3);
    // 指针引用R0
    BoyiaValue* result = &vmPtr->mCpu->mReg0;
    // 设置result的值
    ValueCopy(result, value);
    // 拷贝出新的内部实现
    BoyiaFunction* newFunc = CopyFunction(value, NUM_FUNC_PARAMS, vmPtr);
    result->mValue.mObj.mPtr = (LIntPtr)newFunc;
    result->mValue.mObj.mSuper = value->mValue.mObj.mSuper;
    BOYIA_LOG("HandleCallInternal CreateObject %d", 4);

    GCAppendRef(newFunc, BY_CLASS, vm);
    return kOpResultSuccess;
}

LVoid ValueCopyNoName(BoyiaValue* dest, BoyiaValue* src) {
    dest->mValueType = src->mValueType;
    switch (src->mValueType) {
    case BY_INT:
    case BY_CHAR:
    case BY_NAVCLASS:
        dest->mValue.mIntVal = src->mValue.mIntVal;
        break;
    case BY_FUNC:
        dest->mValue.mObj.mPtr = src->mValue.mObj.mPtr;
        break;
    case BY_PROP_FUNC:
    case BY_ASYNC_PROP:
    case BY_NAV_PROP:
    case BY_ANONYM_FUNC:
    case BY_CLASS: {
        dest->mValue.mObj.mPtr = src->mValue.mObj.mPtr;
        dest->mValue.mObj.mSuper = src->mValue.mObj.mSuper;
    } break;
    case BY_STRING:
        MStrcpy(&dest->mValue.mStrVal, &src->mValue.mStrVal);
        break;
    default:
        dest->mValue = src->mValue;
        break;
    }
}

LVoid ValueCopy(BoyiaValue* dest, BoyiaValue* src) {
    dest->mNameKey = src->mNameKey;
    ValueCopyNoName(dest, src);
}

static LInt HandleBreak(Instruction* inst, BoyiaVM* vm) {
    BOYIA_LOG("HandleBreak mLoopSize=%d \n", vm->mEState->mStackFrame.mLoopSize);
    vm->mEState->mStackFrame.mPC = (Instruction*)vm->mLoopStack[--vm->mEState->mStackFrame.mLoopSize];
    return kOpResultSuccess;
}

static LVoid BreakStatement(CompileState* cs) {
    BOYIA_LOG("BreakStatement inst code=%d \n", 1);
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdBreak, cs);
}

static LInt HandleCreateProp(Instruction* inst, BoyiaVM* vm) {
    BoyiaFunction* func = (BoyiaFunction*)vm->mEState->mStackFrame.mClass.mValue.mObj.mPtr;
    func->mParams[func->mParamSize].mNameKey = (LUintPtr)inst->mOPLeft.mValue;
    func->mParams[func->mParamSize].mValue.mIntVal = 0;
    func->mParamSize++;
    return kOpResultSuccess;
}

static LVoid PropStatement(CompileState* cs) {
    NextToken(cs);
    if (cs->mToken.mTokenType == IDENTIFIER) {
        OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };
        PutInstruction(&cmd, kBoyiaNull, kCmdPropCreate, cs);
        Putback(cs);
        EvalExpression(cs);

        if (cs->mToken.mTokenValue != SEMI) {
            SntxErrorBuild(SEMI_EXPECTED, cs);
        }
    } else if (cs->mToken.mTokenType == KEYWORD) {
        // 属性方法，初始化的时候会被拷贝到对象属性中，用于做回调使用
        if (cs->mToken.mTokenValue == BY_FUNC) {
            FunStatement(cs, BY_PROP_FUNC);
        } else if (cs->mToken.mTokenValue = BY_ASYNC) {
            FunStatement(cs, BY_ASYNC_PROP);
        }
    } else {
        SntxErrorBuild(SYNTAX, cs);
    }
}

LVoid LocalPush(BoyiaValue* value, LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    if (vmPtr->mEState->mStackFrame.mLValSize > NUM_LOCAL_VARS) {
        SntxError(TOO_MANY_LVARS, GetCodeRow(vmPtr));
    }

    ValueCopy(vmPtr->mLocals + (vmPtr->mEState->mStackFrame.mLValSize++), value);
}

static BoyiaValue* FindGlobal(LUintPtr key, BoyiaVM* vm) {
    for (LInt idx = 0; idx < vm->mGValSize; ++idx) {
        if (vm->mGlobals[idx].mNameKey == key)
            return &vm->mGlobals[idx];
    }

    return kBoyiaNull;
}

/* Find the value of a variable. */
static BoyiaValue* GetVal(LUintPtr key, BoyiaVM* vm) {
    /* First, see if has obj scope */
    if (key == kBoyiaThis) {
        return &vm->mEState->mStackFrame.mClass;
    }

    if (key == kBoyiaSuper) {
        return (BoyiaValue*)vm->mEState->mStackFrame.mClass.mValue.mObj.mSuper;
    }

    /* second, see if it's a local variable */
    if (vm->mEState->mFrameIndex > 0) {
        LInt start = vm->mExecStack[vm->mEState->mFrameIndex - 1].mLValSize;
        LInt idx = vm->mEState->mStackFrame.mLValSize - 1;
        // idx>localLen而不是idx>=localLen，原因则是，第一个元素实际上是函数变量本身
        for (; idx > start; --idx) {
            if (vm->mLocals[idx].mNameKey == key)
                return &vm->mLocals[idx];
        }
    }

    /* otherwise, try global vars */
    BoyiaValue* val = FindGlobal(key, vm);
    if (val) {
        return val;
    }

    return FindObjProp(&vm->mEState->mStackFrame.mClass, key, kBoyiaNull, vm);
}

static BoyiaValue* FindVal(LUintPtr key, BoyiaVM* vm) {
    BoyiaValue* value = GetVal(key, vm);
    if (!value) {
        RuntimeError(key, NOT_VAR, vm);
    }

    return value;
}

static BoyiaValue* GetOpValue(Instruction* inst, LInt8 type, BoyiaVM* vm) {
    BoyiaValue* val = kBoyiaNull;
    OpCommand* op = type == OpLeft ? &inst->mOPLeft : &inst->mOPRight;
    switch (op->mType) { // 赋值左值不可能是常量
    case OP_REG0:
        val = &vm->mCpu->mReg0;
        break;
    case OP_REG1:
        val = &vm->mCpu->mReg1;
        break;
    case OP_VAR:
        val = FindVal((LUintPtr)op->mValue, vm);
        break;
    }

    return val;
}

static LInt HandleCallInternal(Instruction* inst, BoyiaVM* vm) {
    LInt idx = inst->mOPLeft.mValue;
    BOYIA_LOG("HandleCallInternal Exec idx=%d", idx);
    
    // Result from native function
    return CallNativeFunction(idx, vm);
}

static LVoid SaveTemplLocalSize(BoyiaVM* vm) {
    // mTmpLValSize要存入栈帧中，因为如果存储在vm->mEState的变量中时，函数调用前的实参如果也是函数调用，其popscene时会将这个数据重写掉.
    // 会导致执行该函数的pushscene时出现mLValSize错乱问题，因而当执行该函数时必须将mTmpLValSize也存入栈帧中，保证其调用时的正确性.
    vm->mExecStack[vm->mEState->mFrameIndex].mTmpLValSize = vm->mEState->mStackFrame.mTmpLValSize;
    vm->mEState->mStackFrame.mTmpLValSize = vm->mEState->mStackFrame.mLValSize;
}

static LInt HandleTempLocalSize(Instruction* inst, BoyiaVM* vm) {
    SaveTemplLocalSize(vm);
    return kOpResultSuccess;
}

static LInt HandlePushScene(Instruction* inst, BoyiaVM* vm) {
    if (vm->mEState->mFrameIndex >= FUNC_CALLS) {
        SntxError(NEST_FUNC, GetCodeRow(vm));
        return kOpResultEnd;
    }
    vm->mExecStack[vm->mEState->mFrameIndex].mLValSize = vm->mEState->mStackFrame.mTmpLValSize;
    vm->mExecStack[vm->mEState->mFrameIndex].mPC = inst ? (Instruction*)(inst + inst->mOPLeft.mValue) : kBoyiaNull;
    vm->mExecStack[vm->mEState->mFrameIndex].mContext = vm->mEState->mStackFrame.mContext;
    vm->mExecStack[vm->mEState->mFrameIndex].mResultNum = vm->mEState->mStackFrame.mResultNum;
    vm->mExecStack[vm->mEState->mFrameIndex++].mLoopSize = vm->mEState->mStackFrame.mLoopSize;

    return kOpResultSuccess;
}

static LInt HandlePopScene(Instruction* inst, BoyiaVM* vm) {   
    if (vm->mEState->mFrameIndex > 0) {
        vm->mEState->mStackFrame.mLValSize = vm->mExecStack[--vm->mEState->mFrameIndex].mLValSize;
        vm->mEState->mStackFrame.mPC = vm->mExecStack[vm->mEState->mFrameIndex].mPC;
        vm->mEState->mStackFrame.mContext = vm->mExecStack[vm->mEState->mFrameIndex].mContext;
        vm->mEState->mStackFrame.mLoopSize = vm->mExecStack[vm->mEState->mFrameIndex].mLoopSize;
        vm->mEState->mStackFrame.mTmpLValSize = vm->mExecStack[vm->mEState->mFrameIndex].mTmpLValSize;
        AssignStateClass(vm->mEState, &vm->mExecStack[vm->mEState->mFrameIndex].mClass);
    }

    return kOpResultSuccess;
}

static LInt HandlePushArg(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* value = GetOpValue(inst, OpLeft, vm);
    if (value) {
        LocalPush(value, vm);
        return kOpResultSuccess;
    }

    return kOpResultEnd;
}

static LInt HandlePushObj(Instruction* inst, BoyiaVM* vm) {
    vm->mExecStack[vm->mEState->mFrameIndex].mClass = vm->mEState->mStackFrame.mClass;
    if (inst->mOPLeft.mType == OP_VAR) {
        LUintPtr objKey = (LUintPtr)inst->mOPLeft.mValue;
        if (objKey != kBoyiaSuper) {
            AssignStateClass(vm->mEState, &vm->mCpu->mReg0);
        }
    } else {
        AssignStateClass(vm->mEState, kBoyiaNull);
    }

    return kOpResultSuccess;
}

static LVoid ElseStatement(CompileState* cs) {
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdElse, cs);
    BlockStatement(cs);
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdElEnd, cs);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst); // 最后地址值
}

static LInt HandleReturn(Instruction* inst, BoyiaVM* vm) {
    vm->mEState->mStackFrame.mPC = vm->mEState->mStackFrame.mContext->mEnd;
    return kOpResultSuccess;
}

static LVoid ReturnStatement(CompileState* cs) {
    EvalExpression(cs); // => R0
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdReturn, cs);
}

static LVoid ForStatement(CompileState* cs) {
    // Such as, for (var i=0; i<10; i++)
    NextToken(cs); // '('
    if (cs->mToken.mTokenValue != LPTR) {
        SntxErrorBuild(LPTR_EXPECTED, cs);
    }

    NextToken(cs);
    // First expression
    if (cs->mToken.mTokenValue == BY_VAR) {
        LocalStatement(cs);
    } else {
        Putback(cs);
        EvalExpression(cs);
    }

    // Create loop instruction
    Instruction* beginInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoop, cs);

    // Second expression
    /* check the conditional expression => R0 */
    EvalExpression(cs);

    // If true, execute the block
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoopTrue, cs);

    // Third expression
    EvalExpression(cs);
    if (cs->mToken.mTokenValue != RPTR) {
        SntxErrorBuild(RPTR_EXPECTED, cs);
    }
    // i++ execute finished, then jmp to begin
    Instruction* lastInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdJmpTo, cs);
    lastInst->mOPLeft.mType = OP_CONST_NUMBER;
    lastInst->mOPLeft.mValue = (LIntPtr)(lastInst - beginInst);

    logicInst->mOPLeft.mType = OP_CONST_NUMBER;
    logicInst->mOPLeft.mValue = (LIntPtr)(lastInst - logicInst);

    BlockStatement(cs);
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdJmpTo, cs);
    beginInst->mOPLeft.mType = OP_CONST_NUMBER;
    beginInst->mOPLeft.mValue = (LIntPtr)(endInst - beginInst);

    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst);

    // execute i++
    endInst->mOPLeft.mType = OP_CONST_NUMBER;
    endInst->mOPLeft.mValue = (LIntPtr)(endInst - logicInst);
}

// 解析块{}中的内容
static LVoid BlockStatement(CompileState* cs) {
    LBool block = LFalse;
    do {
        NextToken(cs);
        /* If interpreting single statement, return on first semicolon. */
        /* see what kind of token is up */
        if (cs->mToken.mTokenType == IDENTIFIER) {
            // such as i=a+b; expression
            /* Not a keyword, so process expression. */
            Putback(cs); /* restore token to input stream for further processing by EvalExpression() */
            EvalExpression(cs); /* process the expression */
            if (cs->mToken.mTokenValue != SEMI) {
                SntxErrorBuild(SEMI_EXPECTED, cs);
            }
        } else if (cs->mToken.mTokenValue == BLOCK_START) {
            block = LTrue;
        } else if (cs->mToken.mTokenValue == BLOCK_END) {
            block = LFalse;
            return; /* is a }, so return */
        } else if (cs->mToken.mTokenType == KEYWORD) { /* is keyword */
            switch (cs->mToken.mTokenValue) {
            case BY_VAR:
                // such as var i=a+b;
                LocalStatement(cs);
                break;
            case BY_FUNC:
            case BY_ASYNC:    
                FunStatement(cs, cs->mToken.mTokenValue);
                break;
            case BY_PROP:
                PropStatement(cs);
                break;
            case BY_RETURN: /* return from function call */
                ReturnStatement(cs);
                break;
            case BY_IF: /* process an if statement */
            case BY_ELIF:
                IfStatement(cs);
                break;
            case BY_ELSE: /* process an else statement */
                ElseStatement(cs);
                break;
            case BY_WHILE: /* process a while loop */
                WhileStatement(cs);
                break;
            case BY_DO: /* process a do-while loop */
                DoStatement(cs);
                break;
            case BY_FOR:
                ForStatement(cs);
                break;
            case BY_BREAK:
                BOYIA_LOG("BREAK BreakStatement %d \n", 1);
                BreakStatement(cs);
                break;
            case BY_AWAIT:
                EvalAwait(cs);
                break;
            }
        }
    } while (cs->mToken.mTokenValue != BY_END && block);
}

// 过滤注释
static LVoid SkipComment(CompileState* cs) {
    if (*cs->mProg == '/') {
        if (*(cs->mProg + 1) == '*') { // 多行注释
            //cs->mProg += 2;
            AddColumn(cs, 2);
            do {
                while (*cs->mProg != '*') {
                    if (*cs->mProg == '\n') {
                        NewLine(cs);
                    }
                    AddColumn(cs, 1);
                }
                AddColumn(cs, 1);
            } while (*cs->mProg != '/');
            AddColumn(cs, 1);
            SkipComment(cs);
        } else if (*(cs->mProg + 1) == '/') { //单行注释
            while (*cs->mProg && *cs->mProg != '\n') {
                AddColumn(cs, 1);
            }

            if (*cs->mProg == '\n') {
                NewLine(cs);
                AddColumn(cs, 1);
            }

            SkipComment(cs);
        }
    }
}

// 逻辑运算符
static LInt GetLogicValue(CompileState* cs) {
    if (MStrchr("&|!<>=", *cs->mProg)) {
        LInt len = 0;
        cs->mToken.mTokenName.mPtr = cs->mProg;
        switch (*cs->mProg) {
        case '=':
            if (*(cs->mProg + 1) == '=') {
                //cs->mProg += 2;
                AddColumn(cs, 2);
                len += 2;
                cs->mToken.mTokenValue = EQ;
            }
            break;
        case '!':
            if (*(cs->mProg + 1) == '=') {
                AddColumn(cs, 2);
                len += 2;
                cs->mToken.mTokenValue = NE;
            } else {
                AddColumn(cs, 1);
                len = 1;
                cs->mToken.mTokenValue = NOT;
            }
            break;
        case '<':
            if (*(cs->mProg + 1) == '=') {
                AddColumn(cs, 2);
                len += 2;
                cs->mToken.mTokenValue = LE;
            } else {
                AddColumn(cs, 1);
                len = 1;
                cs->mToken.mTokenValue = LT;
            }
            break;
        case '>':
            if (*(cs->mProg + 1) == '=') {
                AddColumn(cs, 2);
                len += 2;
                cs->mToken.mTokenValue = GE;
            } else {
                AddColumn(cs, 1);
                len = 1;
                cs->mToken.mTokenValue = GT;
            }
            break;
        case '&':
            if (*(cs->mProg + 1) == '&') {
                AddColumn(cs, 2);
                len += 2;
                cs->mToken.mTokenValue = AND;
            }
            break;
        case '|':
            if (*(cs->mProg + 1) == '|') {
                AddColumn(cs, 2);
                len += 2;
                cs->mToken.mTokenValue = OR;
            }
            break;
        }

        if (len) {
            cs->mToken.mTokenName.mLen = len;
            return (cs->mToken.mTokenType = DELIMITER);
        }
    }

    return 0;
}

// 算数运算符
static LInt GetDelimiter(CompileState* cs) {
    const char* delimiConst = "+-*/%^=;,'.:()[]{}";
    LInt op = ADD;
    do {
        if (*delimiConst == *cs->mProg) {
            cs->mToken.mTokenValue = op;
            cs->mToken.mTokenName.mPtr = cs->mProg;
            cs->mToken.mTokenName.mLen = 1;
            AddColumn(cs, 1);
            return (cs->mToken.mTokenType = DELIMITER);
        }
        ++op;
        ++delimiConst;
    } while (*delimiConst);

    return 0;
}

static LInt GetIdentifer(CompileState* cs) {
    LInt len = 0;
    if (LIsAlpha(*cs->mProg)) {
        cs->mToken.mTokenName.mPtr = cs->mProg;
        while (*cs->mProg == '_' || LIsAlpha(*cs->mProg) || LIsDigit(*cs->mProg)) {
            ++len;
            AddColumn(cs, 1);
        }

        cs->mToken.mTokenName.mLen = len;
        cs->mToken.mTokenType = TEMP;
    }

    if (cs->mToken.mTokenType == TEMP) {
        cs->mToken.mTokenValue = LookUp(&cs->mToken.mTokenName);
        if (cs->mToken.mTokenValue) {
            cs->mToken.mTokenType = KEYWORD;
        } else {
            cs->mToken.mTokenType = IDENTIFIER;
        }
    }

    cs->mToken.mTokenName.mLen = len;
    return cs->mToken.mTokenType;
}

static LInt GetStringValue(CompileState* cs) {
    // const string parse
    LInt len = 0;
    if (*cs->mProg == '"') {
        AddColumn(cs, 1);
        cs->mToken.mTokenName.mPtr = cs->mProg;

        while (*cs->mProg != '"' && *cs->mProg != '\r') {
            ++len;
            AddColumn(cs, 1);
        }

        if (*cs->mProg == '\r') {
            SntxErrorBuild(SYNTAX, cs);
        }

        AddColumn(cs, 1);
        // +2 for putback just in case
        cs->mToken.mTokenName.mLen = len + 2;
        return (cs->mToken.mTokenType = STRING_VALUE);
    }

    return 0;
}

static LInt GetNumberValue(CompileState* cs) {
    LInt len = 0;
    if (LIsDigit(*cs->mProg)) {
        cs->mToken.mTokenName.mPtr = cs->mProg;

        while (LIsDigit(*cs->mProg)) {
            ++len;
            AddColumn(cs, 1);
        }

        cs->mToken.mTokenName.mLen = len;
        return (cs->mToken.mTokenType = NUMBER);
    }

    return 0;
}

static LInt NextToken(CompileState* cs) {
    cs->mToken.mTokenType = 0;
    cs->mToken.mTokenValue = 0;
    InitStr(&cs->mToken.mTokenName, kBoyiaNull);
    while (LIsSpace(*cs->mProg) && *cs->mProg) {
        if (*cs->mProg == '\n') {
            NewLine(cs);
        }
        AddColumn(cs, 1);
    }

    if (*cs->mProg == '\0') {
        cs->mToken.mTokenValue = BY_END;
        return (cs->mToken.mTokenType = DELIMITER);
    }

    // 程序注释部分
    SkipComment(cs);
    // 处理Token操作
    if (GetIdentifer(cs)
        || GetLogicValue(cs)
        || GetStringValue(cs)
        || GetDelimiter(cs)
        || GetNumberValue(cs)) {
        return cs->mToken.mTokenType;
    }

    return 0;
}

static LInt HandleCreateParam(Instruction* inst, BoyiaVM* vm) {
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;

    BoyiaFunction* function = &vm->mFunTable[vm->mFunSize - 1];
    BoyiaValue* value = &function->mParams[function->mParamSize++];
    value->mNameKey = hashKey;
    return kOpResultSuccess;
}

static LVoid InitParams(CompileState* cs) {
    do {
        NextToken(cs); // 得到属性名
        // 如果是右括号，则跳出循环
        if (cs->mToken.mTokenValue == RPTR) {
            break;
        }

        OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };
        PutInstruction(&cmd, kBoyiaNull, kCmdParamCreate, cs);
        // 获取逗号分隔符',', 最后一个是), 则跳出循环
        NextToken(cs); 
    } while (cs->mToken.mTokenValue == COMMA);
    if (cs->mToken.mTokenValue != RPTR) {
        SntxErrorBuild(PAREN_EXPECTED, cs);
    }
}

static CommandTable* CreateExecutor(CompileState* cs) {
    CommandTable* newTable = NEW(CommandTable, cs->mVm);
    newTable->mBegin = kBoyiaNull;
    newTable->mEnd = kBoyiaNull;
    return newTable;
}

// 初始化函数
static LVoid InitFunction(BoyiaFunction* fun, BoyiaVM* vm) {
    fun->mParamSize = 0;
    fun->mParams = NEW_ARRAY(BoyiaValue, NUM_FUNC_PARAMS, vm);
    fun->mParamCount = NUM_FUNC_PARAMS;
    ++vm->mFunSize;
}

// 创建函数
static BoyiaValue* CreateFunVal(LUintPtr hashKey, LUint8 type, BoyiaVM* vm) {
    // 初始化class类或函数变量
    BoyiaValue* val = &vm->mGlobals[vm->mGValSize++];
    BoyiaFunction* fun = &vm->mFunTable[vm->mFunSize];
    val->mValueType = type;
    val->mNameKey = hashKey;
    val->mValue.mObj.mPtr = (LIntPtr)fun;
    val->mValue.mObj.mSuper = kBoyiaNull;
    if (type == BY_CLASS) {
        fun->mFuncBody = (LIntPtr)val;
    }
    // 初始化类属性成员列表
    InitFunction(fun, vm);
    return val;
}

static LInt HandleCreateExecutor(Instruction* inst, BoyiaVM* vm) {
    CommandTable* newTable = NEW(CommandTable, vm);
    if (inst->mOPLeft.mValue != -1) {
        newTable->mBegin = &vm->mVMCode->mCode[inst->mOPLeft.mValue];
        newTable->mEnd = &vm->mVMCode->mCode[inst->mOPRight.mValue];
    } else {
        newTable->mBegin = kBoyiaNull;
        newTable->mEnd = kBoyiaNull;
    }

    vm->mFunTable[vm->mFunSize - 1].mFuncBody = (LIntPtr)newTable;
    return kOpResultSuccess;
}

// 解析函数体或者类体中的内容
static LVoid BodyStatement(CompileState* cs, LBool isFunction) {
    CommandTable* cmds = cs->mCmds;

    CommandTable tmpTable;
    tmpTable.mBegin = kBoyiaNull;
    tmpTable.mEnd = kBoyiaNull;

    Instruction* funInst = kBoyiaNull;
    if (isFunction) {
        // 类成员的创建在主体上下中进行
        OpCommand cmd = { OP_CONST_NUMBER, -1 };
        funInst = PutInstruction(&cmd, &cmd, kCmdExecCreate, cs);
        cs->mCmds = &tmpTable;
    }

    BlockStatement(cs);
    // 拷贝tmpTable中的offset给instruction, 非空函数
    if (funInst && tmpTable.mBegin) {
        funInst->mOPLeft.mType = OP_CONST_NUMBER;
        funInst->mOPLeft.mValue = (LIntPtr)(tmpTable.mBegin - cs->mVm->mVMCode->mCode);

        funInst->mOPRight.mType = OP_CONST_NUMBER;
        funInst->mOPRight.mValue = (LIntPtr)(tmpTable.mEnd - cs->mVm->mVMCode->mCode);
    }
    cs->mCmds = cmds;
}

static LInt HandleCreateClass(Instruction* inst, BoyiaVM* vm) {
    if (inst->mOPLeft.mType == OP_NONE) {
        AssignStateClass(vm->mEState, kBoyiaNull);
        return kOpResultSuccess;
    }
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;
    AssignStateClass(vm->mEState, CreateFunVal(hashKey, BY_CLASS, vm));
    return kOpResultSuccess;
}

static LInt HandleExtend(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* classVal = FindGlobal((LUintPtr)inst->mOPLeft.mValue, vm);
    BoyiaValue* extendVal = FindGlobal((LUintPtr)inst->mOPRight.mValue, vm);

    // set super pointer
    classVal->mValue.mObj.mSuper = (LIntPtr)extendVal;
    return kOpResultSuccess;
}

static LVoid ClassStatement(CompileState* cs) {
    NextToken(cs);
    LUintPtr classKey = GenIdentifier(&cs->mToken.mTokenName, cs->mVm);
    OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)classKey };
    PutInstruction(&cmd, kBoyiaNull, kCmdCreateClass, cs);
    // 判断继承关系
    NextToken(cs);
    LUintPtr extendKey = 0;
    if (BY_EXTEND == cs->mToken.mTokenValue) {
        NextToken(cs);
        extendKey = GenIdentifier(&cs->mToken.mTokenName, cs->mVm);
    } else {
        Putback(cs);
    }
    // 初始化类体
    BodyStatement(cs, LFalse);
    // 设置继承成员
    if (extendKey != 0) {
        OpCommand extendCmd = { OP_CONST_NUMBER, (LIntPtr)extendKey };
        PutInstruction(&cmd, &extendCmd, kCmdClassExtend, cs);
    }

    // 执行完后需将CLASS置为kBoyiaNull
    OpCommand cmdEnd = { OP_NONE, 0 };
    PutInstruction(&cmdEnd, kBoyiaNull, kCmdCreateClass, cs);
}

static LInt HandleFunCreate(Instruction* inst, BoyiaVM* vm) {
    // 匿名函数hashKey初始值位0，首次计算完后hashKey变成funTable索引
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;

    if (vm->mEState->mStackFrame.mClass.mValue.mObj.mPtr) {
        LIntPtr funType = inst->mOPRight.mValue;
        // 如果funType是BY_ANONYM_FUNC，表示是一个匿名函数
        if (funType == BY_ANONYM_FUNC && hashKey != 0) {
            return SetIntResult(hashKey, vm);
        }
        
        BoyiaFunction* func = (BoyiaFunction*)vm->mEState->mStackFrame.mClass.mValue.mObj.mPtr;
        func->mParams[func->mParamSize].mNameKey = hashKey;
        func->mParams[func->mParamSize].mValueType = funType;
        // 指针指向函数地址
        func->mParams[func->mParamSize].mValue.mObj.mPtr = (LIntPtr)&vm->mFunTable[vm->mFunSize];
        // 属性函数的mSuper指针指向对象实例
        func->mParams[func->mParamSize++].mValue.mObj.mSuper = IsObjectPropFunc(funType) ? (LIntPtr)func : kBoyiaNull;
        // 初始化函数参数列表
        InitFunction(&vm->mFunTable[vm->mFunSize], vm);
        if (funType == BY_ANONYM_FUNC && hashKey == 0) {
            // 将函数索引存起来，第二次遇到函数构建则直接返回
            inst->mOPLeft = { OP_CONST_NUMBER, vm->mFunSize - 1 };
            return SetIntResult(vm->mFunSize - 1, vm);
        }
    } else {
        CreateFunVal(hashKey, BY_FUNC, vm);
    }
    
    return kOpResultSuccess;
}

// funType可以是function，prop function, prop async
static LVoid FunStatement(CompileState* cs, LInt funType) {
    NextToken(cs);
    // 第一步，Function变量
    OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };
    OpCommand propCmd = { OP_CONST_NUMBER, funType };
    PutInstruction(&cmd, &propCmd, kCmdCreateFunction, cs);
    // 第二步，初始化函数参数
    NextToken(cs); //   '(', 即LPTR
    
    //  初始化参数
    InitParams(cs); 
    // 第三步，函数体内部编译
    BodyStatement(cs, LTrue);
}

static LVoid AnonymFunStatement(CompileState* cs) {
    NextToken(cs);
    if (cs->mToken.mTokenValue != LPTR) {
        SntxErrorBuild(PAREN_EXPECTED, cs);
        return;
    }

    // 如果是匿名函数
    OpCommand propCmd = { OP_CONST_NUMBER, BY_ANONYM_FUNC };
    PutInstruction(kBoyiaNull, &propCmd, kCmdCreateFunction, cs);
    // 函数指针放在R0，将R0 push到栈中
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);

    OpCommand jmpCmd = { OP_CONST_NUMBER, LTrue };
    Instruction* logicInst = PutInstruction(&jmpCmd, kBoyiaNull, kCmdOnceJmpTrue, cs);
    //  初始化参数
    InitParams(cs); 
    // 第三步，函数体内部编译
    BodyStatement(cs, LTrue);

    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdIfEnd, cs);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst); // Compute offset

    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPop, cs);
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdSetAnonym, cs);
}

// 执行全局的调用
static LVoid ExecuteCode(BoyiaVM* vm) {
    vm->mEState->mStackFrame.mPC = vm->mEState->mStackFrame.mContext->mBegin;
    ExecInstruction(vm);
    // 执行完指令后重置现场
    ResetScene(vm->mEState);
}

static LInt HandleDeclGlobal(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue val;
    val.mValueType = inst->mOPLeft.mValue;
    val.mNameKey = (LUintPtr)inst->mOPRight.mValue;
    ValueCopy(vm->mGlobals + vm->mGValSize++, &val);
    return kOpResultSuccess;
}

static LVoid GlobalStatement(CompileState* cs) {
    LInt type = cs->mToken.mTokenValue;
    do {
        NextToken(cs); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };

        PutInstruction(&cmdLeft, &cmdRight, kCmdDeclGlobal, cs);
        Putback(cs);
        EvalExpression(cs);
    } while (cs->mToken.mTokenValue == COMMA);

    if (cs->mToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED, cs);
    }
}

static LVoid AppendEntry(CompileState* cs) {
    CommandTable* table = cs->mCmds;
    // Entry的begin为空，证明没有指令
    if (!table || !table->mBegin) {
        return;
    }

    BoyiaVM* vm = cs->mVm;
    // Entry起始位置获取在VMCode中的索引
    vm->mEntry->mTable[vm->mEntry->mSize++] = table->mBegin - vm->mVMCode->mCode;
}

// 该函数记录全局变量以及函数接口
static LVoid ParseStatement(CompileState* cs) {
    LInt brace = 0; // ‘{’的个数
    cs->mCmds = CreateExecutor(cs);
    do {
        while (brace) {
            NextToken(cs);
            if (cs->mToken.mTokenValue == BLOCK_START) {
                ++brace;
            }

            if (cs->mToken.mTokenValue == BLOCK_END) {
                --brace;
            }
        }

        NextToken(cs);
        if (cs->mToken.mTokenValue == BY_VAR) {
            GlobalStatement(cs);
        } else if (cs->mToken.mTokenValue == BY_FUNC) {
            FunStatement(cs, BY_FUNC);
        } else if (cs->mToken.mTokenValue == BY_CLASS) {
            ClassStatement(cs);
        } else if (cs->mToken.mTokenType == IDENTIFIER) {
            Putback(cs);
            EvalExpression(cs);

            if (cs->mToken.mTokenValue != SEMI) {
                SntxErrorBuild(SEMI_EXPECTED, cs);
            }
        } else if (cs->mToken.mTokenValue == BLOCK_START) {
            ++brace;
        }
    } while (cs->mToken.mTokenValue != BY_END);    
    // 多文件嵌套编译时，需要保存执行环境
    ExecState* es = cs->mVm->mEState;
    // 如果cs->mVm->mEState->mStackFrame.mContext不为空，则表示嵌套编译
    // 既执行全局代码时，编译另外的boyia代码文件
    if (cs->mVm->mEState->mStackFrame.mContext) {
        ExecState* state = CreateExecState(cs->mVm);
        SwitchExecState(state, cs->mVm);
    }

    // 执行全局声明和定义
    cs->mVm->mEState->mStackFrame.mContext = cs->mCmds;
    ExecuteCode(cs->mVm);

    // 恢复执行环境
    if (cs->mVm->mEState != es) {
        DestroyExecState(cs->mVm->mEState, cs->mVm);
        SwitchExecState(es, cs->mVm);
    }

    AppendEntry(cs);
}

static LInt HandleDeclLocal(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue local;
    local.mValueType = inst->mOPLeft.mValue;
    local.mNameKey = (LUintPtr)inst->mOPRight.mValue;
    LocalPush(&local, vm); // 塞入本地符号表
    return kOpResultSuccess;
}

/* Declare a local variable. */
static LVoid LocalStatement(CompileState* cs) {
    LInt type = cs->mToken.mTokenValue;
    do {
        NextToken(cs); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };
        //EngineStrLog("value Name=%s", cs->mToken.mTokenName);
        PutInstruction(&cmdLeft, &cmdRight, kCmdDeclLocal, cs);
        Putback(cs);
        EvalExpression(cs);
    } while (cs->mToken.mTokenValue == COMMA);

    if (cs->mToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED, cs);
    }
}

static LVoid HandleCallAsyncFunction(BoyiaVM* vm) {
    ExecState* current = vm->mEState;
    // 第一个参数为调用该函数的函数指针
    LInt start = current->mExecStack[current->mFrameIndex - 1].mLValSize;
    BoyiaValue* value = &current->mLocals[start];
    BOYIA_LOG("HandlePushParams functionName=%u \n", value->mValueType);

    ExecState* state = CreateExecState(vm);
    state->mPrevious = current;
    SwitchExecState(state, vm);
    HandlePushScene(kBoyiaNull, vm);
    
    BoyiaFunction* func = (BoyiaFunction*)value->mValue.mObj.mPtr;
    // 从第二个参数开始，将形参key赋给实参
    LInt idx = start;
    LInt end = idx + func->mParamSize;
    for (; idx < end; ++idx) {
        ValueCopy(&state->mLocals[idx - start], &current->mLocals[idx]);
    }
    state->mStackFrame.mLValSize = func->mParamSize + 1;

    BoyiaValue val;
    val.mValueType = BY_CLASS;
    BoyiaFunction* objBody = (BoyiaFunction*)value->mValue.mObj.mSuper;

    val.mValue.mObj.mPtr = value->mValue.mObj.mSuper;
    val.mValue.mObj.mSuper = objBody->mFuncBody;

    AssignStateClass(vm->mEState, &val);
}

static LInt HandleCallFunction(Instruction* inst, BoyiaVM* vm) {
    // localstack第一个值为函数指针
    LInt start = vm->mExecStack[vm->mEState->mFrameIndex - 1].mLValSize;

    BoyiaValue* value = &vm->mLocals[start];
    BoyiaFunction* func = (BoyiaFunction*)value->mValue.mObj.mPtr;
    ValueCopy(&vm->mEState->mFun, value);
    
    LUint8 valueType = value->mValueType;
    // 内置类产生的对象，调用其方法
    if (valueType == BY_NAV_FUNC || valueType == BY_NAV_PROP) {
        // 将对象作为最后一个参数传入
        LocalPush(&vm->mEState->mStackFrame.mClass, vm);
        NativePtr navFun = (NativePtr)func->mFuncBody;
        // native函数没有instruction，所以pc置为null
        vm->mEState->mStackFrame.mPC = kBoyiaNull;
        return navFun(vm);
    } else if (valueType == BY_PROP_FUNC || valueType == BY_ANONYM_FUNC) {
        BoyiaValue val;
        val.mValueType = BY_CLASS;
        BoyiaFunction* objBody = (BoyiaFunction*)value->mValue.mObj.mSuper;

        val.mValue.mObj.mPtr = value->mValue.mObj.mSuper;
        val.mValue.mObj.mSuper = objBody->mFuncBody;
        AssignStateClass(vm->mEState, &val);
    }
    else if (valueType == BY_ASYNC_PROP) { HandleCallAsyncFunction(vm); }
    else if (valueType == BY_ASYNC) {}
    
    CommandTable* cmds = (CommandTable*)func->mFuncBody;
    vm->mEState->mStackFrame.mContext = cmds;
    vm->mEState->mStackFrame.mPC = cmds->mBegin;

    return kOpResultJumpFun;
}

static LInt HandlePushParams(Instruction* inst, BoyiaVM* vm) {
    ExecState* current = vm->mEState;
    // 第一个参数为调用该函数的函数指针
    LInt start = current->mExecStack[current->mFrameIndex - 1].mLValSize;
    BoyiaValue* value = &current->mLocals[start];
    BOYIA_LOG("HandlePushParams functionName=%u \n", value->mValueType);

    if (value->mValueType == BY_FUNC 
        || value->mValueType == BY_PROP_FUNC
        || value->mValueType == BY_ASYNC_PROP
        || value->mValueType == BY_ANONYM_FUNC) { // BY_NAV_FUNC不需要对参数名进行赋值
        BoyiaFunction* func = (BoyiaFunction*)value->mValue.mObj.mPtr;
        if (func->mParamSize <= 0) {
            return kOpResultSuccess;
        }
        // 从第二个参数开始，将形参key赋给实参
        LInt idx = start + 1;
        LInt end = idx + func->mParamSize;
        for (; idx < end; ++idx) {
            LUintPtr vKey = func->mParams[idx - start - 1].mNameKey;
            vm->mLocals[idx].mNameKey = vKey;
        }
    }

    return kOpResultSuccess;
}

static LInt HandlePop(Instruction* inst, BoyiaVM* vm) {
    if (inst->mOPLeft.mType != OP_REG0 && inst->mOPLeft.mType != OP_REG1) {
        --vm->mEState->mStackFrame.mResultNum;
        return kOpResultSuccess;
    }
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    ValueCopy(value, vm->mOpStack + (--vm->mEState->mStackFrame.mResultNum));
    return kOpResultSuccess;
}

/* Call a function. */
static void CallStatement(CompileState* cs, OpCommand* objCmd) {
    // 之所以要先用tmp保存当前局部变量个数，在于当前函数可能存在obj对象上下文
    // 如果直接pushscene，这个上下文就会消失，所以需要tmp先保存局部变量个数，
    // 在pushscene的时候恢复上一帧的局部变量个数
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdTmpLocal, cs);
    // 设置参数
    PushArgStatement(cs);
    // POP CLASS context
    if (objCmd->mType == OP_VAR) {
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPop, cs);
    }
    // 保存对象环境
    PutInstruction(objCmd, kBoyiaNull, kCmdPushObj, cs);
    // 保存调用堆栈
    Instruction* pushInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushScene, cs);
    // 函数形参名哈希值赋给局部变量
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushParams, cs);
    // 执行函数体
    Instruction* funInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdCallFunction, cs);
    //EngineLog("CallStatement=>%d HandleFunction", 1);
    pushInst->mOPLeft.mType = OP_CONST_NUMBER;
    pushInst->mOPLeft.mValue = (LIntPtr)(funInst - pushInst);
}

/* Push the arguments to a function onto the local variable stack. */
static LVoid PushArgStatement(CompileState* cs) {
    // push函数指针
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, cs);
    NextToken(cs); // if token == ')' exit
    if (cs->mToken.mTokenValue == RPTR) {
        return;
    }
    Putback(cs);

    do {
        // 参数值在R0中
        EvalExpression(cs); // => R0
        // 将函数实参压栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, cs);
        //NextToken();
    } while (cs->mToken.mTokenValue == COMMA);
}

/* Assign a value to a Register 0 or 1. */
static LInt HandleAssignment(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    if (!left)
        return kOpResultEnd;

    switch (inst->mOPRight.mType) {
    case OP_CONST_STRING: { // 字符串是存在全局表中
        BoyiaValue* val = (BoyiaValue*)inst->mOPRight.mValue;
        ValueCopyNoName(left, val);
    } break;
    case OP_CONST_NUMBER: {
        left->mValueType = BY_INT;
        left->mValue.mIntVal = inst->mOPRight.mValue;
    } break;
    case OP_VAR: {
        BoyiaValue* val = FindVal((LUintPtr)inst->mOPRight.mValue, vm);
        if (!val) {
            return kOpResultEnd;
        }

        if (val->mValueType == BY_VAR) {
            ValueCopy(left, val);
        } else {
            ValueCopyNoName(left, val);
            left->mNameKey = (LUintPtr)val;
        }
    } break;
    case OP_REG0: {
        ValueCopyNoName(left, &vm->mCpu->mReg0);
    } break;
    case OP_REG1: {
        ValueCopyNoName(left, &vm->mCpu->mReg1);
    } break;
    }

    return kOpResultSuccess;
}

// 执行到ifend证明整个if elif, else执行完毕，
// 无需检索是否还有elif，或者else的逻辑判断和内部block
static LInt HandleIfEnd(Instruction* inst, BoyiaVM* vm) {
    Instruction* pc = vm->mEState->mStackFrame.mPC;
    Instruction* tmpInst = NextInstruction(pc, vm); //inst->mNext;
    // 查看下一个是否是elseif
    BOYIA_LOG("HandleIfEnd R0=>%d \n", 1);
    while (tmpInst && (tmpInst->mOPCode == kCmdElif || tmpInst->mOPCode == kCmdElse)) {
        pc = (Instruction*)(tmpInst + tmpInst->mOPRight.mValue); // 跳转到elif对应的IFEND
        tmpInst = NextInstruction(pc, vm);
    }
    BOYIA_LOG("HandleIfEnd END R0=>%d \n", 1);
    if (pc) {
        vm->mEState->mStackFrame.mPC = pc;
    }
    return kOpResultSuccess;
}

static LInt HandleJumpToIfTrue(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* value = GetOpValue(inst, OpLeft, vm);
    // 如果为false，则跳过执行
    if (value && !value->mValue.mIntVal) {
        vm->mEState->mStackFrame.mPC = inst + inst->mOPRight.mValue;
    }

    return kOpResultSuccess;
}

// 该指令表示ifend之前的代码，只执行一遍
static LInt HandleOnceJmpTrue(Instruction* inst, BoyiaVM* vm) {
    // 必须是数字，否则执行失败，结束执行
    if (inst->mOPLeft.mType != OP_CONST_NUMBER) {
        return kOpResultEnd;
    }
    // 第一次默认执行，第二次执行该指令进行跳转
    if (!inst->mOPLeft.mValue) {
        vm->mEState->mStackFrame.mPC = inst + inst->mOPRight.mValue;
    } else {
        // 设置为False，第二次执行，则跳过
        inst->mOPLeft.mValue = LFalse;
    }

    return kOpResultSuccess;
}

/* Execute an if statement. */
static LVoid IfStatement(CompileState* cs) {
    NextToken(cs);
    // token = (
    EvalExpression(cs); /* check the conditional expression => R0 */
    Instruction* logicInst = PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdJmpTrue, cs);
    //EngineStrLog("endif last inst name=%s", cs->mToken.mTokenName);
    BlockStatement(cs); /* if true, interpret */
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdIfEnd, cs);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    //logicInst->mOPRight.mValue = (LIntPtr)endInst; // 最后地址值
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst); // Compute offset
}

static LInt HandleLoopBegin(Instruction* inst, BoyiaVM* vm) {
    // push left => loop stack
    vm->mLoopStack[vm->mEState->mStackFrame.mLoopSize++] = (LIntPtr)(inst + inst->mOPLeft.mValue);
    return kOpResultSuccess;
}

static LInt HandleLoopIfTrue(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* value = &vm->mCpu->mReg0;
    if (!value->mValue.mIntVal) {
        vm->mEState->mStackFrame.mPC = inst + inst->mOPRight.mValue;
        vm->mEState->mStackFrame.mLoopSize--;
        return kOpResultSuccess;
    }

    if (inst->mOPLeft.mValue) {
        vm->mEState->mStackFrame.mPC = inst + inst->mOPLeft.mValue;
    }

    return kOpResultSuccess;
}

static LInt HandleJumpTo(Instruction* inst, BoyiaVM* vm) {
    if (inst->mOPLeft.mType == OP_CONST_NUMBER) {
        vm->mEState->mStackFrame.mPC = inst - inst->mOPLeft.mValue;
    }
    return kOpResultSuccess;
}

/* Execute a while loop. */
static LVoid WhileStatement(CompileState* cs) {
    Instruction* beginInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoop, cs);
    NextToken(cs); // '('
    if (cs->mToken.mTokenValue != LPTR) {
        SntxErrorBuild(LPTR_EXPECTED, cs);
    }
    EvalExpression(cs); /* check the conditional expression => R0 */
    if (cs->mToken.mTokenValue != RPTR) {
        SntxErrorBuild(RPTR_EXPECTED, cs);
    }

    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoopTrue, cs);
    BlockStatement(cs); /* If true, execute block */
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdJmpTo, cs);
    beginInst->mOPLeft.mType = OP_CONST_NUMBER;
    //beginInst->mOPLeft.mValue = (LIntPtr)endInst; // 最后地址值
    beginInst->mOPLeft.mValue = (LIntPtr)(endInst - beginInst);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    //logicInst->mOPRight.mValue = (LIntPtr)endInst; // 最后地址值
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst);
    endInst->mOPLeft.mType = OP_CONST_NUMBER;
    //endInst->mOPLeft.mValue = (LIntPtr)beginInst; // LOOP开始地址值
    endInst->mOPLeft.mValue = (LIntPtr)(endInst - beginInst);
}

/* Execute a do loop. */
static LVoid DoStatement(CompileState* cs) {
    Instruction* beginInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoop, cs);
    BlockStatement(cs); /* interpret loop */
    NextToken(cs);
    if (cs->mToken.mTokenValue != BY_WHILE) {
        SntxErrorBuild(WHILE_EXPECTED, cs);
    }
    EvalExpression(cs); /* check the loop condition */

    if (cs->mToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED, cs);
    }
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoopTrue, cs);
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdJmpTo, cs);
    beginInst->mOPLeft.mType = OP_CONST_NUMBER;
    //beginInst->mOPLeft.mValue = (LIntPtr)endInst; // 最后地址值
    beginInst->mOPLeft.mValue = (LIntPtr)(endInst - beginInst);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    //logicInst->mOPRight.mValue = (LIntPtr)endInst; // 最后地址值
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst);
    endInst->mOPLeft.mType = OP_CONST_NUMBER;
    //endInst->mOPLeft.mValue = (LIntPtr)beginInst; // LOOP开始地址值
    endInst->mOPLeft.mValue = (LIntPtr)(endInst - beginInst);
}

static LInt HandleAdd(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);
    if (!left || !right) {
        return kOpResultEnd;
    }

    if (left->mValueType == BY_INT && right->mValueType == BY_INT) {
        right->mValue.mIntVal += left->mValue.mIntVal;
        BOYIA_LOG("HandleAdd result=%d", right->mValue.mIntVal);
        return kOpResultSuccess;
    }

    if (left->mValueType != BY_CLASS && right->mValueType != BY_CLASS) {
        return kOpResultEnd;
    }

    if (GetBoyiaClassId(left) == kBoyiaString || GetBoyiaClassId(right) == kBoyiaString) {
        BOYIA_LOG("StringAdd Begin %d", 1);
        StringAdd(left, right, vm);
        return kOpResultSuccess;
    }

    return kOpResultEnd;
}

static LInt HandleSub(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);
    if (!left || !right) {
        return kOpResultEnd;
    }

    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return kOpResultEnd;

    right->mValue.mIntVal = left->mValue.mIntVal - right->mValue.mIntVal;
    BOYIA_LOG("HandleSub R0=>%d", vm->mCpu->mReg0.mValue.mIntVal);
    return kOpResultSuccess;
}

static LInt HandleMul(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return kOpResultEnd;
    }

    BOYIA_LOG("HandleMul left=%d \n", left->mValue.mIntVal);
    BOYIA_LOG("HandleMul right=%d \n", right->mValue.mIntVal);
    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return kOpResultEnd;

    right->mValue.mIntVal *= left->mValue.mIntVal;
    BOYIA_LOG("HandleMul result=%d \n", right->mValue.mIntVal);
    return kOpResultSuccess;
}

static LInt HandleDiv(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return kOpResultEnd;
    }

    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return kOpResultEnd;

    if (right->mValue.mIntVal == 0)
        return kOpResultEnd;

    right->mValue.mIntVal = left->mValue.mIntVal / right->mValue.mIntVal;
    return kOpResultSuccess;
}

static LInt HandleMod(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return kOpResultEnd;
    }

    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return kOpResultEnd;

    if (right->mValue.mIntVal == 0)
        return kOpResultEnd;

    right->mValue.mIntVal = left->mValue.mIntVal % right->mValue.mIntVal;
    return kOpResultSuccess;
}

static LInt HandleRelational(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return kOpResultEnd;
    }

    BOYIA_LOG("HandleLogic left=%d \n", left->mValue.mIntVal);
    BOYIA_LOG("HandleLogic right=%d \n", right->mValue.mIntVal);

    LInt result = kOpResultEnd;
    switch (inst->mOPCode) {
    case kCmdNotRelation:
        result = right->mValue.mIntVal ? kOpResultEnd : kOpResultSuccess;
        break;
    case kCmdLtRelation:
        result = left->mValue.mIntVal < right->mValue.mIntVal ? kOpResultSuccess : kOpResultEnd;
        break;
    case kCmdLeRelation:
        result = left->mValue.mIntVal <= right->mValue.mIntVal ? kOpResultSuccess : kOpResultEnd;
        break;
    case kCmdGtRelation:
        result = left->mValue.mIntVal > right->mValue.mIntVal ? kOpResultSuccess : kOpResultEnd;
        break;
    case kCmdGeRelation:
        result = left->mValue.mIntVal >= right->mValue.mIntVal ? kOpResultSuccess : kOpResultEnd;
        break;
    case kCmdEqRelation:
        result = left->mValue.mIntVal == right->mValue.mIntVal ? kOpResultSuccess : kOpResultEnd;
        break;
    case kCmdNeRelation:
        result = left->mValue.mIntVal != right->mValue.mIntVal ? kOpResultSuccess : kOpResultEnd;
        break;
    }

    right->mValueType = BY_INT;
    right->mValue.mIntVal = result;
    return kOpResultSuccess;
}

static LInt HandleLogic(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return kOpResultEnd;
    }

    LInt result = kOpResultEnd;
    switch (inst->mOPCode) {
    case kCmdAndLogic:
        result = left->mValue.mIntVal && right->mValue.mIntVal ? kOpResultSuccess : kOpResultEnd;
        break;
    case kCmdOrLogic:
        result = left->mValue.mIntVal || right->mValue.mIntVal ? kOpResultSuccess : kOpResultEnd;
        break;
    }

    right->mValueType = BY_INT;
    right->mValue.mIntVal = result;
    return kOpResultSuccess;
}

static LInt HandleAwait(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);

    // 获取对象
    BoyiaFunction* fun = (BoyiaFunction*)left->mValue.mObj.mPtr;
    BoyiaValue* klass = (BoyiaValue*)fun->mFuncBody;
    PrintValueKey(klass, vm);
    if (GetBoyiaClassId(klass) != kBoyiaMicroTask) {
        return HandleReturn(inst, vm);
    }

    vm->mEState->mWait = LTrue;

    vm->mEState->mTopTask = AllocMicroTask(vm);
    vm->mEState->mTopTask->mAsyncEs = kBoyiaNull;
    
    // 获取微任务
    MicroTask* task = (MicroTask*)fun->mParams[1].mValue.mIntVal;
    task->mAsyncEs = vm->mEState;

    // 设置顶层async与task之间的联系
    BoyiaFunction* taskObj = CreateMicroTaskObject(vm);
    MicroTask* topTask = vm->mEState->mTopTask;
    taskObj->mParams[1].mValue.mIntVal = (LIntPtr)topTask;

    BoyiaValue result;
    result.mValueType = BY_CLASS;
    result.mValue.mObj.mPtr = (LIntPtr)taskObj;
    result.mValue.mObj.mSuper = kBoyiaNull;
    SetNativeResult(&result, vm);
    
    if (vm->mEState->mPrevious && !vm->mEState->mPrevious->mWait) {
        SwitchExecState(vm->mEState->mPrevious, vm);
        // 切换到之前的ExecState，但由于函数调用前便执行了PopScene，
        // 之前的ExecState的mFrameIndex是进行了+1，因而需要pc置为Null，
        // 这样才会在执行命令时执行PopScene，返回到先前的场景
        vm->mEState->mStackFrame.mPC = kBoyiaNull;
    } else {
        return kOpResultEnd;
    }

    return kOpResultSuccess;
}

static LInt HandleSetAnonym(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* value = GetOpValue(inst, OpLeft, vm);
    BoyiaFunction* func = &vm->mFunTable[value->mValue.mIntVal];
    BoyiaValue result;
    result.mValueType = BY_ANONYM_FUNC;
    result.mValue.mObj.mPtr = (LIntPtr)func;
    // 匿名函数指向当前对象
    result.mValue.mObj.mSuper = (LIntPtr)vm->mEState->mStackFrame.mClass.mValue.mObj.mPtr;
    SetNativeResult(&result, vm);
    
    return kOpResultSuccess;
}

// 执行到await，当前调用栈将被中断
static LVoid EvalAwait(CompileState* cs) {
    NextToken(cs);
    // 先执行异步函数，强制认为其后是一个函数
    Atom(cs);
    // Atom创建函数执行指令，执行函数后返回一个微任务，同时将这个微任务挂起，阻塞当前函数的执行
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdAwait, cs);
}

// 处理匿名函数
static LBool EvalAnonymFunc(CompileState* cs) {
    if (cs->mToken.mTokenType == KEYWORD
        && cs->mToken.mTokenValue == BY_FUNC) {        
        AnonymFunStatement(cs);
        NextToken(cs);
        return LTrue;
    }

    return LFalse;
}

/* parser lib define */
// 不能直接使用await方法来做运算，例如await function() * 32
// 需要在await function()两侧加括号，表示这是一个表达式，例如
// (await function()) * 32; boyia语言将严格这一范式
static LVoid EvalExpression(CompileState* cs) {
    NextToken(cs); // var or await
    if (cs->mToken.mTokenValue == BY_AWAIT) {
        EvalAwait(cs);
        return;
    }

    // 如果是匿名函数
    if (EvalAnonymFunc(cs)) {
        return;
    }

    if (!cs->mToken.mTokenName.mLen) {
        SntxErrorBuild(NO_EXP, cs);
        return;
    }

    if (cs->mToken.mTokenValue == SEMI) {
        return;
    }

    EvalAssignment(cs);
}

static LInt HandlePush(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    ValueCopy(vm->mOpStack + (vm->mEState->mStackFrame.mResultNum++), value);
    return kOpResultSuccess;
}

static LInt HandleAssignVar(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* result = GetOpValue(inst, OpRight, vm);
    if (!left || !result) {
        return kOpResultEnd;
    }

    
    BoyiaValue* value = kBoyiaNull;
    if (left->mValueType == BY_VAR) {
        value = FindVal(left->mNameKey, vm);
    } else {
        value = (BoyiaValue*)left->mNameKey;
    }

    //BoyiaValue* value = (BoyiaValue*)left->mNameKey;

    ValueCopyNoName(value, result);
    ValueCopy(&vm->mCpu->mReg0, value);
    return kOpResultSuccess;
}

// 调用native函数时不需要保存函数指针
static LVoid CallNativeStatement(CompileState* cs, LInt idx) {
    NextToken(cs);
    if (cs->mToken.mTokenValue != LPTR) // '('
        SntxErrorBuild(PAREN_EXPECTED, cs);

    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdTmpLocal, cs);
    do {
        // 参数值在R0中
        EvalExpression(cs); // => R0
        // 将函数参数压栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, cs);
        // if token == ')' exit
    } while (cs->mToken.mTokenValue == COMMA);

    // 保存obj现场是必不可少的一步
    OpCommand cmdSet = { OP_CONST_NUMBER, 0 };
    PutInstruction(&cmdSet, kBoyiaNull, kCmdPushObj, cs);

    Instruction* pushInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushScene, cs);
    OpCommand cmd = { OP_CONST_NUMBER, idx };
    PutInstruction(&cmd, kBoyiaNull, kCmdCallNative, cs);
    Instruction* popInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPopScene, cs);
    pushInst->mOPLeft.mType = OP_CONST_NUMBER;
    pushInst->mOPLeft.mValue = (LIntPtr)(popInst - pushInst);
}

static BoyiaValue* FindObjProp(BoyiaValue* lVal, LUintPtr rVal, Instruction* inst, BoyiaVM* vm) {
    yanbo::TimeAnalysis analysis("FindObjProp");
    if (!lVal || lVal->mValueType != BY_CLASS) {
        return kBoyiaNull;
    }

    // find props, such as obj.prop1.
    BoyiaFunction* fun = (BoyiaFunction*)lVal->mValue.mObj.mPtr;
    BoyiaValue* klass = (BoyiaValue*)fun->mFuncBody;

    LInt idx = 0;
    for (; idx < fun->mParamSize; ++idx) {
        if (fun->mParams[idx].mNameKey == rVal) {
            if (inst) {
                InlineCache* cache = inst->mCache ? inst->mCache : (inst->mCache = CreateInlineCache(vm));
                AddPropInlineCache(cache, klass, idx);
            }
            return fun->mParams + idx;
        }
    }

    // find function, such as obj.func1
    BoyiaValue* cls = klass;
    while (cls && cls->mValueType == BY_CLASS) {
        BoyiaFunction* clsMap = (BoyiaFunction*)cls->mValue.mObj.mPtr;
        LInt funIdx = 0;
        for (; funIdx < clsMap->mParamSize; ++funIdx) {
            if (clsMap->mParams[funIdx].mNameKey == rVal) {
                BoyiaValue* result = clsMap->mParams + funIdx;
                if (inst) {
                    InlineCache* cache = inst->mCache ? inst->mCache : (inst->mCache = CreateInlineCache(vm));
                    AddFunInlineCache(cache, klass, result);
                }
                return result;
            }
        }

        cls = (BoyiaValue*)cls->mValue.mObj.mSuper;
    }

    return kBoyiaNull;
}

static LVoid ValueCopyWithKey(BoyiaValue* dest, BoyiaValue* src) {
    dest->mNameKey = (LUintPtr)src;
    ValueCopyNoName(dest, src);
}

static LInt HandleGetProp(Instruction* inst, BoyiaVM* vm) {
    // r0 -> lVal
    BoyiaValue* lVal = GetOpValue(inst, OpLeft, vm);
    if (!lVal) {
        return kOpResultEnd;
    }

    // fetch value from inline cache
    BoyiaValue* result = GetInlineCache(inst->mCache, lVal);
    if (result) {
        ValueCopyWithKey(&vm->mCpu->mReg0, result);
        return kOpResultSuccess;
    }

    LUintPtr rVal = (LUintPtr)inst->mOPRight.mValue;
    result = FindObjProp(lVal, rVal, inst, vm);
    if (result) {
        // maybe function
        ValueCopyWithKey(&vm->mCpu->mReg0, result);
        return kOpResultSuccess;
    }

    RuntimeError(rVal, RUNTIME_NO_PROP, vm);
    return kOpResultEnd;
}

static LInt HandleCreateMap(Instruction* inst, BoyiaVM* vm) {
    BoyiaFunction* fun = CreatMapObject(vm);
    
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    value->mValueType = BY_CLASS;
    value->mValue.mObj.mPtr = (LIntPtr)fun;
    value->mValue.mObj.mSuper = kBoyiaNull;
    
    return kOpResultSuccess;
}

static LInt HandleSetMapKey(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;

    BoyiaFunction* function = (BoyiaFunction*)value->mValue.mObj.mPtr;
    BoyiaValue* param = &function->mParams[function->mParamSize++];
    param->mNameKey = (LUintPtr)inst->mOPRight.mValue;
    
    return kOpResultSuccess;
}

static LInt HandleSetMapValue(Instruction* inst, BoyiaVM* vm) {
    BoyiaValue* obj = inst->mOPRight.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;

    BoyiaFunction* function = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    BoyiaValue* param = &function->mParams[function->mParamSize - 1];
    ValueCopyNoName(param, value);
    
    SetNativeResult(obj, vm);
    
    return kOpResultSuccess;
}

static LInt HandleCreateArray(Instruction* inst, BoyiaVM* vm) {
    BoyiaFunction* fun = CreateArrayObject(vm);
    
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    value->mValueType = BY_CLASS;
    value->mValue.mObj.mPtr = (LIntPtr)fun;
    value->mValue.mObj.mSuper = kBoyiaNull;
        
    return kOpResultSuccess;
}

static LInt HandleAddArrayItem(Instruction* inst, BoyiaVM* vm) {
    // right是数组对象
    BoyiaValue* obj = inst->mOPRight.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    // left是需要添加的数组的值
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    BoyiaFunction* function = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    BoyiaValue* param = &function->mParams[function->mParamSize++];
    // 将值赋值到对应的索引
    ValueCopy(param, value);
    // 设置给R0
    SetNativeResult(obj, vm);
    
    return kOpResultSuccess;
}

// According to reg0, get reg0 obj's prop
static LVoid EvalGetProp(CompileState* cs) {
    NextToken(cs);
    if (cs->mToken.mTokenType != IDENTIFIER) {
        return;
    }

    // Push class context for callstatement
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);
    LUintPtr propKey = GenIdentifier(&cs->mToken.mTokenName, cs->mVm);
    OpCommand cmdR = { OP_CONST_NUMBER, (LIntPtr)propKey };
    PutInstruction(&COMMAND_R0, &cmdR, kCmdGetProp, cs);

    // Last must next
    NextToken(cs);
    if (cs->mToken.mTokenValue == LPTR) {
        OpCommand objCmd = { OP_VAR, 0 };
        CallStatement(cs, &objCmd); // result into r0
        NextToken(cs);
        if (cs->mToken.mTokenValue == DOT) {
            // obj.func().prop
            EvalGetProp(cs);
        }
    } else if (cs->mToken.mTokenValue == DOT) {
        OpCommand cmd = { OP_CONST_NUMBER, 0 };
        PutInstruction(&cmd, kBoyiaNull, kCmdPop, cs);
        EvalGetProp(cs);
    } else {
        OpCommand cmd = { OP_CONST_NUMBER, 0 };
        PutInstruction(&cmd, kBoyiaNull, kCmdPop, cs);
    }
}

static LVoid EvalGetValue(CompileState* cs, LUintPtr objKey) {
    OpCommand cmdR = { OP_VAR, (LIntPtr)objKey };
    PutInstruction(&COMMAND_R0, &cmdR, kCmdAssign, cs);
    if (cs->mToken.mTokenValue == DOT) {
        EvalGetProp(cs);
    }
}

static LVoid CopyStringFromToken(CompileState* cs, BoyiaStr* str) {
    str->mLen = cs->mToken.mTokenName.mLen - 2;
    str->mPtr = FAST_NEW_ARRAY(LInt8, str->mLen);
    LMemcpy(str->mPtr, cs->mToken.mTokenName.mPtr, str->mLen * sizeof(LInt8));
}

static LInt HandleConstString(Instruction* inst, BoyiaVM* vm) {
    
    BoyiaStr* str = &vm->mStrTable->mTable[inst->mOPLeft.mValue];

    CreateConstString(&vm->mCpu->mReg0, str->mPtr, str->mLen, vm);
    return kOpResultSuccess;
}

static LVoid Atom(CompileState* cs) {
    switch (cs->mToken.mTokenType) {
    case IDENTIFIER: {
        LUintPtr key = GenIdentifier(&cs->mToken.mTokenName, cs->mVm);
        LInt idx = FindNativeFunc(key, cs->mVm);
        if (idx != -1) {
            CallNativeStatement(cs, idx);
            NextToken(cs);
        } else {
            NextToken(cs);
            if (cs->mToken.mTokenValue == LPTR) {
                OpCommand cmd = { OP_VAR, (LIntPtr)key };
                PutInstruction(&COMMAND_R0, &cmd, kCmdAssign, cs);
                OpCommand objCmd = { OP_CONST_NUMBER, 0 };
                CallStatement(cs, &objCmd);
                NextToken(cs);
                // fun(xxx).prop
                if (cs->mToken.mTokenValue == DOT) {
                    EvalGetProp(cs);
                }
            } else {
                //EngineLog("Atom var name %u \n", key);
                EvalGetValue(cs, key);
            }
        }
    }
        return;
    case NUMBER: {
        OpCommand cmd = { OP_CONST_NUMBER, STR2_INT(cs->mToken.mTokenName) };
        //EngineLog("Atom NUMBER=%d \n", cmd.mValue);
        PutInstruction(&COMMAND_R0, &cmd, kCmdAssign, cs);
        NextToken(cs);
    }
        return;
    case STRING_VALUE: {
        // 从StringTable中分配常量字符串
        BoyiaStr* constStr = &cs->mVm->mStrTable->mTable[cs->mVm->mStrTable->mSize];
        CopyStringFromToken(cs, constStr);
        OpCommand lCmd = { OP_CONST_NUMBER, cs->mVm->mStrTable->mSize++ };
        //OpCommand rCmd = { OP_CONST_NUMBER, constStr.mLen };
        PutInstruction(&lCmd, kBoyiaNull, kCmdConstStr, cs);
        NextToken(cs);
    }
        return;
    default: {
        if (cs->mToken.mTokenValue == RPTR) {
            return;
        }

        SntxErrorBuild(SYNTAX, cs);
    } break;
    }
}

// 执行子表达式
static LVoid EvalSubexpr(CompileState* cs) {
    if (cs->mToken.mTokenValue == LPTR) {
        EvalExpression(cs);
        if (cs->mToken.mTokenValue != RPTR) {
            SntxErrorBuild(PAREN_EXPECTED, cs);
        }
        NextToken(cs);
    } else {
        Atom(cs);
    }
}

// 正负数,+1,-1
static LVoid EvalMinus(CompileState* cs) {
    LInt8 op = 0;
    if (cs->mToken.mTokenValue == ADD || cs->mToken.mTokenValue == SUB) {
        op = cs->mToken.mTokenValue;
        NextToken(cs);
    }
    EvalSubexpr(cs); // => R0
    if (op && op == SUB) { // negative must multiply -1
        OpCommand cmd = { OP_CONST_NUMBER, -1 };
        PutInstruction(&COMMAND_R1, &cmd, kCmdAssign, cs);
        PutInstruction(&COMMAND_R0, &COMMAND_R1, kCmdMul, cs);
    }
}

// 乘除, *,/,%
static LVoid EvalArith(CompileState* cs) {
    LInt8 op = 0;
    EvalMinus(cs);

    while ((op = cs->mToken.mTokenValue) == MUL || op == DIV || op == MOD) { // * / %
        // PUSH R0
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);
        NextToken(cs);
        EvalMinus(cs);
        // POP R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, cs);
        switch (op) {
        case MUL:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdMul, cs);
            break;
        case DIV:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdDiv, cs);
            break;
        case MOD:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdMod, cs);
            break;
        }
    }
}

// 加减,+,-
static LVoid EvalAddSub(CompileState* cs) {
    LInt8 op = 0;
    EvalArith(cs); // => R0
    while ((op = cs->mToken.mTokenValue) == ADD || op == SUB) {
        // PUSH
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);

        NextToken(cs);
        EvalArith(cs);
        // POP R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, cs);
        // R0 + R1 => R0
        // R1 - R0 => R0
        PutInstruction(&COMMAND_R1, &COMMAND_R0,
            op == ADD ? kCmdAdd : kCmdSub, cs);
    }
}

static LVoid EvalRelationalImpl(LInt8 opToken, CompileState* cs) {
    switch (opToken) {
    case NOT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdNotRelation, cs);
        break;
    case LT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdLtRelation, cs);
        break;
    case LE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdLeRelation, cs);
        break;
    case GT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdGtRelation, cs);
        break;
    case GE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdGeRelation, cs);
        break;
    case EQ:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdEqRelation, cs);
        break;
    case NE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdNeRelation, cs);
        break;
    }
}

// 关系比较判断,>,<,==,!=
static LVoid EvalRelational(CompileState* cs) {
    static LInt8 relops[8] = {
        NOT, LT, LE, GT, GE, EQ, NE, 0
    };

    // '<' and '>' etc need both sides of expression
    // ，but '!' only need right side.
    if (cs->mToken.mTokenValue != NOT) {
        EvalAddSub(cs); // 计算结果 => R0
    }

    LInt8 op = cs->mToken.mTokenValue;
    if (MStrchr(relops, op)) {
        // 计算的结果存入栈中
        if (op != NOT) {
            PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);
        }

        NextToken(cs); // 查找标识符或者常量
        EvalAddSub(cs); // 先执行优先级高的操作 => R0
        // pop R1
        // 上次计算的结果出栈至R1
        if (op != NOT) {
            PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, cs);
        }

        // 计算R0 OP R1, 结果存入R0中
        //PutInstruction(&COMMAND_R1, &COMMAND_R0, op, HandleRelational);
        EvalRelationalImpl(op, cs);
    }
}

static LVoid EvalLogic(CompileState* cs) {
    static LInt8 logicops[3] = {
        AND, OR, 0
    };

    EvalRelational(cs);
    LInt8 op = 0;
    while (MStrchr(logicops, (op = cs->mToken.mTokenValue))) {
        // 计算的结果存入栈中
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);

        NextToken(cs); // 查找标识符或者常量
        EvalRelational(cs); // 先执行优先级高的操作 => R0
        // pop R1
        // 上次计算的结果出栈至R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, cs);

        // 计算R0 OP R1, 结果存入R0中
        PutInstruction(&COMMAND_R1, &COMMAND_R0, op == AND ? kCmdAndLogic : kCmdOrLogic, cs);
    }
}

// 如果是Map对象
static LVoid EvalObject(CompileState* cs) {
    if (cs->mToken.mTokenValue != BLOCK_START) {
        EvalLogic(cs);
        return;
    }
    
    // 创建一个Map对象
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdCreateMap, cs);    
    do {
        NextToken(cs);
        // 如果是 } 则语句结束
        if (cs->mToken.mTokenValue == BLOCK_END) {
            return;
        }

        // 如果不是标识符提示出错
        if (cs->mToken.mTokenType != IDENTIFIER && cs->mToken.mTokenType != STRING_VALUE) {
            SntxErrorBuild(CREATE_MAP_ERROR, cs);
            return;
        }

        BoyiaStr keyStr;
        keyStr.mPtr = cs->mToken.mTokenName.mPtr;
        keyStr.mLen = cs->mToken.mTokenType == STRING_VALUE
            ? cs->mToken.mTokenName.mLen - 2
            : cs->mToken.mTokenName.mLen;
        
        OpCommand cmd = {
            OP_CONST_NUMBER,
            (LIntPtr)GenIdentifier(&keyStr, cs->mVm)
        };

        // 此时R0为Map对象
        PutInstruction(&COMMAND_R0, &cmd, kCmdSetMapKey, cs);

        NextToken(cs); // : or =
        if (cs->mToken.mTokenValue != COLON && cs->mToken.mTokenValue != ASSIGN) {
            SntxErrorBuild(MAP_KEY_VALUE_ERROR, cs);
            return;
        }

        // R0存入栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);
        // 计算value
        EvalExpression(cs); // => R0
        // 从栈中吐出数据到R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, cs);
        PutInstruction(&COMMAND_R0, &COMMAND_R1, kCmdSetMapValue, cs);
    } while (cs->mToken.mTokenValue == COMMA); // 遇到}结束
    // 进入下一个token
    NextToken(cs);
}

// 计算Array
static LVoid EvalArray(CompileState* cs) {
    // 数组以[开头
    if (cs->mToken.mTokenValue != ARRAY_BEGIN) {
        EvalObject(cs);
        return;
    }
    
    // 创建一个Array对象
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdCreateArray, cs);
    
    NextToken(cs);
    // 如果是 ] 则语句结束
    if (cs->mToken.mTokenValue == ARRAY_END) {
        return;
    }
    
    Putback(cs);
    
    do {
        // R0存入栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);
        
        // 计算value
        EvalExpression(cs);
        // 从栈中吐出数据到R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, cs);
        PutInstruction(&COMMAND_R0, &COMMAND_R1, kCmdAddArrayItem, cs);
    } while(cs->mToken.mTokenValue == COMMA); // 遇到]结束
    
    // 进入下一个token
    NextToken(cs);
}

// 赋值,=
static LVoid EvalAssignment(CompileState* cs) {
    EvalArray(cs); // =>R0
    if (cs->mToken.mTokenValue != ASSIGN) { // '='
        return;
    }
    
    // R0存入栈
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, cs);
    NextToken(cs);
    EvalArray(cs); // =>R0
    // 从栈中吐出数据到R1
    PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, cs);
    PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdAssignVar, cs);
}

LInt SetIntResult(LInt result, LVoid* vm)
{
    BoyiaValue value;
    value.mValueType = BY_INT;
    value.mValue.mIntVal = result;
    SetNativeResult(&value, vm);
    return kOpResultSuccess;
}

LVoid SetNativeResult(LVoid* result, LVoid* vm) {
    BoyiaValue* value = (BoyiaValue*)result;
    ValueCopy(&((BoyiaVM*)vm)->mCpu->mReg0, value);
}

LVoid* CopyObject(LUintPtr hashKey, LInt size, LVoid* vm) {
    BoyiaFunction* objBody = CopyFunction(FindGlobal(hashKey, (BoyiaVM*)vm), size, (BoyiaVM*)vm);
    GCAppendRef(objBody, BY_CLASS, vm);
    return objBody;
}

LVoid* GetNativeResult(LVoid* vm) {
    return &((BoyiaVM*)vm)->mCpu->mReg0;
}

LVoid* GetNativeHelperResult(LVoid* vm) {
    return &((BoyiaVM*)vm)->mCpu->mReg1;
}

LVoid* GetLocalStack(LIntPtr* stack, LInt* size, LVoid* vm, LVoid* ptr) {
    if (!ptr) {
        *size = 0;
        return ptr;
    }

    ExecState* state = kBoyiaNull;
    if (vm == ptr) {
        BoyiaVM* vmPtr = (BoyiaVM*)vm;
        state = vmPtr->mESLink;
    } else {
        state = (ExecState*)ptr;
    }

    *stack = (LIntPtr)state->mLocals;
    *size = state->mStackFrame.mLValSize;

    return (LVoid*)state->mPrev;
}

LVoid GetGlobalTable(LIntPtr* table, LInt* size, LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    *table = (LIntPtr)vmPtr->mGlobals;
    *size = vmPtr->mGValSize;
}

/*  output function */
LVoid CompileCode(LInt8* code, LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    CompileState cs;
    cs.mProg = code;
    cs.mLineNum = 1;
    cs.mColumnNum = 0;
    cs.mVm = vmPtr;
    ParseStatement(&cs); // 该函数记录全局变量以及函数接口
}

LVoid* GetLocalValue(LInt idx, LVoid* vm) {
    LInt size = GetLocalSize(vm);
    if (idx >= size) {
        return kBoyiaNull;
    }
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    LInt start = vmPtr->mExecStack[vmPtr->mEState->mFrameIndex - 1].mLValSize;
    return &vmPtr->mLocals[start + idx];
}

LInt GetLocalSize(LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    return vmPtr->mEState->mStackFrame.mLValSize - vmPtr->mExecStack[vmPtr->mEState->mFrameIndex - 1].mLValSize;
}

LVoid SaveLocalSize(LVoid* vm) {
    HandleTempLocalSize(kBoyiaNull, (BoyiaVM*)vm);
}

// 针对回调函数单独设计
LInt NativeCallImpl(BoyiaValue* args, LInt argc, BoyiaValue* obj, LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    ExecState* current = vmPtr->mEState;
    {
        // 第一个参数为调用该函数的函数指针
        BoyiaValue* value = &args[0];
        BOYIA_LOG("HandlePushParams functionName=%u \n", value->mValueType);

        // 因为ExecState相对较大，因而在C++堆上分配执行栈
        ExecState* state = CreateExecState(vmPtr);

        // 切换执行栈
        SwitchExecState(state, vmPtr);
        HandlePushScene(kBoyiaNull, vmPtr);
        LInt start = state->mExecStack[state->mFrameIndex - 1].mLValSize;

        // 拷贝参数到新的执行栈
        LInt idx = 0;
        for (; idx < argc; idx++) {
            ValueCopy(&state->mLocals[idx + start], &args[idx]);
        }
        state->mStackFrame.mLValSize = argc;
        
        BoyiaFunction* func = (BoyiaFunction*)value->mValue.mObj.mPtr;
        if (value->mValueType == BY_NAV_PROP) {
            LocalPush(obj, vm);
            NativePtr navFun = (NativePtr)func->mFuncBody;
            navFun(vm);
        } else {
            AssignStateClass(vmPtr->mEState, obj);
            CommandTable* cmds = (CommandTable*)func->mFuncBody;
            vmPtr->mEState->mStackFrame.mContext = cmds;
            vmPtr->mEState->mStackFrame.mPC = cmds->mBegin;
            ExecInstruction(vmPtr);
        }
        DestroyExecState(state, vmPtr);
    }
    SwitchExecState(current, vmPtr);
    return kOpResultSuccess;
}

LVoid ExecuteGlobalCode(LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;

    ResetScene(vmPtr->mEState);
    CommandTable cmds;
    for (LInt i = 0; i < vmPtr->mEntry->mSize; i++) {
        cmds.mBegin = vmPtr->mVMCode->mCode + vmPtr->mEntry->mTable[i];
        vmPtr->mEState->mStackFrame.mContext = &cmds;
        ExecuteCode(vmPtr);
    }
}

LVoid CacheVMCode(LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    CacheStringTable(vmPtr->mStrTable->mTable, vmPtr->mStrTable->mSize, vm);
    CacheSymbolTable(vm);
    CacheInstuctionEntry(vmPtr->mEntry->mTable, sizeof(LInt) * vmPtr->mEntry->mSize);

    // clear inline cache
    for (LInt i = 0; i < vmPtr->mVMCode->mSize; i++) {
        Instruction* inst = &vmPtr->mVMCode->mCode[i];
        if (inst->mCache) {
            inst->mCache = kBoyiaNull;
            FAST_DELETE(inst->mCache);
        }

        if (inst->mOPCode == kCmdCreateFunction
            && inst->mOPRight.mValue == BY_ANONYM_FUNC) {
            inst->mOPLeft = { OP_NONE, 0 };
        }

        if (inst->mOPCode == kCmdOnceJmpTrue) {
            inst->mOPLeft = { OP_CONST_NUMBER, LTrue };
        }
    }
    CacheInstuctions(vmPtr->mVMCode->mCode, sizeof(Instruction) * vmPtr->mVMCode->mSize);
    CacheDebugInfo(vm);
}

LVoid LoadStringTable(BoyiaStr* stringTable, LInt size, LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    for (LInt i = 0; i < size; i++) {
        vmPtr->mStrTable->mTable[i].mPtr = stringTable[i].mPtr;
        vmPtr->mStrTable->mTable[i].mLen = stringTable[i].mLen;
    }

    vmPtr->mStrTable->mSize = size;
}

LVoid LoadInstructions(LVoid* buffer, LInt size, LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    vmPtr->mVMCode->mSize = size / sizeof(Instruction);
    LMemcpy(vmPtr->mVMCode->mCode, buffer, size);
}

LVoid LoadEntryTable(LVoid* buffer, LInt size, LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    vmPtr->mEntry->mSize = size / sizeof(LInt);
    LMemcpy(vmPtr->mEntry->mTable, buffer, size);
}

LVoid* CreateGlobalClass(LUintPtr key, LVoid* vm) {
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    // 创建一个类对象
    return CreateFunVal(key, BY_CLASS, vmPtr);
}

// 执行异步任务完成后，需要将使用接过来创建一个微任务
LVoid* CreateMicroTask(LVoid* vmPtr, BoyiaValue* value) {
    BoyiaVM* vm = (BoyiaVM*)vmPtr;
    MicroTask* task = AllocMicroTask(vm);
    ValueCopyNoName(&task->mObjRef, value);
    task->mAsyncEs = kBoyiaNull;
    return task;
}

LVoid ResumeMicroTask(LVoid* taskPtr, BoyiaValue* value, LVoid* vmPtr) {
    BoyiaVM* vm = (BoyiaVM*)vmPtr;
    MicroTask* task = (MicroTask*)taskPtr;
    ValueCopyNoName(&task->mResult, value);
    AddMicroTask(task, vm);
}

// 消费微任务，若选择支持微任务，则宏任务结束后执行该函数
LVoid ConsumeMicroTask(LVoid* vmPtr) {
    BoyiaVM* vm = (BoyiaVM*)vmPtr;
    MicroTaskQueue* queue = vm->mTaskQueue;
    MicroTask* task = queue->mUsedTasks.mHead;
    while (task) {
        BoyiaStr* keyStr = GetStringBuffer(&task->mResult);

        ExecState* aes = task->mAsyncEs;
        if (aes && aes->mStackFrame.mPC) {
            //PrintValueKey(&aes->mLocals[1], vm);
            // 恢复await中断的程序
            ExecState* currentState = vm->mEState;
            SwitchExecState(aes, vm);
            aes->mWait = LFalse;
            aes->mStackFrame.mPC = NextInstruction(aes->mStackFrame.mPC, vm);

            BoyiaStr* keyStr = GetStringBuffer(&task->mResult);
            // await执行完成后，将结果赋值给R0虚拟寄存器
            ValueCopy(&vm->mCpu->mReg0, &task->mResult);
            // 这个过程中可能会销毁处于最外层且没有被await的ExecState
            ExecInstruction(vm);
                
            // 执行过程中可能会遇到其他Async函数，因而必须使用aes变量，而不是vm->mEState
            // 执行ExecInstruction, 如果整个异步函数执行到末尾了，则将结果输出给顶层microtask
            // 这里之所以判断mContext为空，是因为异步函数执行到末尾时会pop一次，这一次mContext会置为空
            if (!aes->mStackFrame.mContext) {
                // aes销毁时mTopTask会置为空
                if (aes->mTopTask) {
                    ValueCopy(&aes->mTopTask->mResult, &vm->mCpu->mReg0);
                    AddMicroTask(aes->mTopTask, vm);
                }
                // 销毁ExecState
                if (aes->mPrevious && aes->mPrevious->mWait) {
                    DestroyExecState(aes, vm);
                }
            }

            SwitchExecState(currentState, vm);
        }

        MicroTask* tmp = task;
        task = tmp->mNext;
        queue->mUsedTasks.mHead = task;
        // 释放任务
        FreeMicroTask(tmp, vm);
    }
}