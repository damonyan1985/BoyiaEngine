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
#define RuntimeError(key, error, vm) PrintErrorKey(key, error, vm->mEState->mPC->mCodeLine, vm)

/* Type Define Begin */
#define NUM_FUNC ((LInt)1024)
#define NUM_FUNC_PARAMS ((LInt)32)
#define NUM_GLOBAL_VARS ((LInt)512)
#define NUM_LOCAL_VARS ((LInt)512)
#define NUM_RESULT ((LInt)128)
#define FUNC_CALLS ((LInt)32)
#define NUM_PARAMS ((LInt)32)
#define LOOP_NEST ((LInt)32)
#define MEMORY_SIZE ((LInt)1024 * 1024 * 6)
#define CODE_CAPACITY ((LInt)1024 * 32) // Instruction Capacity
#define CONST_CAPACITY ((LInt)1024)
#define ENTRY_CAPACITY ((LInt)1024)

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

enum LogicValue {
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

enum MathValue {
    ADD = NE + 1,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    ASSIGN
}; // 33
// 标点符号
enum DelimiValue {
    SEMI = ASSIGN + 1,
    COMMA,
    QUOTE,
    DOT
}; // 37
// 小括号，中括号，大括号
enum BracketValue {
    LPTR = DOT + 1,
    RPTR,
    ARRAY_BEGIN,
    ARRAY_END,
    BLOCK_START,
    BLOCK_END
};
struct BoyiaVM;
typedef LInt (*OPHandler)(LVoid* instruction, BoyiaVM* vm);

enum OpType {
    OP_NONE,
    OP_CONST_NUMBER,
    OP_CONST_STRING,
    OP_REG0,
    OP_REG1,
    OP_VAR,
};

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
    kCmdReturn
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
    LInt mCodeLine;
    OpCommand mOPLeft;
    OpCommand mOPRight;
    //OPHandler mHandler;
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

typedef struct {
    LInt mLoopSize;
    LInt mFunctos;
    LInt mFunSize; /* index to top of function call stack */
    LInt mGValSize; /* count of function table */
    LInt mTmpLValSize;
    LInt mLValSize; /* count of global variable stack */
    LInt mResultNum;
    BoyiaValue* mClass;
    CommandTable* mContext;
    CommandTable* mCmds;
    Instruction* mPC; // pc , 指令计数器
} ExecState;

typedef struct {
    Instruction* mPC;
    LInt mLValSize;
    LInt mTmpLValSize;
    LInt mLoopSize;
    CommandTable* mContext;
    BoyiaValue* mClass;
} ExecScene;

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

/* Boyia VM Define
 * Member
 * 1, mPool
 * 2, Function Area
 * 3, gBoyiaVM->mGlobals
 */
typedef struct BoyiaVM {
    BoyiaFunction* mFunTable;
    BoyiaValue* mGlobals;
    BoyiaValue* mLocals;
    VMCpu* mCpu;
    ExecState* mEState;
    ExecScene* mExecStack;
    LIntPtr* mLoopStack;
    BoyiaValue* mOpStack;
    VMCode* mVMCode;
    VMStrTable* mStrTable;
    VMEntryTable* mEntry;
    OPHandler* mHandlers;
    LVoid* mCreator; // 此处传入创建vm的外部对象
} BoyiaVM;

typedef struct {
    LInt8* mProg;
    LInt mLineNum; // Current Line num of source code
    BoyiaToken mToken;
    BoyiaVM* mVm;
} CompileState;

/* Global value define end */
static LVoid LocalStatement(CompileState* cs);

static LVoid IfStatement(CompileState* cs);

static LVoid PushArgStatement(CompileState* cs);

static LVoid WhileStatement(CompileState* cs);

static LVoid DoStatement(CompileState* cs);

static LVoid BlockStatement(CompileState* cs);

static LVoid FunStatement(CompileState* cs);

static LInt NextToken(CompileState* cs);

static LVoid EvalExpression(CompileState* cs);

static LVoid EvalAssignment(CompileState* cs);

static BoyiaValue* FindVal(LUintPtr key, BoyiaVM* vm);

static BoyiaValue* FindObjProp(BoyiaValue* lVal, LUintPtr rVal, Instruction* inst, BoyiaVM* vm);

// Handler Declarations Begin
static LInt HandleAssignment(LVoid* ins, BoyiaVM* vm);

static LInt HandleJumpToIfTrue(LVoid* ins, BoyiaVM* vm);

static LInt HandleIfEnd(LVoid* ins, BoyiaVM* vm);

static LInt HandlePushObj(LVoid* ins, BoyiaVM* vm);

static LInt HandleAdd(LVoid* ins, BoyiaVM* vm);

static LInt HandleSub(LVoid* ins, BoyiaVM* vm);

static LInt HandleMul(LVoid* ins, BoyiaVM* vm);

static LInt HandleDiv(LVoid* ins, BoyiaVM* vm);

static LInt HandleMod(LVoid* ins, BoyiaVM* vm);

static LInt HandleLogic(LVoid* ins, BoyiaVM* vm);

static LInt HandleRelational(LVoid* ins, BoyiaVM* vm);

static LInt HandlePush(LVoid* ins, BoyiaVM* vm);

static LInt HandlePop(LVoid* ins, BoyiaVM* vm);

static LInt HandlePushScene(LVoid* ins, BoyiaVM* vm);

static LInt HandlePopScene(LVoid* ins, BoyiaVM* vm);

static LInt HandlePushArg(LVoid* ins, BoyiaVM* vm);

static LInt HandleTempLocalSize(LVoid* ins, BoyiaVM* vm);

static LInt HandlePushParams(LVoid* ins, BoyiaVM* vm);

static LInt HandleCallFunction(LVoid* ins, BoyiaVM* vm);

static LInt HandleCallInternal(LVoid* ins, BoyiaVM* vm);

static LInt HandleLoopIfTrue(LVoid* ins, BoyiaVM* vm);

static LInt HandleJumpTo(LVoid* ins, BoyiaVM* vm);

static LInt HandleGetProp(LVoid* ins, BoyiaVM* vm);

static LInt HandleConstString(LVoid* ins, BoyiaVM* vm);

static LInt HandleAssignment(LVoid* ins, BoyiaVM* vm);

static LInt HandleAssignVar(LVoid* ins, BoyiaVM* vm);

static LInt HandleLoopBegin(LVoid* ins, BoyiaVM* vm);

static LInt HandleCreateClass(LVoid* ins, BoyiaVM* vm);

static LInt HandleExtend(LVoid* ins, BoyiaVM* vm);

static LInt HandleDeclGlobal(LVoid* ins, BoyiaVM* vm);

static LInt HandleDeclLocal(LVoid* ins, BoyiaVM* vm);

static LInt HandleFunCreate(LVoid* ins, BoyiaVM* vm);

static LInt HandleCreateExecutor(LVoid* ins, BoyiaVM* vm);

static LInt HandleCreateParam(LVoid* ins, BoyiaVM* vm);

static LInt HandleReturn(LVoid* ins, BoyiaVM* vm);

static LInt HandleBreak(LVoid* ins, BoyiaVM* vm);

static LInt HandleCreateProp(LVoid* ins, BoyiaVM* vm);
// Handler Declarations End

// Reset scene of global execute state
static LVoid ResetScene(BoyiaVM* vm)
{
    vm->mEState->mLValSize = 0; /* Initialize local variable stack index */
    vm->mEState->mFunctos = 0; /* Initialize the call stack index */
    vm->mEState->mLoopSize = 0;
    vm->mEState->mResultNum = 0;
    vm->mEState->mTmpLValSize = 0;
    vm->mEState->mClass = kBoyiaNull;
}

static VMCode* CreateVMCode(LVoid* vm)
{
    VMCode* code = NEW(VMCode, vm);
    code->mCode = NEW_ARRAY(Instruction, CODE_CAPACITY, vm); //new Instruction[CODE_CAPACITY];//
    code->mSize = 0;
    return code;
}

static OPHandler* InitHandlers(LVoid* vm)
{
    OPHandler* handlers = NEW_ARRAY(OPHandler, 100, vm);
    LMemset(handlers, 0, sizeof(OPHandler) * 100);
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

    return handlers;
}

static VMStrTable* CreateVMStringTable(LVoid* vm)
{
    VMStrTable* table = NEW(VMStrTable, vm);
    table->mSize = 0;
    return table;
}

static VMEntryTable* CreateVMEntryTable(LVoid* vm)
{
    VMEntryTable* table = NEW(VMEntryTable, vm);
    table->mSize = 0;
    return table;
}

LVoid* InitVM(LVoid* creator)
{
    BoyiaVM* vm = FAST_NEW(BoyiaVM);
    vm->mCreator = creator;
    /* 一个页面只允许最多NUM_GLOBAL_VARS个函数 */
    vm->mGlobals = NEW_ARRAY(BoyiaValue, NUM_GLOBAL_VARS, vm);
    vm->mLocals = NEW_ARRAY(BoyiaValue, NUM_LOCAL_VARS, vm);
    vm->mFunTable = NEW_ARRAY(BoyiaFunction, NUM_FUNC, vm);

    vm->mOpStack = NEW_ARRAY(BoyiaValue, NUM_RESULT, vm);

    vm->mExecStack = NEW_ARRAY(ExecScene, FUNC_CALLS, vm);
    vm->mLoopStack = NEW_ARRAY(LIntPtr, LOOP_NEST, vm);
    vm->mEState = NEW(ExecState, vm);
    vm->mCpu = NEW(VMCpu, vm);
    vm->mVMCode = CreateVMCode(vm);
    vm->mHandlers = InitHandlers(vm);
    vm->mStrTable = CreateVMStringTable(vm);
    vm->mEntry = CreateVMEntryTable(vm);

    vm->mEState->mGValSize = 0;
    vm->mEState->mFunSize = 0;

    ResetScene(vm);
    return vm;
}

LVoid* GetVMCreator(LVoid* vm)
{
    return ((BoyiaVM*)vm)->mCreator;
}

LVoid DestroyVM(LVoid* vm)
{
    FAST_DELETE(vm);
}

static Instruction* AllocateInstruction(BoyiaVM* vm)
{
    BOYIA_LOG("AllocateInstruction size=%d", vm->mVMCode->mSize);
    return vm->mVMCode->mCode + vm->mVMCode->mSize++;
}

static Instruction* PutInstruction(
    OpCommand* left,
    OpCommand* right,
    LUint8 op,
    OPHandler handler,
    CompileState* cs)
{
    Instruction* newIns = AllocateInstruction(cs->mVm);
    // Init member
    newIns->mOPLeft.mType = 0;
    newIns->mOPLeft.mValue = 0;
    newIns->mOPRight.mType = 0;
    newIns->mOPRight.mValue = 0;

    if (left) {
        newIns->mOPLeft.mType = left->mType;
        newIns->mOPLeft.mValue = left->mValue;
    }

    if (right) {
        newIns->mOPRight.mType = right->mType;
        newIns->mOPRight.mValue = right->mValue;
    }

    newIns->mCodeLine = cs->mLineNum;
    newIns->mOPCode = op;
    //newIns->mHandler = kBoyiaNull;
    newIns->mNext = kInvalidInstruction;
    newIns->mCache = kBoyiaNull;
    Instruction* ins = cs->mVm->mEState->mContext->mEnd;
    if (!ins) {
        cs->mVm->mEState->mContext->mBegin = newIns;
    } else {
        ins->mNext = cs->mVm->mVMCode->mSize - 1;
    }

    cs->mVm->mEState->mContext->mEnd = newIns;
    return newIns;
}

static LInt HandlePopScene(LVoid* ins, BoyiaVM* vm)
{
    if (vm->mEState->mFunctos > 0) {
        vm->mEState->mLValSize = vm->mExecStack[--vm->mEState->mFunctos].mLValSize;
        vm->mEState->mPC = vm->mExecStack[vm->mEState->mFunctos].mPC;
        vm->mEState->mContext = vm->mExecStack[vm->mEState->mFunctos].mContext;
        vm->mEState->mLoopSize = vm->mExecStack[vm->mEState->mFunctos].mLoopSize;
        vm->mEState->mClass = vm->mExecStack[vm->mEState->mFunctos].mClass;
        vm->mEState->mTmpLValSize = vm->mExecStack[vm->mEState->mFunctos].mTmpLValSize;
    }

    return 1;
}

static Instruction* NextInstruction(Instruction* instruction, BoyiaVM* vm)
{
    if (instruction->mNext == kInvalidInstruction) {
        return kBoyiaNull;
    }

    return vm->mVMCode->mCode + instruction->mNext;
}

static LVoid ExecPopFunction(BoyiaVM* vm)
{
    // 指令为空，则判断是否处于函数范围中，是则pop，
    // 从而取得调用之前的运行环境, 即之前指向的pc指令
    // 如果不为空，就获取下一条指令，递归调用本函数
    if (!vm->mEState->mPC && vm->mEState->mFunctos > 0) {
        HandlePopScene(kBoyiaNull, vm);
        if (vm->mEState->mPC) {
            vm->mEState->mPC = NextInstruction(vm->mEState->mPC, vm); // vm->mEState->mPC->mNext;
            ExecPopFunction(vm);
        }
    }
}

static LVoid ExecInstruction(BoyiaVM* vm)
{
    // 通过指令寄存器进行计算
    ExecState* es = vm->mEState;
    if (!es->mPC) {
        ExecPopFunction(vm);
        return;
    }

    while (es->mPC) {
        OPHandler handler = vm->mHandlers[es->mPC->mOPCode];
        if (handler) {
            LInt result = handler(es->mPC, vm);
            if (result == 0) { // 指令运行出错跳出循环
                break;
            } else if (es->mPC && result == 2) { // 函数跳转
                continue;
            } // 指令计算结果为1即为正常情况
        }

        if (es->mPC) {
            es->mPC = NextInstruction(es->mPC, vm); //es->mPC->mNext;
        }

        ExecPopFunction(vm);
    }
}

static LVoid Putback(CompileState* cs)
{
    cs->mProg -= cs->mToken.mTokenName.mLen;
}

static LUint8 LookUp(BoyiaStr* name)
{
    register LInt i = 0;
    for (; gKeywordTable[i].mName.mLen; ++i) {
        if (MStrcmp(&gKeywordTable[i].mName, name))
            return gKeywordTable[i].mType;
    }

    return 0;
}

static BoyiaFunction* CopyFunction(BoyiaValue* clsVal, LInt count, BoyiaVM* vm)
{
    BoyiaFunction* newFunc = NEW(BoyiaFunction, vm);
    // copy function
    BOYIA_LOG("HandleCallInternal CreateObject %d", 5);

    BoyiaFunction* func = (BoyiaFunction*)clsVal->mValue.mObj.mPtr;
    BOYIA_LOG("HandleCallInternal CreateObject %d", 6);
    newFunc->mParams = NEW_ARRAY(BoyiaValue, NUM_FUNC_PARAMS, vm);
    //EngineLog("HandleCallInternal CreateObject %d", 7);
    newFunc->mParamSize = 0;
    newFunc->mFuncBody = func->mFuncBody;
    newFunc->mParamCount = count;
    //EngineLog("HandleCallInternal CreateObject %d", 7);
    //EngineLog("HandleCallInternal CreateObject mParams mAddr %d",  (int)newFunc->mParams);

    while (clsVal) {
        BoyiaFunction* func = (BoyiaFunction*)clsVal->mValue.mObj.mPtr;
        LInt idx = func->mParamSize;
        while (idx--) {
            if (func->mParams[idx].mValueType != BY_FUNC) {
                ValueCopy(newFunc->mParams + newFunc->mParamSize++, func->mParams + idx);
            }
        }

        clsVal = (BoyiaValue*)clsVal->mValue.mObj.mSuper;
    }

    return newFunc;
}

LInt CreateObject(LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    BOYIA_LOG("HandleCallInternal CreateObject %d", 1);
    BoyiaValue* value = (BoyiaValue*)GetLocalValue(0, vm);
    if (!value || value->mValueType != BY_CLASS) {
        return 0;
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
    return 1;
}

LVoid ValueCopyNoName(BoyiaValue* dest, BoyiaValue* src)
{
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
    case BY_CLASS: {
        //dest->mValue.mIntVal = src->mValue.mIntVal;
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

LVoid ValueCopy(BoyiaValue* dest, BoyiaValue* src)
{
    dest->mNameKey = src->mNameKey;
    ValueCopyNoName(dest, src);
}

static LInt HandleBreak(LVoid* ins, BoyiaVM* vm)
{
    BOYIA_LOG("HandleBreak mLoopSize=%d \n", vm->mEState->mLoopSize);
    vm->mEState->mPC = (Instruction*)vm->mLoopStack[--vm->mEState->mLoopSize];
    //EngineLog("HandleBreak end=%d \n", gBoyiaVM->mEState->mPC->mNext->mOPCode);
    return 1;
}

static LVoid BreakStatement(CompileState* cs)
{
    BOYIA_LOG("BreakStatement inst code=%d \n", 1);
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdBreak, HandleBreak, cs);
}

static LInt HandleCreateProp(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaFunction* func = (BoyiaFunction*)vm->mEState->mClass->mValue.mObj.mPtr;
    func->mParams[func->mParamSize].mNameKey = (LUintPtr)inst->mOPLeft.mValue;
    func->mParams[func->mParamSize].mValue.mIntVal = 0;
    func->mParamSize++;
    return 1;
}

static LVoid PropStatement(CompileState* cs)
{
    NextToken(cs);
    //EngineStrLog("PropStatement name=%s", cs->mToken.mTokenName);
    if (cs->mToken.mTokenType == IDENTIFIER) {
        OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };
        PutInstruction(&cmd, kBoyiaNull, kCmdPropCreate, HandleCreateProp, cs);
        Putback(cs);
        EvalExpression(cs);

        if (cs->mToken.mTokenValue != SEMI) {
            SntxErrorBuild(SEMI_EXPECTED, cs);
        }
    } else {
        SntxErrorBuild(SYNTAX, cs);
    }
}

LVoid LocalPush(BoyiaValue* value, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    if (vmPtr->mEState->mLValSize > NUM_LOCAL_VARS) {
        SntxError(TOO_MANY_LVARS, vmPtr->mEState->mPC->mCodeLine);
    }

    ValueCopy(vmPtr->mLocals + (vmPtr->mEState->mLValSize++), value);
}

static BoyiaValue* FindGlobal(LUintPtr key, BoyiaVM* vm)
{
    for (LInt idx = 0; idx < vm->mEState->mGValSize; ++idx) {
        if (vm->mGlobals[idx].mNameKey == key)
            return &vm->mGlobals[idx];
    }

    return kBoyiaNull;
}

/* Find the value of a variable. */
static BoyiaValue* GetVal(LUintPtr key, BoyiaVM* vm)
{
    /* First, see if has obj scope */
    if (key == kBoyiaThis) {
        return vm->mEState->mClass;
    }

    if (key == kBoyiaSuper) {
        return vm->mEState->mClass ? (BoyiaValue*)vm->mEState->mClass->mValue.mObj.mSuper : kBoyiaNull;
    }

    /* second, see if it's a local variable */
    if (vm->mEState->mFunctos > 0) {
        LInt start = vm->mExecStack[vm->mEState->mFunctos - 1].mLValSize;
        LInt idx = vm->mEState->mLValSize - 1;
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

    return FindObjProp(vm->mEState->mClass, key, kBoyiaNull, vm);
}

static BoyiaValue* FindVal(LUintPtr key, BoyiaVM* vm)
{
    BoyiaValue* value = GetVal(key, vm);
    if (!value) {
        RuntimeError(key, NOT_VAR, vm);
    }

    return value;
}

static BoyiaValue* GetOpValue(Instruction* inst, LInt8 type, BoyiaVM* vm)
{
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

static LInt HandleCallInternal(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;

    LInt idx = inst->mOPLeft.mValue;
    BOYIA_LOG("HandleCallInternal Exec idx=%d", idx);
    //return (*gNativeFunTable[idx].mAddr)(vm);
    return CallNativeFunction(idx, vm);
}

static LInt HandleTempLocalSize(LVoid* ins, BoyiaVM* vm)
{
    vm->mExecStack[vm->mEState->mFunctos].mTmpLValSize = vm->mEState->mTmpLValSize;
    vm->mEState->mTmpLValSize = vm->mEState->mLValSize;
    return 1;
}

static LInt HandlePushScene(LVoid* ins, BoyiaVM* vm)
{
    if (vm->mEState->mFunctos >= FUNC_CALLS) {
        SntxError(NEST_FUNC, vm->mEState->mPC->mCodeLine);
        return 0;
    }

    Instruction* inst = (Instruction*)ins;
    vm->mExecStack[vm->mEState->mFunctos].mLValSize = vm->mEState->mTmpLValSize;
    vm->mExecStack[vm->mEState->mFunctos].mPC = (Instruction*)(inst + inst->mOPLeft.mValue);
    vm->mExecStack[vm->mEState->mFunctos].mContext = vm->mEState->mContext;
    vm->mExecStack[vm->mEState->mFunctos++].mLoopSize = vm->mEState->mLoopSize;

    return 1;
}

static LInt HandlePushArg(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* value = GetOpValue(inst, OpLeft, vm);
    if (value) {
        LocalPush(value, vm);
        return 1;
    }

    return 0;
}

static LInt HandlePushObj(LVoid* ins, BoyiaVM* vm)
{
    vm->mExecStack[vm->mEState->mFunctos].mClass = vm->mEState->mClass;
    Instruction* inst = (Instruction*)ins;

    if (inst->mOPLeft.mType == OP_VAR) {
        LUintPtr objKey = (LUintPtr)inst->mOPLeft.mValue;
        if (objKey != kBoyiaSuper) {
            vm->mEState->mClass = (BoyiaValue*)vm->mCpu->mReg0.mNameKey;
        }
    } else {
        vm->mEState->mClass = kBoyiaNull;
    }

    return 1;
}

static LVoid ElseStatement(CompileState* cs)
{
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdElse, kBoyiaNull, cs);
    BlockStatement(cs);
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdElEnd, kBoyiaNull, cs);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst); // 最后地址值
}

static LInt HandleReturn(LVoid* ins, BoyiaVM* vm)
{
    vm->mEState->mPC = vm->mEState->mContext->mEnd;
    return 1;
}

static LVoid ReturnStatement(CompileState* cs)
{
    EvalExpression(cs); // => R0
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdReturn, HandleReturn, cs);
}

static LVoid ForStatement(CompileState* cs)
{
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
    Instruction* beginInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoop, HandleLoopBegin, cs);

    // Second expression
    /* check the conditional expression => R0 */
    EvalExpression(cs);

    // If true, execute the block
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoopTrue, HandleLoopIfTrue, cs);

    // Third expression
    EvalExpression(cs);
    if (cs->mToken.mTokenValue != RPTR) {
        SntxErrorBuild(RPTR_EXPECTED, cs);
    }
    // i++ execute finished, then jmp to begin
    Instruction* lastInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdJmpTo, HandleJumpTo, cs);
    lastInst->mOPLeft.mType = OP_CONST_NUMBER;
    lastInst->mOPLeft.mValue = (LIntPtr)(lastInst - beginInst);

    logicInst->mOPLeft.mType = OP_CONST_NUMBER;
    logicInst->mOPLeft.mValue = (LIntPtr)(lastInst - logicInst);

    BlockStatement(cs);
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdJmpTo, HandleJumpTo, cs);
    beginInst->mOPLeft.mType = OP_CONST_NUMBER;
    beginInst->mOPLeft.mValue = (LIntPtr)(endInst - beginInst);

    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst);

    // execute i++
    endInst->mOPLeft.mType = OP_CONST_NUMBER;
    endInst->mOPLeft.mValue = (LIntPtr)(endInst - logicInst);
}

static LVoid BlockStatement(CompileState* cs)
{
    LBool block = LFalse;
    do {
        NextToken(cs);
        //EngineStrLog("BlockStatement name=%s", cs->mToken.mTokenName);
        /* If interpreting single statement, return on first semicolon. */
        /* see what kind of token is up */
        if (cs->mToken.mTokenType == IDENTIFIER) {
            /* Not a keyword, so process expression. */
            Putback(cs); /* restore token to input stream for further processing by EvalExpression() */
            //EngineStrLog("token name=%s", cs->mToken.mTokenName);
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
                LocalStatement(cs);
                break;
            case BY_FUNC:
                FunStatement(cs);
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
            }
        }
    } while (cs->mToken.mTokenValue != BY_END && block);
}

static LVoid SkipComment(CompileState* cs)
{
    if (*cs->mProg == '/') {
        if (*(cs->mProg + 1) == '*') { // 多行注释
            cs->mProg += 2;
            do {
                while (*cs->mProg != '*') {
                    if (*cs->mProg == '\n') {
                        ++cs->mLineNum;
                    }
                    ++cs->mProg;
                }
                ++cs->mProg;
            } while (*cs->mProg != '/');
            ++cs->mProg;
            SkipComment(cs);
        } else if (*(cs->mProg + 1) == '/') { //单行注释
            while (*cs->mProg && *cs->mProg != '\n') {
                ++cs->mProg;
            }

            if (*cs->mProg == '\n') {
                ++cs->mLineNum;
                ++cs->mProg;
            }

            SkipComment(cs);
        }
    }
}

static LInt GetLogicValue(CompileState* cs)
{
    if (MStrchr("&|!<>=", *cs->mProg)) {
        LInt len = 0;
        cs->mToken.mTokenName.mPtr = cs->mProg;
        switch (*cs->mProg) {
        case '=':
            if (*(cs->mProg + 1) == '=') {
                cs->mProg += 2;
                len += 2;
                cs->mToken.mTokenValue = EQ;
            }
            break;
        case '!':
            if (*(cs->mProg + 1) == '=') {
                cs->mProg += 2;
                len += 2;
                cs->mToken.mTokenValue = NE;
            } else {
                ++cs->mProg;
                len = 1;
                cs->mToken.mTokenValue = NOT;
            }
            break;
        case '<':
            if (*(cs->mProg + 1) == '=') {
                cs->mProg += 2;
                len += 2;
                cs->mToken.mTokenValue = LE;
            } else {
                ++cs->mProg;
                len = 1;
                cs->mToken.mTokenValue = LT;
            }
            break;
        case '>':
            if (*(cs->mProg + 1) == '=') {
                cs->mProg += 2;
                len += 2;
                cs->mToken.mTokenValue = GE;
            } else {
                cs->mProg++;
                len = 1;
                cs->mToken.mTokenValue = GT;
            }
            break;
        case '&':
            if (*(cs->mProg + 1) == '&') {
                cs->mProg += 2;
                len += 2;
                cs->mToken.mTokenValue = AND;
            }
            break;
        case '|':
            if (*(cs->mProg + 1) == '|') {
                cs->mProg += 2;
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

static LInt GetDelimiter(CompileState* cs)
{
    const char* delimiConst = "+-*/%^=;,'.()[]{}";
    LInt op = ADD;
    do {
        if (*delimiConst == *cs->mProg) {
            cs->mToken.mTokenValue = op;
            cs->mToken.mTokenName.mPtr = cs->mProg;
            cs->mToken.mTokenName.mLen = 1;
            ++cs->mProg;
            return (cs->mToken.mTokenType = DELIMITER);
        }
        ++op;
        ++delimiConst;
    } while (*delimiConst);

    return 0;
}

static LInt GetIdentifer(CompileState* cs)
{
    LInt len = 0;
    if (LIsAlpha(*cs->mProg)) {
        cs->mToken.mTokenName.mPtr = cs->mProg;
        while (*cs->mProg == '_' || LIsAlpha(*cs->mProg) || LIsDigit(*cs->mProg)) {
            ++len;
            ++cs->mProg;
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

static LInt GetStringValue(CompileState* cs)
{
    // string
    LInt len = 0;
    if (*cs->mProg == '"') {
        ++cs->mProg;
        cs->mToken.mTokenName.mPtr = cs->mProg;

        while (*cs->mProg != '"' && *cs->mProg != '\r') {
            ++len;
            ++cs->mProg;
        }

        if (*cs->mProg == '\r') {
            SntxErrorBuild(SYNTAX, cs);
        }

        ++cs->mProg;
        // +2 for putback just in case
        cs->mToken.mTokenName.mLen = len + 2;
        return (cs->mToken.mTokenType = STRING_VALUE);
    }

    return 0;
}

static LInt GetNumberValue(CompileState* cs)
{
    LInt len = 0;
    if (LIsDigit(*cs->mProg)) {
        cs->mToken.mTokenName.mPtr = cs->mProg;

        while (LIsDigit(*cs->mProg)) {
            ++len;
            ++cs->mProg;
        }

        cs->mToken.mTokenName.mLen = len;
        return (cs->mToken.mTokenType = NUMBER);
    }

    return 0;
}

static LInt NextToken(CompileState* cs)
{
    cs->mToken.mTokenType = 0;
    cs->mToken.mTokenValue = 0;
    InitStr(&cs->mToken.mTokenName, kBoyiaNull);
    while (LIsSpace(*cs->mProg) && *cs->mProg) {
        if (*cs->mProg == '\n') {
            ++cs->mLineNum;
        }
        ++cs->mProg;
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
        || GetDelimiter(cs)
        || GetStringValue(cs)
        || GetNumberValue(cs)) {
        return cs->mToken.mTokenType;
    }

    return 0;
}

static LInt HandleCreateParam(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;

    BoyiaFunction* function = &vm->mFunTable[vm->mEState->mFunSize - 1];
    BoyiaValue* value = &function->mParams[function->mParamSize++];
    value->mNameKey = hashKey;
    return 1;
}

static LVoid InitParams(CompileState* cs)
{
    do {
        NextToken(cs); // 得到属性名
        // 如果是右括号，则跳出循环
        if (cs->mToken.mTokenValue == RPTR) {
            break;
        }

        OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };
        PutInstruction(&cmd, kBoyiaNull, kCmdParamCreate, HandleCreateParam, cs);
        NextToken(cs); // 获取逗号分隔符','
    } while (cs->mToken.mTokenValue == COMMA);
    if (cs->mToken.mTokenValue != RPTR)
        SntxErrorBuild(PAREN_EXPECTED, cs);
}

static CommandTable* CreateExecutor(CompileState* cs)
{
    CommandTable* newTable = NEW(CommandTable, cs->mVm);
    newTable->mBegin = kBoyiaNull;
    newTable->mEnd = kBoyiaNull;
    return newTable;
}

static LVoid InitFunction(BoyiaFunction* fun, BoyiaVM* vm)
{
    fun->mParamSize = 0;
    fun->mParams = NEW_ARRAY(BoyiaValue, NUM_FUNC_PARAMS, vm);
    fun->mParamCount = NUM_FUNC_PARAMS;
    ++vm->mEState->mFunSize;
}

static BoyiaValue* CreateFunVal(LUintPtr hashKey, LUint8 type, BoyiaVM* vm)
{
    // 初始化class类或函数变量
    BoyiaValue* val = &vm->mGlobals[vm->mEState->mGValSize++];
    BoyiaFunction* fun = &vm->mFunTable[vm->mEState->mFunSize];
    val->mValueType = type;
    val->mNameKey = hashKey;
    val->mValue.mObj.mPtr = (LIntPtr)fun;
    val->mValue.mObj.mSuper = 0;
    if (type == BY_CLASS) {
        fun->mFuncBody = (LIntPtr)val;
    }
    // 初始化类属性成员列表
    InitFunction(fun, vm);
    return val;
}

static LInt HandleCreateExecutor(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;

    CommandTable* newTable = NEW(CommandTable, vm);
    if (inst->mOPLeft.mValue != -1) {
        newTable->mBegin = &vm->mVMCode->mCode[inst->mOPLeft.mValue];
        newTable->mEnd = &vm->mVMCode->mCode[inst->mOPRight.mValue];
    } else {
        newTable->mBegin = kBoyiaNull;
        newTable->mEnd = kBoyiaNull;
    }

    vm->mFunTable[vm->mEState->mFunSize - 1].mFuncBody = (LIntPtr)newTable;
    return 1;
}

static LVoid BodyStatement(CompileState* cs, LBool isFunction)
{
    CommandTable* cmds = cs->mVm->mEState->mContext;

    CommandTable tmpTable;
    tmpTable.mBegin = kBoyiaNull;
    tmpTable.mEnd = kBoyiaNull;

    Instruction* funInst = kBoyiaNull;
    if (isFunction) {
        // 类成员的创建在主体上下中进行
        OpCommand cmd = { OP_CONST_NUMBER, -1 };
        funInst = PutInstruction(&cmd, &cmd, kCmdExecCreate, HandleCreateExecutor, cs);
        cs->mVm->mEState->mContext = &tmpTable;
    }

    BlockStatement(cs);
    // 拷贝tmpTable中的offset给instruction, 非空函数
    if (funInst && tmpTable.mBegin) {
        funInst->mOPLeft.mType = OP_CONST_NUMBER;
        funInst->mOPLeft.mValue = (LIntPtr)(tmpTable.mBegin - cs->mVm->mVMCode->mCode); //(LIntPtr)es->mContext->mBegin;

        funInst->mOPRight.mType = OP_CONST_NUMBER;
        funInst->mOPRight.mValue = (LIntPtr)(tmpTable.mEnd - cs->mVm->mVMCode->mCode);
    }
    cs->mVm->mEState->mContext = cmds;
}

static LInt HandleCreateClass(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    if (inst->mOPLeft.mType == OP_NONE) {
        vm->mEState->mClass = kBoyiaNull;
        return 1;
    }
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;
    vm->mEState->mClass = CreateFunVal(hashKey, BY_CLASS, vm);
    return 1;
}

static LInt HandleExtend(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* classVal = FindGlobal((LUintPtr)inst->mOPLeft.mValue, vm);
    BoyiaValue* extendVal = FindGlobal((LUintPtr)inst->mOPRight.mValue, vm);

    // set super pointer
    classVal->mValue.mObj.mSuper = (LIntPtr)extendVal;
    return 1;
}

static LVoid ClassStatement(CompileState* cs)
{
    NextToken(cs);
    LUintPtr classKey = GenIdentifier(&cs->mToken.mTokenName, cs->mVm);
    OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)classKey };
    PutInstruction(&cmd, kBoyiaNull, kCmdCreateClass, HandleCreateClass, cs);
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
        PutInstruction(&cmd, &extendCmd, kCmdClassExtend, HandleExtend, cs);
    }

    // 执行完后需将CLASS置为kBoyiaNull
    OpCommand cmdEnd = { OP_NONE, 0 };
    PutInstruction(&cmdEnd, kBoyiaNull, kCmdCreateClass, HandleCreateClass, cs);
}

static LInt HandleFunCreate(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;

    if (vm->mEState->mClass) {
        BoyiaFunction* func = (BoyiaFunction*)vm->mEState->mClass->mValue.mObj.mPtr;
        func->mParams[func->mParamSize].mNameKey = hashKey;
        func->mParams[func->mParamSize].mValueType = BY_FUNC;
        func->mParams[func->mParamSize++].mValue.mObj.mPtr = (LIntPtr)&vm->mFunTable[vm->mEState->mFunSize];
        // 初始化函数参数列表
        InitFunction(&vm->mFunTable[vm->mEState->mFunSize], vm);
    } else {
        CreateFunVal(hashKey, BY_FUNC, vm);
    }

    return 1;
}

static LVoid FunStatement(CompileState* cs)
{
    NextToken(cs);
    //EngineStrLog("HandlePushParams FunStatement name %s", cs->mToken.mTokenName);
    // 第一步，Function变量
    OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };
    PutInstruction(&cmd, kBoyiaNull, kCmdCreateFunction, HandleFunCreate, cs);
    //EngineStrLog("FunctionName=%s", cs->mToken.mTokenName);
    // 第二步，初始化函数参数
    NextToken(cs); //   '(', 即LPTR
    InitParams(cs); //  初始化参数
    // 第三步，函数体内部编译
    BodyStatement(cs, LTrue);
}

/*
static LVoid DeleteExecutor(CommandTable* table)
{
    Instruction* pc = table->mBegin;
    while (pc != table->mEnd) {
        Instruction* next = NextInstruction(pc);//pc->mNext;
        VM_DELETE(pc);
        pc = next;
    }

    VM_DELETE(table);
}
*/
// 执行全局的调用
static LVoid ExecuteCode(BoyiaVM* vm)
{
    //vm->mEState->mContext = vm->mEState->mContext;
    vm->mEState->mPC = vm->mEState->mContext->mBegin;
    ExecInstruction(vm);
    // 删除执行体
    //DeleteExecutor(cmds);
    ResetScene(vm);
}

static LInt HandleDeclGlobal(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue val;
    val.mValueType = inst->mOPLeft.mValue;
    val.mNameKey = (LUintPtr)inst->mOPRight.mValue;
    ValueCopy(vm->mGlobals + vm->mEState->mGValSize++, &val);
    return 1;
}

static LVoid GlobalStatement(CompileState* cs)
{
    LInt type = cs->mToken.mTokenValue;
    do {
        NextToken(cs); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };

        PutInstruction(&cmdLeft, &cmdRight, kCmdDeclGlobal, HandleDeclGlobal, cs);
        Putback(cs);
        EvalExpression(cs);
    } while (cs->mToken.mTokenValue == COMMA);

    if (cs->mToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED, cs);
    }
}

static LVoid AppendEntry(BoyiaVM* vm)
{
    CommandTable* table = vm->mEState->mContext;
    // Entry的begin为空，证明没有指令
    if (!table || !table->mBegin) {
        return;
    }

    // Entry起始位置获取在VMCode中的索引
    vm->mEntry->mTable[vm->mEntry->mSize++] = table->mBegin - vm->mVMCode->mCode;
}

// 该函数记录全局变量以及函数接口
static LVoid ParseStatement(CompileState* cs)
{
    LInt brace = 0; // ‘{’的个数
    cs->mVm->mEState->mContext = CreateExecutor(cs);
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
            FunStatement(cs);
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

    AppendEntry(cs->mVm);
    // 执行全局声明和定义
    ExecuteCode(cs->mVm);
}

static LInt HandleDeclLocal(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue local;
    local.mValueType = inst->mOPLeft.mValue;
    local.mNameKey = (LUintPtr)inst->mOPRight.mValue;
    LocalPush(&local, vm); // 塞入本地符号表
    return 1;
}

/* Declare a local variable. */
static LVoid LocalStatement(CompileState* cs)
{
    LInt type = cs->mToken.mTokenValue;
    do {
        NextToken(cs); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName, cs->mVm) };
        //EngineStrLog("value Name=%s", cs->mToken.mTokenName);
        PutInstruction(&cmdLeft, &cmdRight, kCmdDeclLocal, HandleDeclLocal, cs);
        Putback(cs);
        EvalExpression(cs);
    } while (cs->mToken.mTokenValue == COMMA);

    if (cs->mToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED, cs);
    }
}

static LInt HandleCallFunction(LVoid* ins, BoyiaVM* vm)
{
    //EngineLog("HandleFunction begin %d \n", 1);
    // localstack第一个值为函数指针
    LInt start = vm->mExecStack[vm->mEState->mFunctos - 1].mLValSize;
    BoyiaValue* value = &vm->mLocals[start];
    BoyiaFunction* func = (BoyiaFunction*)value->mValue.mObj.mPtr;

    vm->mEState->mContext = (CommandTable*)func->mFuncBody;
    vm->mEState->mPC = vm->mEState->mContext->mBegin;

    return 2;
}

static LInt HandlePushParams(LVoid* ins, BoyiaVM* vm)
{
    // 第一个参数为调用该函数的函数指针
    LInt start = vm->mExecStack[vm->mEState->mFunctos - 1].mLValSize;
    BoyiaValue* value = &vm->mLocals[start];
    BOYIA_LOG("HandlePushParams functionName=%u \n", value->mValueType);
    if (value->mValueType == BY_FUNC) {
        BoyiaFunction* func = (BoyiaFunction*)value->mValue.mObj.mPtr;
        if (func->mParamSize <= 0) {
            return 1;
        }
        // 从第二个参数开始，将形参key赋给实参
        LInt idx = start + 1;
        LInt end = idx + func->mParamSize;
        for (; idx < end; ++idx) {
            LUintPtr vKey = func->mParams[idx - start - 1].mNameKey;
            vm->mLocals[idx].mNameKey = vKey;
        }
    }

    return 1;
}

static LInt HandlePop(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    if (inst->mOPLeft.mType != OP_REG0 && inst->mOPLeft.mType != OP_REG1) {
        --vm->mEState->mResultNum;
        return 1;
    }
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    ValueCopy(value, vm->mOpStack + (--vm->mEState->mResultNum));
    return 1;
}

/* Call a function. */
static void CallStatement(CompileState* cs, OpCommand* objCmd)
{
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdTmpLocal, HandleTempLocalSize, cs);
    // 设置参数
    PushArgStatement(cs);
    // POP CLASS context
    if (objCmd->mType == OP_VAR) {
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPop, HandlePop, cs);
    }
    // 保存对象环境
    PutInstruction(objCmd, kBoyiaNull, kCmdPushObj, HandlePushObj, cs);
    // 保存调用堆栈
    Instruction* pushInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushScene, HandlePushScene, cs);
    // 函数形参名哈希值赋给局部变量
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushParams, HandlePushParams, cs);
    // 执行函数体
    Instruction* funInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdCallFunction, HandleCallFunction, cs);
    //EngineLog("CallStatement=>%d HandleFunction", 1);
    pushInst->mOPLeft.mType = OP_CONST_NUMBER;
    pushInst->mOPLeft.mValue = (LIntPtr)(funInst - pushInst);
}

/* Push the arguments to a function onto the local variable stack. */
static LVoid PushArgStatement(CompileState* cs)
{
    // push函数指针
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, HandlePushArg, cs);
    NextToken(cs); // if token == ')' exit
    if (cs->mToken.mTokenValue == RPTR) {
        return;
    }
    Putback(cs);

    do {
        // 参数值在R0中
        EvalExpression(cs); // => R0
        // 将函数实参压栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, HandlePushArg, cs);
        //NextToken();
    } while (cs->mToken.mTokenValue == COMMA);
}

/* Assign a value to a Register 0 or 1. */
static LInt HandleAssignment(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    if (!left)
        return 0;

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
            return 0;
        }

        ValueCopyNoName(left, val);
        left->mNameKey = (LUintPtr)val;
    } break;
    case OP_REG0: {
        ValueCopyNoName(left, &vm->mCpu->mReg0);
    } break;
    case OP_REG1: {
        ValueCopyNoName(left, &vm->mCpu->mReg1);
    } break;
    }

    return 1;
}

// 执行到ifend证明整个if elseif, else执行完毕，
// 无需检索是否还有elseif，或者else的逻辑判断和内部block
static LInt HandleIfEnd(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = vm->mEState->mPC;
    Instruction* tmpInst = NextInstruction(inst, vm); //inst->mNext;
    // 查看下一个是否是elseif
    BOYIA_LOG("HandleIfEnd R0=>%d \n", 1);
    while (tmpInst && (tmpInst->mOPCode == kCmdElif || tmpInst->mOPCode == kCmdElse)) {
        inst = (Instruction*)(tmpInst + tmpInst->mOPRight.mValue); // 跳转到elif对应的IFEND
        tmpInst = NextInstruction(inst, vm); //inst->mNext;
    }
    BOYIA_LOG("HandleIfEnd END R0=>%d \n", 1);
    if (inst) {
        vm->mEState->mPC = inst;
    }
    return 1;
}

static LInt HandleJumpToIfTrue(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* value = &vm->mCpu->mReg0;
    if (!value->mValue.mIntVal) {
        vm->mEState->mPC = inst + inst->mOPRight.mValue;
    }

    return 1;
}

/* Execute an if statement. */
static LVoid IfStatement(CompileState* cs)
{
    NextToken(cs);
    // token = (
    EvalExpression(cs); /* check the conditional expression => R0 */
    Instruction* logicInst = PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdJmpTrue, HandleJumpToIfTrue, cs);
    //EngineStrLog("endif last inst name=%s", cs->mToken.mTokenName);
    BlockStatement(cs); /* if true, interpret */
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdIfEnd, HandleIfEnd, cs);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    //logicInst->mOPRight.mValue = (LIntPtr)endInst; // 最后地址值
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst); // Compute offset
}

static LInt HandleLoopBegin(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    // push left => loop stack
    vm->mLoopStack[vm->mEState->mLoopSize++] = (LIntPtr)(inst + inst->mOPLeft.mValue);
    return 1;
}

static LInt HandleLoopIfTrue(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* value = &vm->mCpu->mReg0;
    //EngineLog("HandleLoopIfTrue value=%d", value->mValue.mIntVal);
    if (!value->mValue.mIntVal) {
        vm->mEState->mPC = inst + inst->mOPRight.mValue;
        vm->mEState->mLoopSize--;
        return 1;
    }

    if (inst->mOPLeft.mValue) {
        vm->mEState->mPC = inst + inst->mOPLeft.mValue;
    }

    return 1;
}

static LInt HandleJumpTo(LVoid* ins, BoyiaVM* vm)
{
    //EngineLog("HandleJumpTo %d", 1);
    Instruction* inst = (Instruction*)ins;
    if (inst->mOPLeft.mType == OP_CONST_NUMBER) {
        vm->mEState->mPC = inst - inst->mOPLeft.mValue;
    }
    //EngineLog("HandleJumpTo %d", 2);
    return 1;
}

/* Execute a while loop. */
static LVoid WhileStatement(CompileState* cs)
{
    //EngineLog("WhileStatement %d", 0);
    Instruction* beginInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoop, HandleLoopBegin, cs);
    NextToken(cs); // '('
    if (cs->mToken.mTokenValue != LPTR) {
        SntxErrorBuild(LPTR_EXPECTED, cs);
    }
    EvalExpression(cs); /* check the conditional expression => R0 */
    if (cs->mToken.mTokenValue != RPTR) {
        SntxErrorBuild(RPTR_EXPECTED, cs);
    }
    //EngineStrLog("WhileStatement last inst name=%s", cs->mToken.mTokenName);
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoopTrue, HandleLoopIfTrue, cs);
    BlockStatement(cs); /* If true, execute block */
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdJmpTo, HandleJumpTo, cs);
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
static LVoid DoStatement(CompileState* cs)
{
    Instruction* beginInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoop, HandleLoopBegin, cs);
    BlockStatement(cs); /* interpret loop */
    NextToken(cs);
    if (cs->mToken.mTokenValue != BY_WHILE) {
        SntxErrorBuild(WHILE_EXPECTED, cs);
    }
    EvalExpression(cs); /* check the loop condition */

    if (cs->mToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED, cs);
    }
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdLoopTrue, HandleLoopIfTrue, cs);
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdJmpTo, HandleJumpTo, cs);
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

static LInt HandleAdd(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);
    if (!left || !right) {
        return 0;
    }
    //EngineLog("HandleAdd left=%d \n", left->mValue.mIntVal);
    //EngineLog("HandleAdd right=%d \n", right->mValue.mIntVal);
    if (left->mValueType == BY_INT && right->mValueType == BY_INT) {
        right->mValue.mIntVal += left->mValue.mIntVal;
        BOYIA_LOG("HandleAdd result=%d", right->mValue.mIntVal);
        return 1;
    }

    if (left->mValueType == BY_STRING || right->mValueType == BY_STRING) {
        BOYIA_LOG("StringAdd Begin %d", 1);
        StringAdd(left, right, vm);
        return 1;
    }

    return 0;
}

static LInt HandleSub(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);
    if (!left || !right) {
        return 0;
    }

    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return 0;

    right->mValue.mIntVal = left->mValue.mIntVal - right->mValue.mIntVal;
    BOYIA_LOG("HandleSub R0=>%d", vm->mCpu->mReg0.mValue.mIntVal);
    return 1;
}

static LInt HandleMul(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return 0;
    }

    BOYIA_LOG("HandleMul left=%d \n", left->mValue.mIntVal);
    BOYIA_LOG("HandleMul right=%d \n", right->mValue.mIntVal);
    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return 0;

    right->mValue.mIntVal *= left->mValue.mIntVal;
    BOYIA_LOG("HandleMul result=%d \n", right->mValue.mIntVal);
    return 1;
}

static LInt HandleDiv(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return 0;
    }

    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return 0;

    if (right->mValue.mIntVal == 0)
        return 0;

    right->mValue.mIntVal = left->mValue.mIntVal / right->mValue.mIntVal;
    return 1;
}

static LInt HandleMod(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return 0;
    }

    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return 0;

    if (right->mValue.mIntVal == 0)
        return 0;

    right->mValue.mIntVal = left->mValue.mIntVal % right->mValue.mIntVal;
    return 1;
}

static LInt HandleRelational(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return 0;
    }

    BOYIA_LOG("HandleLogic left=%d \n", left->mValue.mIntVal);
    BOYIA_LOG("HandleLogic right=%d \n", right->mValue.mIntVal);

    LInt result = 0;
    switch (inst->mOPCode) {
    case kCmdNotRelation:
        result = right->mValue.mIntVal ? 0 : 1;
        break;
    case kCmdLtRelation:
        result = left->mValue.mIntVal < right->mValue.mIntVal ? 1 : 0;
        break;
    case kCmdLeRelation:
        result = left->mValue.mIntVal <= right->mValue.mIntVal ? 1 : 0;
        break;
    case kCmdGtRelation:
        result = left->mValue.mIntVal > right->mValue.mIntVal ? 1 : 0;
        break;
    case kCmdGeRelation:
        result = left->mValue.mIntVal >= right->mValue.mIntVal ? 1 : 0;
        break;
    case kCmdEqRelation:
        result = left->mValue.mIntVal == right->mValue.mIntVal ? 1 : 0;
        break;
    case kCmdNeRelation:
        result = left->mValue.mIntVal != right->mValue.mIntVal ? 1 : 0;
        break;
    }

    right->mValueType = BY_INT;
    right->mValue.mIntVal = result;
    return 1;
}

static LInt HandleLogic(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* right = GetOpValue(inst, OpRight, vm);

    if (!left || !right) {
        return 0;
    }

    LInt result = 0;
    switch (inst->mOPCode) {
    case kCmdAndLogic:
        result = left->mValue.mIntVal && right->mValue.mIntVal ? 1 : 0;
        break;
    case kCmdOrLogic:
        result = left->mValue.mIntVal || right->mValue.mIntVal ? 1 : 0;
        break;
    }

    right->mValueType = BY_INT;
    right->mValue.mIntVal = result;
    return 1;
}

/* parser lib define */
static LVoid EvalExpression(CompileState* cs)
{
    NextToken(cs);
    if (!cs->mToken.mTokenName.mLen) {
        SntxErrorBuild(NO_EXP, cs);
        return;
    }

    if (cs->mToken.mTokenValue == SEMI) {
        return;
    }

    EvalAssignment(cs);
}

static LInt HandlePush(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &vm->mCpu->mReg0 : &vm->mCpu->mReg1;
    ValueCopy(vm->mOpStack + (vm->mEState->mResultNum++), value);
    return 1;
}

static LInt HandleAssignVar(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft, vm);
    BoyiaValue* result = GetOpValue(inst, OpRight, vm);
    if (!left || !result) {
        return 0;
    }
    BoyiaValue* value = (BoyiaValue*)left->mNameKey;
    ValueCopyNoName(value, result);
    ValueCopy(&vm->mCpu->mReg0, value);
    return 1;
}

static LVoid CallNativeStatement(CompileState* cs, LInt idx)
{
    NextToken(cs);
    if (cs->mToken.mTokenValue != LPTR) // '('
        SntxErrorBuild(PAREN_EXPECTED, cs);

    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdTmpLocal, HandleTempLocalSize, cs);
    do {
        // 参数值在R0中
        EvalExpression(cs); // => R0
        // 将函数参数压栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, HandlePushArg, cs);
        // if token == ')' exit
    } while (cs->mToken.mTokenValue == COMMA);

    // 保存obj现场是必不可少的一步
    OpCommand cmdSet = { OP_CONST_NUMBER, 0 };
    PutInstruction(&cmdSet, kBoyiaNull, kCmdPushObj, HandlePushObj, cs);

    Instruction* pushInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushScene, HandlePushScene, cs);
    OpCommand cmd = { OP_CONST_NUMBER, idx };
    PutInstruction(&cmd, kBoyiaNull, kCmdCallNative, HandleCallInternal, cs);
    Instruction* popInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPopScene, HandlePopScene, cs);
    pushInst->mOPLeft.mType = OP_CONST_NUMBER;
    pushInst->mOPLeft.mValue = (LIntPtr)(popInst - pushInst);
}

static BoyiaValue* FindObjProp(BoyiaValue* lVal, LUintPtr rVal, Instruction* inst, BoyiaVM* vm)
{
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

static LVoid ValueCopyWithKey(BoyiaValue* dest, BoyiaValue* src)
{
    dest->mNameKey = (LUintPtr)src;
    ValueCopyNoName(dest, src);
}

static LInt HandleGetProp(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    // r0 -> lVal
    BoyiaValue* lVal = GetOpValue(inst, OpLeft, vm);
    if (!lVal) {
        return 0;
    }

    // fetch value from inline cache
    BoyiaValue* result = GetInlineCache(inst->mCache, lVal);
    if (result) {
        ValueCopyWithKey(&vm->mCpu->mReg0, result);
        return 1;
    }

    LUintPtr rVal = (LUintPtr)inst->mOPRight.mValue;
    result = FindObjProp(lVal, rVal, inst, vm);
    if (result) {
        // maybe function
        ValueCopyWithKey(&vm->mCpu->mReg0, result);
        return 1;
    }

    return 0;
}

// According to reg0, get reg0 obj's prop
static LVoid EvalGetProp(CompileState* cs)
{
    NextToken(cs);
    if (cs->mToken.mTokenType != IDENTIFIER) {
        return;
    }

    // Push class context for callstatement
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush, cs);
    LUintPtr propKey = GenIdentifier(&cs->mToken.mTokenName, cs->mVm);
    OpCommand cmdR = { OP_CONST_NUMBER, (LIntPtr)propKey };
    PutInstruction(&COMMAND_R0, &cmdR, kCmdGetProp, HandleGetProp, cs);

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
        PutInstruction(&cmd, kBoyiaNull, kCmdPop, HandlePop, cs);
        EvalGetProp(cs);
    } else {
        OpCommand cmd = { OP_CONST_NUMBER, 0 };
        PutInstruction(&cmd, kBoyiaNull, kCmdPop, HandlePop, cs);
    }
}

static LVoid EvalGetValue(CompileState* cs, LUintPtr objKey)
{
    OpCommand cmdR = { OP_VAR, (LIntPtr)objKey };
    PutInstruction(&COMMAND_R0, &cmdR, kCmdAssign, HandleAssignment, cs);
    if (cs->mToken.mTokenValue == DOT) {
        EvalGetProp(cs);
    }
}

static LVoid CopyStringFromToken(CompileState* cs, BoyiaStr* str)
{
    str->mLen = cs->mToken.mTokenName.mLen - 2;
    str->mPtr = NEW_ARRAY(LInt8, str->mLen, cs->mVm);
    LMemcpy(str->mPtr, cs->mToken.mTokenName.mPtr, str->mLen * sizeof(LInt8));
}

static LInt HandleConstString(LVoid* ins, BoyiaVM* vm)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaStr* constStr = &vm->mStrTable->mTable[inst->mOPLeft.mValue];

    vm->mCpu->mReg0.mValueType = BY_STRING;
    vm->mCpu->mReg0.mValue.mStrVal.mPtr = constStr->mPtr;
    vm->mCpu->mReg0.mValue.mStrVal.mLen = constStr->mLen;
    return 1;
}

static LVoid Atom(CompileState* cs)
{
    switch (cs->mToken.mTokenType) {
    case IDENTIFIER: {
        LInt idx = FindNativeFunc(GenIdentifier(&cs->mToken.mTokenName, cs->mVm), cs->mVm);
        if (idx != -1) {
            CallNativeStatement(cs, idx);
            NextToken(cs);
        } else {
            LUintPtr key = GenIdentifier(&cs->mToken.mTokenName, cs->mVm);
            NextToken(cs);
            if (cs->mToken.mTokenValue == LPTR) {
                OpCommand cmd = { OP_VAR, (LIntPtr)key };
                PutInstruction(&COMMAND_R0, &cmd, kCmdAssign, HandleAssignment, cs);
                OpCommand objCmd = { OP_CONST_NUMBER, 0 };
                CallStatement(cs, &objCmd);
                NextToken(cs);
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
        PutInstruction(&COMMAND_R0, &cmd, kCmdAssign, HandleAssignment, cs);
        NextToken(cs);
    }
        return;
    case STRING_VALUE: {
        // 从StringTable中分配常量字符串
        BoyiaStr* constStr = &cs->mVm->mStrTable->mTable[cs->mVm->mStrTable->mSize];
        CopyStringFromToken(cs, constStr);
        OpCommand lCmd = { OP_CONST_NUMBER, cs->mVm->mStrTable->mSize++ };
        //OpCommand rCmd = { OP_CONST_NUMBER, constStr.mLen };
        PutInstruction(&lCmd, kBoyiaNull, kCmdConstStr, HandleConstString, cs);
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

static LVoid EvalSubexpr(CompileState* cs)
{
    if (cs->mToken.mTokenValue == LPTR) {
        NextToken(cs);
        //EngineStrLog("EvalSubexpr %s", cs->mToken.mTokenName);
        EvalAssignment(cs);
        //EngineStrLog("EvalSubexpr %s", cs->mToken.mTokenName);
        if (cs->mToken.mTokenValue != RPTR) {
            SntxErrorBuild(PAREN_EXPECTED, cs);
        }
        NextToken(cs);
    } else {
        Atom(cs);
    }
}

// 正负数,+1,-1
static LVoid EvalMinus(CompileState* cs)
{
    LInt8 op = 0;
    if (cs->mToken.mTokenValue == ADD || cs->mToken.mTokenValue == SUB) {
        op = cs->mToken.mTokenValue;
        NextToken(cs);
    }
    EvalSubexpr(cs); // => R0
    if (op && op == SUB) { // negative must multiply -1
        OpCommand cmd = { OP_CONST_NUMBER, -1 };
        PutInstruction(&COMMAND_R1, &cmd, kCmdAssign, HandleAssignment, cs);
        PutInstruction(&COMMAND_R0, &COMMAND_R1, kCmdMul, HandleMul, cs);
    }
}

// 乘除, *,/,%
static LVoid EvalArith(CompileState* cs)
{
    LInt8 op = 0;
    EvalMinus(cs);

    while ((op = cs->mToken.mTokenValue) == MUL || op == DIV || op == MOD) { // * / %
        // PUSH R0
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush, cs);
        NextToken(cs);
        EvalMinus(cs);
        // POP R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop, cs);
        switch (op) {
        case MUL:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdMul, HandleMul, cs);
            break;
        case DIV:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdDiv, HandleDiv, cs);
            break;
        case MOD:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdMod, HandleMod, cs);
            break;
        }
    }
}

// 加减,+,-
static LVoid EvalAddSub(CompileState* cs)
{
    LInt8 op = 0;
    EvalArith(cs); // => R0
    while ((op = cs->mToken.mTokenValue) == ADD || op == SUB) {
        // PUSH
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush, cs);

        NextToken(cs);
        EvalArith(cs);
        // POP R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop, cs);
        // R0 + R1 => R0
        // R1 - R0 => R0
        PutInstruction(&COMMAND_R1, &COMMAND_R0,
            op == ADD ? kCmdAdd : kCmdSub,
            op == ADD ? HandleAdd : HandleSub, cs);
    }
}

static LVoid EvalRelationalImpl(LInt8 opToken, CompileState* cs)
{
    switch (opToken) {
    case NOT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdNotRelation, HandleRelational, cs);
        break;
    case LT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdLtRelation, HandleRelational, cs);
        break;
    case LE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdLeRelation, HandleRelational, cs);
        break;
    case GT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdGtRelation, HandleRelational, cs);
        break;
    case GE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdGeRelation, HandleRelational, cs);
        break;
    case EQ:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdEqRelation, HandleRelational, cs);
        break;
    case NE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdNeRelation, HandleRelational, cs);
        break;
    }
}

// 关系比较判断,>,<,==,!=
static LVoid EvalRelational(CompileState* cs)
{
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
            PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush, cs);
        }

        NextToken(cs); // 查找标识符或者常量
        EvalAddSub(cs); // 先执行优先级高的操作 => R0
        // pop R1
        // 上次计算的结果出栈至R1
        if (op != NOT) {
            PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop, cs);
        }

        // 计算R0 OP R1, 结果存入R0中
        //PutInstruction(&COMMAND_R1, &COMMAND_R0, op, HandleRelational);
        EvalRelationalImpl(op, cs);
    }
}

static LVoid EvalLogic(CompileState* cs)
{
    static LInt8 logicops[3] = {
        AND, OR, 0
    };

    EvalRelational(cs);
    LInt8 op = 0;
    while (MStrchr(logicops, (op = cs->mToken.mTokenValue))) {
        // 计算的结果存入栈中
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush, cs);

        NextToken(cs); // 查找标识符或者常量
        EvalRelational(cs); // 先执行优先级高的操作 => R0
        // pop R1
        // 上次计算的结果出栈至R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop, cs);

        // 计算R0 OP R1, 结果存入R0中
        PutInstruction(&COMMAND_R1, &COMMAND_R0, op == AND ? kCmdAndLogic : kCmdOrLogic, HandleLogic, cs);
    }
}

// 赋值,=
static LVoid EvalAssignment(CompileState* cs)
{
    EvalLogic(cs); // =>R0
    if (cs->mToken.mTokenValue == ASSIGN) { // '='
        // R0存入栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush, cs);
        NextToken(cs);
        EvalLogic(cs); // =>R0
        // 从栈中吐出数据到R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop, cs);
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdAssignVar, HandleAssignVar, cs);
    }
}

LVoid SetNativeResult(LVoid* result, LVoid* vm)
{
    BoyiaValue* value = (BoyiaValue*)result;
    ValueCopy(&((BoyiaVM*)vm)->mCpu->mReg0, value);
}

LVoid* CopyObject(LUintPtr hashKey, LInt size, LVoid* vm)
{
    return CopyFunction(FindGlobal(hashKey, (BoyiaVM*)vm), size, (BoyiaVM*)vm);
}

LVoid* GetNativeResult(LVoid* vm)
{
    return &((BoyiaVM*)vm)->mCpu->mReg0;
}

LVoid GetLocalStack(LInt* stack, LInt* size, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    *stack = (LIntPtr)vmPtr->mLocals;
    *size = vmPtr->mEState->mLValSize;
}

LVoid GetGlobalTable(LInt* table, LInt* size, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    *table = (LIntPtr)vmPtr->mGlobals;
    *size = vmPtr->mEState->mGValSize;
}

/*  output function */
LVoid CompileCode(LInt8* code, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    vmPtr->mEState->mTmpLValSize = 0;
    vmPtr->mEState->mResultNum = 0;
    vmPtr->mEState->mLoopSize = 0;
    vmPtr->mEState->mClass = kBoyiaNull;
    CompileState cs;
    cs.mProg = code;
    cs.mLineNum = 1;
    cs.mVm = vmPtr;
    ParseStatement(&cs); // 该函数记录全局变量以及函数接口
    ResetScene(vmPtr);
}

LVoid* GetLocalValue(LInt idx, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    LInt start = vmPtr->mExecStack[vmPtr->mEState->mFunctos - 1].mLValSize;
    return &vmPtr->mLocals[start + idx];
}

LInt GetLocalSize(LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    return vmPtr->mEState->mLValSize - vmPtr->mExecStack[vmPtr->mEState->mFunctos - 1].mLValSize;
}

LVoid CallFunction(LInt8* fun, LVoid* ret, LVoid* vm)
{
    BOYIA_LOG("callFunction=>%d \n", 1);
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    CompileState cs;
    cs.mProg = fun;
    cs.mLineNum = 1;
    cs.mVm = vmPtr;
    CommandTable* cmds = CreateExecutor(&cs);
    vmPtr->mEState->mContext = cmds;

    EvalExpression(&cs); // 解析例如func(a,b,c);
    ExecuteCode(cs.mVm);

    if (ret) {
        BoyiaValue* value = (BoyiaValue*)ret;
        ValueCopy(value, &vmPtr->mCpu->mReg0);
    }
}

LVoid SaveLocalSize(LVoid* vm)
{
    HandleTempLocalSize(kBoyiaNull, (BoyiaVM*)vm);
}

LVoid NativeCall(BoyiaValue* obj, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    vmPtr->mExecStack[vmPtr->mEState->mFunctos].mClass = vmPtr->mEState->mClass;
    vmPtr->mExecStack[vmPtr->mEState->mFunctos].mLValSize = vmPtr->mEState->mTmpLValSize;
    vmPtr->mExecStack[vmPtr->mEState->mFunctos].mPC = vmPtr->mEState->mPC;
    vmPtr->mExecStack[vmPtr->mEState->mFunctos].mContext = vmPtr->mEState->mContext;
    vmPtr->mExecStack[vmPtr->mEState->mFunctos++].mLoopSize = vmPtr->mEState->mLoopSize;
    vmPtr->mEState->mClass = obj;

    HandlePushParams(kBoyiaNull, vmPtr);
    HandleCallFunction(kBoyiaNull, vmPtr);

    ExecInstruction(vmPtr);
}

LVoid ExecuteGlobalCode(LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;

    vmPtr->mEState->mGValSize = 0;
    vmPtr->mEState->mFunSize = 0;
    vmPtr->mEState->mLoopSize = 0;
    ResetScene(vmPtr);
    CommandTable cmds;
    for (LInt i = 0; i < vmPtr->mEntry->mSize; i++) {
        cmds.mBegin = vmPtr->mVMCode->mCode + vmPtr->mEntry->mTable[i];
        vmPtr->mEState->mContext = &cmds;
        ExecuteCode(vmPtr);
    }
}

LVoid CacheVMCode(LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    CacheStringTable(vmPtr->mStrTable->mTable, vmPtr->mStrTable->mSize, vm);
    CacheSymbolTable(vm);
    CacheInstuctionEntry(vmPtr->mEntry->mTable, sizeof(LInt) * vmPtr->mEntry->mSize);

    // clear inline cache
    for (LInt i = 0; i < vmPtr->mVMCode->mSize; i++) {
        vmPtr->mVMCode->mCode[i].mCache = kBoyiaNull;
    }
    CacheInstuctions(vmPtr->mVMCode->mCode, sizeof(Instruction) * vmPtr->mVMCode->mSize);
}

LVoid LoadStringTable(BoyiaStr* stringTable, LInt size, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    for (LInt i = 0; i < size; i++) {
        vmPtr->mStrTable->mTable[i].mPtr = stringTable[i].mPtr;
        vmPtr->mStrTable->mTable[i].mLen = stringTable[i].mLen;
    }

    vmPtr->mStrTable->mSize = size;
}

LVoid LoadInstructions(LVoid* buffer, LInt size, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    vmPtr->mVMCode->mSize = size / sizeof(Instruction);
    LMemcpy(vmPtr->mVMCode->mCode, buffer, size);
}

LVoid LoadEntryTable(LVoid* buffer, LInt size, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    vmPtr->mEntry->mSize = size / sizeof(LInt);
    LMemcpy(vmPtr->mEntry->mTable, buffer, size);
}

LVoid* CreateGlobalClass(LUintPtr key, LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    // 创建一个类对象
    return CreateFunVal(key, BY_CLASS, vmPtr);
}