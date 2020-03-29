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
#include "SystemUtil.h"
#include "SalLog.h"

#define SntxErrorBuild(error) SntxError(error, gBoyiaVM->mEState->mLineNum)

#ifndef kBoyiaNull
#define kBoyiaNull 0
#endif

#define RuntimeError(key, error) PrintErrorKey(key, error, gBoyiaVM->mEState->mPC->mCodeLine)

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

#define STR2_INT(str) Str2Int(str.mPtr, str.mLen, 10)

extern LInt Str2Int(LInt8* ptr, LInt len, LInt radix);
extern LVoid GCAppendRef(LVoid* address, LUint8 type);

enum TokenType {
    DELIMITER = 1,
    IDENTIFIER,
    NUMBER,
    KEYWORD,
    TEMP,
    VARIABLE,
    STRING_VALUE,
};

enum LogicValue { AND = BY_END + 1,
    OR,
    NOT,
    LT,
    LE,
    GT,
    GE,
    EQ,
    NE }; // 26

enum MathValue { ADD = NE + 1,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    ASSIGN }; // 33
// 标点符号
enum DelimiValue { SEMI = ASSIGN + 1,
    COMMA,
    QUOTE,
    DOT }; // 37
// 小括号，中括号，大括号
enum BracketValue { LPTR = DOT + 1,
    RPTR,
    ARRAY_BEGIN,
    ARRAY_END,
    BLOCK_START,
    BLOCK_END };
// JMP_TRUE,表示 if true,则跳转
enum OpCodeType {
    ASSIGN_VAR = BLOCK_END + 1,
    PUSH,
    POP,
    CALL,
    LOOP,
    LOOP_TRUE,
    JMP,
    JMP_TRUE,
    ELIF,
    IF_END,
    DECL_LOCAL,
    DECL_GLOBAL,
    PUSH_ARG,
    PUSH_PARAMS,
    GET_PROP,
    TMP_LOCAL,
    PUSH_SCENE,
    POP_SCENE,
    CLASS_CREATE,
    FUN_CREATE,
    EXE_CREATE,
    PARAM_CREATE,
    PROP_CREATE,
    HANDLE_EXTEND,
    PUSH_OBJ,
    HANDLE_CONST_STR,
    PROPS_SORT,
};

typedef LInt (*OPHandler)(LVoid* instruction);

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
    kCmdClassExtend,
    kCmdDeclConstStr,
    kCmdExecCreate,
    kCmdParamCreate,
    kCmdPropCreate,
    kCmdFunCreate,
    kCmdAssign
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
    OPHandler mHandler;
    InlineCache* mCache;
    Instruction* mNext;
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
    { D_STR("elif", 4), ELIF },
    { D_STR("else", 4), BY_ELSE },
    { D_STR("do", 2), BY_DO },
    { D_STR("while", 5), BY_WHILE },
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
    LInt8* mProg;
    LInt mLineNum; /* program string position */
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

/* Boyia VM Define
 * Member
 * 1, mPool
 * 2, Function Area
 * 3, gBoyiaVM->mGlobals
 */
typedef struct {
    LVoid* mPool;
    BoyiaFunction* mFunTable;
    BoyiaValue* mGlobals;
    BoyiaValue* mLocals;
    VMCpu* mCpu;
    ExecState* mEState;
    ExecScene* mExecStack;
    LIntPtr* mLoopStack;
    BoyiaValue* mOpStack;
    VMCode* mVMCode;
    OPHandler* mHandlers;
} BoyiaVM;

static NativeFunction* gNativeFunTable = kBoyiaNull;
static BoyiaToken gToken;
static BoyiaVM* gBoyiaVM = kBoyiaNull;

static LUintPtr gThis = GenIdentByStr("this", 4);
static LUintPtr gSuper = GenIdentByStr("super", 5);
/* Global value define end */
static LVoid LocalStatement();

static LVoid IfStatement();

static LVoid PushArgStatement();

static LVoid WhileStatement();

static LVoid DoStatement();

static BoyiaValue* FindVal(LUintPtr key);

static LVoid BlockStatement();

static LVoid FunStatement();

static LInt NextToken();

static LVoid EvalExpression();

static LVoid EvalAssignment();

static BoyiaValue* FindObjProp(BoyiaValue* lVal, LUintPtr rVal, Instruction* inst);

// Handler Declarations Begin
static LInt HandleAssignment(LVoid* ins);

static LInt HandleJumpToIfTrue(LVoid* ins);

static LInt HandleIfEnd(LVoid* ins);

static LInt HandlePushObj(LVoid* ins);

static LInt HandleAdd(LVoid* ins);

static LInt HandleSub(LVoid* ins);

static LInt HandleMul(LVoid* ins);

static LInt HandleDiv(LVoid* ins);

static LInt HandleMod(LVoid* ins);

static LInt HandleLogic(LVoid* ins);

static LInt HandleRelational(LVoid* ins);

static LInt HandlePush(LVoid* ins);

static LInt HandlePop(LVoid* ins);

static LInt HandlePushScene(LVoid* ins);

static LInt HandlePopScene(LVoid* ins);

static LInt HandlePushArg(LVoid* ins);

static LInt HandleTempLocalSize(LVoid* ins);

static LInt HandlePushParams(LVoid* ins);

static LInt HandleCallFunction(LVoid* ins);

static LInt HandleCallInternal(LVoid* ins);
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

static VMCode* CreateVMCode()
{
    VMCode* code = NEW(VMCode);
    code->mCode = NEW_ARRAY(Instruction, CODE_CAPACITY);//new Instruction[CODE_CAPACITY];//
    code->mSize = 0;
    return code;
}

static OPHandler* InitHandlers()
{
    OPHandler* handlers = NEW_ARRAY(OPHandler, 100);
    LMemset(handlers, 0, sizeof(OPHandler) * 100);
    handlers[kCmdJmpTrue] = HandleJumpToIfTrue;
    handlers[kCmdIfEnd] = HandleIfEnd;
    handlers[kCmdElse] = kBoyiaNull;
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

    return handlers;
}

LVoid* InitVM()
{
    BoyiaVM* vm = FAST_NEW(BoyiaVM);
    vm->mPool = InitMemoryPool(MEMORY_SIZE);
    ChangeMemory(vm->mPool);
    /* 一个页面只允许最多NUM_GLOBAL_VARS个函数 */
    vm->mGlobals = NEW_ARRAY(BoyiaValue, NUM_GLOBAL_VARS);
    vm->mLocals = NEW_ARRAY(BoyiaValue, NUM_LOCAL_VARS);
    vm->mFunTable = NEW_ARRAY(BoyiaFunction, NUM_FUNC);

    vm->mOpStack = NEW_ARRAY(BoyiaValue, NUM_RESULT);

    vm->mExecStack = NEW_ARRAY(ExecScene, FUNC_CALLS);
    vm->mLoopStack = NEW_ARRAY(LIntPtr, LOOP_NEST);
    vm->mEState = NEW(ExecState);
    vm->mCpu = NEW(VMCpu);
    vm->mVMCode = CreateVMCode();
    vm->mHandlers = InitHandlers();

    vm->mEState->mGValSize = 0;
    vm->mEState->mFunSize = 0;
    ResetScene(vm);

    return vm;
}

LVoid DestroyVM(LVoid* vm)
{
    BoyiaVM* vmPtr = (BoyiaVM*)vm;
    FreeMemoryPool(vmPtr->mPool);
    FAST_DELETE(vmPtr);
}

LVoid ChangeVM(LVoid* vm)
{
    gBoyiaVM = (BoyiaVM*)vm;
    ChangeMemory(gBoyiaVM->mPool);
}

static BoyiaVM* GetVM()
{
    return gBoyiaVM;
}

static Instruction* AllocateInstruction()
{
    BOYIA_LOG("AllocateInstruction size=%d", gBoyiaVM->mVMCode->mSize);
    return gBoyiaVM->mVMCode->mCode + gBoyiaVM->mVMCode->mSize++;
}

static Instruction* PutInstruction(
    OpCommand* left,
    OpCommand* right,
    LUint8 op,
    OPHandler handler)
{
    //Instruction* newIns = NEW(Instruction);
    Instruction* newIns = AllocateInstruction();
    if (left) {
        newIns->mOPLeft.mType = left->mType;
        newIns->mOPLeft.mValue = left->mValue;
    }

    if (right) {
        newIns->mOPRight.mType = right->mType;
        newIns->mOPRight.mValue = right->mValue;
    }

    newIns->mCodeLine = gBoyiaVM->mEState->mLineNum;
    newIns->mOPCode = op;
    newIns->mHandler = handler;
    newIns->mNext = kBoyiaNull;
    newIns->mCache = kBoyiaNull;
    Instruction* ins = gBoyiaVM->mEState->mContext->mEnd;
    if (!ins) {
        gBoyiaVM->mEState->mContext->mBegin = newIns;
    } else {
        ins->mNext = newIns;
    }

    gBoyiaVM->mEState->mContext->mEnd = newIns;
    return newIns;
}

static LInt HandlePopScene(LVoid* ins)
{
    if (gBoyiaVM->mEState->mFunctos > 0) {
        gBoyiaVM->mEState->mLValSize = gBoyiaVM->mExecStack[--gBoyiaVM->mEState->mFunctos].mLValSize;
        gBoyiaVM->mEState->mPC = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mPC;
        gBoyiaVM->mEState->mContext = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mContext;
        gBoyiaVM->mEState->mLoopSize = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mLoopSize;
        gBoyiaVM->mEState->mClass = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mClass;
        gBoyiaVM->mEState->mTmpLValSize = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mTmpLValSize;
    }

    return 1;
}

static LVoid ExecPopFunction()
{
    // 指令为空，则判断是否处于函数范围中，是则pop，从而取得调用之前的运行环境
    if (!gBoyiaVM->mEState->mPC && gBoyiaVM->mEState->mFunctos > 0) {
        HandlePopScene(kBoyiaNull);
        if (gBoyiaVM->mEState->mPC) {
            gBoyiaVM->mEState->mPC = gBoyiaVM->mEState->mPC->mNext;
            ExecPopFunction();
        }
    }
}

static LVoid ExecInstruction()
{
    // 通过指令寄存器进行计算
    ExecState* es = gBoyiaVM->mEState;
    while (es->mPC) {
        OPHandler handler = es->mPC->mOPCode >= 100 ? es->mPC->mHandler : gBoyiaVM->mHandlers[es->mPC->mOPCode];
        if (!handler) {
           handler = es->mPC->mHandler;
        }
        if (handler) {
            LInt result = handler(es->mPC);
            if (result == 0) { // 指令运行出错跳出循环
                break;
            } else if (es->mPC && result == 2) { // 函数跳转
                continue;
            } // 指令计算结果为1即为正常情况
        }

        if (es->mPC) {
            es->mPC = es->mPC->mNext;
        }

        ExecPopFunction();
    }
}

static LVoid Putback()
{
    gBoyiaVM->mEState->mProg -= gToken.mTokenName.mLen;
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

static LInt FindNativeFunc(LUintPtr key)
{
    LInt idx = -1;
    while (gNativeFunTable[++idx].mAddr) {
        if (gNativeFunTable[idx].mNameKey == key)
            return idx;
    }

    return -1;
}

static BoyiaFunction* CopyFunction(BoyiaValue* clsVal, LInt count)
{
    BoyiaFunction* newFunc = NEW(BoyiaFunction);
    // copy function
    BOYIA_LOG("HandleCallInternal CreateObject %d", 5);

    BoyiaFunction* func = (BoyiaFunction*)clsVal->mValue.mObj.mPtr;
    BOYIA_LOG("HandleCallInternal CreateObject %d", 6);
    newFunc->mParams = NEW_ARRAY(BoyiaValue, NUM_FUNC_PARAMS);
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

LInt CreateObject()
{
    BOYIA_LOG("HandleCallInternal CreateObject %d", 1);
    BoyiaValue* value = (BoyiaValue*)GetLocalValue(0);
    if (!value || value->mValueType != BY_CLASS) {
        return 0;
    }

    BOYIA_LOG("HandleCallInternal CreateObject %d", 2);
    // 获取CLASS的内部实现
    BOYIA_LOG("HandleCallInternal CreateObject %d", 3);
    // 指针引用R0
    BoyiaValue* result = &gBoyiaVM->mCpu->mReg0;
    // 设置result的值
    ValueCopy(result, value);
    // 拷贝出新的内部实现
    BoyiaFunction* newFunc = CopyFunction(value, NUM_FUNC_PARAMS);
    result->mValue.mObj.mPtr = (LIntPtr)newFunc;
    result->mValue.mObj.mSuper = value->mValue.mObj.mSuper;
    BOYIA_LOG("HandleCallInternal CreateObject %d", 4);

    GCAppendRef(newFunc, BY_CLASS);
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

static LInt HandleBreak(LVoid* ins)
{
    BOYIA_LOG("HandleBreak mLoopSize=%d \n", gBoyiaVM->mEState->mLoopSize);
    gBoyiaVM->mEState->mPC = (Instruction*)gBoyiaVM->mLoopStack[--gBoyiaVM->mEState->mLoopSize];
    //EngineLog("HandleBreak end=%d \n", gBoyiaVM->mEState->mPC->mNext->mOPCode);
    return 1;
}

static LVoid BreakStatement()
{
    BOYIA_LOG("BreakStatement inst code=%d \n", 1);
    PutInstruction(kBoyiaNull, kBoyiaNull, BY_BREAK, HandleBreak);
}

static LInt HandleCreateProp(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaFunction* func = (BoyiaFunction*)gBoyiaVM->mEState->mClass->mValue.mObj.mPtr;
    func->mParams[func->mParamSize].mNameKey = (LUintPtr)inst->mOPLeft.mValue;
    func->mParams[func->mParamSize].mValue.mIntVal = 0;
    func->mParamSize++;
    return 1;
}

static LVoid PropStatement()
{
    NextToken();
    //EngineStrLog("PropStatement name=%s", gToken.mTokenName);
    if (gToken.mTokenType == IDENTIFIER) {
        OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&gToken.mTokenName) };
        PutInstruction(&cmd, kBoyiaNull, PROP_CREATE, HandleCreateProp);
        Putback();
        EvalExpression();

        if (gToken.mTokenValue != SEMI) {
            SntxErrorBuild(SEMI_EXPECTED);
        }
    } else {
        SntxErrorBuild(SYNTAX);
    }
}

LVoid LocalPush(BoyiaValue* value)
{
    if (gBoyiaVM->mEState->mLValSize > NUM_LOCAL_VARS) {
        SntxError(TOO_MANY_LVARS, gBoyiaVM->mEState->mPC->mCodeLine);
    }

    ValueCopy(gBoyiaVM->mLocals + (gBoyiaVM->mEState->mLValSize++), value);
}

static BoyiaValue* FindGlobal(LUintPtr key)
{
    for (LInt idx = 0; idx < gBoyiaVM->mEState->mGValSize; ++idx) {
        if (gBoyiaVM->mGlobals[idx].mNameKey == key)
            return &gBoyiaVM->mGlobals[idx];
    }

    return kBoyiaNull;
}

/* Find the value of a variable. */
static BoyiaValue* GetVal(LUintPtr key)
{
    /* First, see if has obj scope */
    if (key == gThis) {
        return gBoyiaVM->mEState->mClass;
    }

    if (key == gSuper) {
        return gBoyiaVM->mEState->mClass ? (BoyiaValue*)gBoyiaVM->mEState->mClass->mValue.mObj.mSuper : kBoyiaNull;
    }

    /* second, see if it's a local variable */
    LInt start = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos - 1].mLValSize;
    LInt idx = gBoyiaVM->mEState->mLValSize - 1;
    // idx>localLen而不是idx>=localLen，原因则是，第一个元素实际上是函数变量本身
    for (; idx > start; --idx) {
        if (gBoyiaVM->mLocals[idx].mNameKey == key)
            return &gBoyiaVM->mLocals[idx];
    }

    /* otherwise, try global vars */
    BoyiaValue* val = FindGlobal(key);
    if (val) {
        return val;
    }

    return FindObjProp(gBoyiaVM->mEState->mClass, key, kBoyiaNull);
}

static BoyiaValue* FindVal(LUintPtr key)
{
    BoyiaValue* value = GetVal(key);
    if (!value) {
        //SntxError(NOT_VAR, gBoyiaVM->mEState->mPC->mCodeLine);
        RuntimeError(key, NOT_VAR);
    }

    return value;
}

static BoyiaValue* GetOpValue(Instruction* inst, LInt8 type)
{
    BoyiaValue* val = kBoyiaNull;
    OpCommand* op = type == OpLeft ? &inst->mOPLeft : &inst->mOPRight;
    switch (op->mType) { // 赋值左值不可能是常量
    case OP_REG0:
        val = &gBoyiaVM->mCpu->mReg0;
        break;
    case OP_REG1:
        val = &gBoyiaVM->mCpu->mReg1;
        break;
    case OP_VAR:
        val = FindVal((LUintPtr)op->mValue);
        break;
    }

    return val;
}

static LInt HandleCallInternal(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;

    LInt idx = inst->mOPLeft.mValue;
    BOYIA_LOG("HandleCallInternal Exec idx=%d", idx);
    return (*gNativeFunTable[idx].mAddr)();
}

static LInt HandleTempLocalSize(LVoid* ins)
{
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mTmpLValSize = gBoyiaVM->mEState->mTmpLValSize;
    gBoyiaVM->mEState->mTmpLValSize = gBoyiaVM->mEState->mLValSize;
    return 1;
}

static LInt HandlePushScene(LVoid* ins)
{
    if (gBoyiaVM->mEState->mFunctos >= FUNC_CALLS) {
        SntxError(NEST_FUNC, gBoyiaVM->mEState->mPC->mCodeLine);
        return 0;
    }

    Instruction* inst = (Instruction*)ins;
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mLValSize = gBoyiaVM->mEState->mTmpLValSize;
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mPC = (Instruction*)(inst + inst->mOPLeft.mValue);
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mContext = gBoyiaVM->mEState->mContext;
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos++].mLoopSize = gBoyiaVM->mEState->mLoopSize;

    return 1;
}

static LInt HandlePushArg(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* value = GetOpValue(inst, OpLeft);
    if (value) {
        LocalPush(value);
    } else {
        return 0;
    }

    return 1;
}

static LInt HandlePushObj(LVoid* ins)
{
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mClass = gBoyiaVM->mEState->mClass;
    Instruction* inst = (Instruction*)ins;

    if (inst->mOPLeft.mType == OP_VAR) {
        LUintPtr objKey = (LUintPtr)inst->mOPLeft.mValue;
        if (objKey != gSuper) {
            gBoyiaVM->mEState->mClass = (BoyiaValue*)gBoyiaVM->mCpu->mReg0.mNameKey;
        }
    } else {
        gBoyiaVM->mEState->mClass = kBoyiaNull;
    }

    return 1;
}

static LVoid ElseStatement()
{
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdElse, kBoyiaNull);
    BlockStatement();
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdElEnd, kBoyiaNull);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr) (endInst - logicInst); // 最后地址值
}

static LInt HandleReturn(LVoid* ins)
{
    gBoyiaVM->mEState->mPC = gBoyiaVM->mEState->mContext->mEnd;
    return 1;
}

static LVoid ReturnStatement()
{
    EvalExpression(); // => R0
    PutInstruction(kBoyiaNull, kBoyiaNull, BY_RETURN, HandleReturn);
}

static LVoid BlockStatement()
{
    LBool block = LFalse;
    do {
        NextToken();
        //EngineStrLog("BlockStatement name=%s", gToken.mTokenName);
        /* If interpreting single statement, return on first semicolon. */
        /* see what kind of token is up */
        if (gToken.mTokenType == IDENTIFIER) {
            /* Not a keyword, so process expression. */
            Putback(); /* restore token to input stream for further processing by EvalExpression() */
            //EngineStrLog("token name=%s", gToken.mTokenName);
            EvalExpression(); /* process the expression */

            if (gToken.mTokenValue != SEMI) {
                SntxErrorBuild(SEMI_EXPECTED);
            }
        } else if (gToken.mTokenValue == BLOCK_START) {
            block = LTrue;
        } else if (gToken.mTokenValue == BLOCK_END) {
            block = LFalse;
            return; /* is a }, so return */
        } else if (gToken.mTokenType == KEYWORD) { /* is keyword */
            switch (gToken.mTokenValue) {
            case BY_VAR:
                LocalStatement();
                break;
            case BY_FUNC:
                FunStatement();
                break;
            case BY_PROP:
                PropStatement();
                break;
            case BY_RETURN: /* return from function call */
                ReturnStatement();
                break;
            case BY_IF: /* process an if statement */
            case ELIF:
                IfStatement();
                break;
            case BY_ELSE: /* process an else statement */
                ElseStatement();
                break;
            case BY_WHILE: /* process a while loop */
                WhileStatement();
                break;
            case BY_DO: /* process a do-while loop */
                DoStatement();
                break;
            case BY_BREAK:
                BOYIA_LOG("BREAK BreakStatement %d \n", 1);
                BreakStatement();
                break;
            }
        }
    } while (gToken.mTokenValue != BY_END && block);
}

static LVoid SkipComment()
{
    if (*gBoyiaVM->mEState->mProg == '/') {
        if (*(gBoyiaVM->mEState->mProg + 1) == '*') { // 多行注释
            gBoyiaVM->mEState->mProg += 2;
            do {
                while (*gBoyiaVM->mEState->mProg != '*') {
                    if (*gBoyiaVM->mEState->mProg == '\n') {
                        ++gBoyiaVM->mEState->mLineNum;
                    }
                    ++gBoyiaVM->mEState->mProg;
                }
                ++gBoyiaVM->mEState->mProg;
            } while (*gBoyiaVM->mEState->mProg != '/');
            ++gBoyiaVM->mEState->mProg;
            SkipComment();
        } else if (*(gBoyiaVM->mEState->mProg + 1) == '/') { //单行注释
            while (*gBoyiaVM->mEState->mProg && *gBoyiaVM->mEState->mProg != '\n') {
                ++gBoyiaVM->mEState->mProg;
            }

            if (*gBoyiaVM->mEState->mProg == '\n') {
                ++gBoyiaVM->mEState->mLineNum;
                ++gBoyiaVM->mEState->mProg;
            }

            SkipComment();
        }
    }
}

static LInt GetLogicValue()
{
    if (MStrchr("&|!<>=", *gBoyiaVM->mEState->mProg)) {
        LInt len = 0;
        gToken.mTokenName.mPtr = gBoyiaVM->mEState->mProg;
        switch (*gBoyiaVM->mEState->mProg) {
        case '=':
            if (*(gBoyiaVM->mEState->mProg + 1) == '=') {
                gBoyiaVM->mEState->mProg += 2;
                len += 2;
                gToken.mTokenValue = EQ;
            }
            break;
        case '!':
            if (*(gBoyiaVM->mEState->mProg + 1) == '=') {
                gBoyiaVM->mEState->mProg += 2;
                len += 2;
                gToken.mTokenValue = NE;
            } else {
                ++gBoyiaVM->mEState->mProg;
                len = 1;
                gToken.mTokenValue = NOT;
            }
            break;
        case '<':
            if (*(gBoyiaVM->mEState->mProg + 1) == '=') {
                gBoyiaVM->mEState->mProg += 2;
                len += 2;
                gToken.mTokenValue = LE;
            } else {
                ++gBoyiaVM->mEState->mProg;
                len = 1;
                gToken.mTokenValue = LT;
            }
            break;
        case '>':
            if (*(gBoyiaVM->mEState->mProg + 1) == '=') {
                gBoyiaVM->mEState->mProg += 2;
                len += 2;
                gToken.mTokenValue = GE;
            } else {
                gBoyiaVM->mEState->mProg++;
                len = 1;
                gToken.mTokenValue = GT;
            }
            break;
        case '&':
            if (*(gBoyiaVM->mEState->mProg + 1) == '&') {
                gBoyiaVM->mEState->mProg += 2;
                len += 2;
                gToken.mTokenValue = AND;
            }
            break;
        case '|':
            if (*(gBoyiaVM->mEState->mProg + 1) == '|') {
                gBoyiaVM->mEState->mProg += 2;
                len += 2;
                gToken.mTokenValue = OR;
            }
            break;
        }

        if (len) {
            gToken.mTokenName.mLen = len;
            return (gToken.mTokenType = DELIMITER);
        }
    }

    return 0;
}

static LInt GetDelimiter()
{
    const char* delimiConst = "+-*/%^=;,'.()[]{}";
    LInt op = ADD;
    do {
        if (*delimiConst == *gBoyiaVM->mEState->mProg) {
            gToken.mTokenValue = op;
            gToken.mTokenName.mPtr = gBoyiaVM->mEState->mProg;
            gToken.mTokenName.mLen = 1;
            ++gBoyiaVM->mEState->mProg;
            return (gToken.mTokenType = DELIMITER);
        }
        ++op;
        ++delimiConst;
    } while (*delimiConst);

    return 0;
}

static LInt GetIdentifer()
{
    LInt len = 0;
    if (LIsAlpha(*gBoyiaVM->mEState->mProg)) {
        gToken.mTokenName.mPtr = gBoyiaVM->mEState->mProg;
        while (*gBoyiaVM->mEState->mProg == '_' || LIsAlpha(*gBoyiaVM->mEState->mProg) || LIsDigit(*gBoyiaVM->mEState->mProg)) {
            ++gBoyiaVM->mEState->mProg;
            ++len;
        }

        gToken.mTokenName.mLen = len;
        gToken.mTokenType = TEMP;
    }

    if (gToken.mTokenType == TEMP) {
        gToken.mTokenValue = LookUp(&gToken.mTokenName);
        if (gToken.mTokenValue) {
            gToken.mTokenType = KEYWORD;
        } else {
            gToken.mTokenType = IDENTIFIER;
        }
    }

    gToken.mTokenName.mLen = len;
    return gToken.mTokenType;
}

static LInt GetStringValue()
{
    // string
    LInt len = 0;
    if (*gBoyiaVM->mEState->mProg == '"') {
        ++gBoyiaVM->mEState->mProg;
        gToken.mTokenName.mPtr = gBoyiaVM->mEState->mProg;
        while (*gBoyiaVM->mEState->mProg != '"' && *gBoyiaVM->mEState->mProg != '\r') {
            ++gBoyiaVM->mEState->mProg;
            ++len;
        }

        if (*gBoyiaVM->mEState->mProg == '\r') {
            SntxErrorBuild(SYNTAX);
        }
        ++gBoyiaVM->mEState->mProg;
        // +2 for putback just in case
        gToken.mTokenName.mLen = len + 2;
        return (gToken.mTokenType = STRING_VALUE);
    }

    return 0;
}

static LInt GetNumberValue()
{
    LInt len = 0;
    if (LIsDigit(*gBoyiaVM->mEState->mProg)) {
        gToken.mTokenName.mPtr = gBoyiaVM->mEState->mProg;

        while (LIsDigit(*gBoyiaVM->mEState->mProg)) {
            ++gBoyiaVM->mEState->mProg;
            ++len;
        }
        gToken.mTokenName.mLen = len;
        return (gToken.mTokenType = NUMBER);
    }

    return 0;
}

static LInt NextToken()
{
    gToken.mTokenType = 0;
    gToken.mTokenValue = 0;
    InitStr(&gToken.mTokenName, kBoyiaNull);
    while (LIsSpace(*gBoyiaVM->mEState->mProg) && *gBoyiaVM->mEState->mProg) {
        if (*gBoyiaVM->mEState->mProg == '\n') {
            ++gBoyiaVM->mEState->mLineNum;
        }
        ++gBoyiaVM->mEState->mProg;
    }

    if (*gBoyiaVM->mEState->mProg == '\0') {
        gToken.mTokenValue = BY_END;
        return (gToken.mTokenType = DELIMITER);
    }
    // 程序注释部分
    SkipComment();
    // 处理Token操作
    if (GetIdentifer()
        || GetLogicValue()
        || GetDelimiter()
        || GetStringValue()
        || GetNumberValue()) {
        return gToken.mTokenType;
    }

    return 0;
}

static LInt HandleCreateParam(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;

    BoyiaFunction* function = &gBoyiaVM->mFunTable[gBoyiaVM->mEState->mFunSize - 1];
    BoyiaValue* value = &function->mParams[function->mParamSize++];
    value->mNameKey = hashKey;
    return 1;
}

static LVoid InitParams()
{
    do {
        NextToken(); // 得到属性名
        if (gToken.mTokenValue != RPTR) {
            OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&gToken.mTokenName) };
            PutInstruction(&cmd, kBoyiaNull, PARAM_CREATE, HandleCreateParam);
            NextToken(); // 获取逗号分隔符','
        } else
            break;
    } while (gToken.mTokenValue == COMMA);
    if (gToken.mTokenValue != RPTR)
        SntxErrorBuild(PAREN_EXPECTED);
}

static CommandTable* CreateExecutor()
{
    CommandTable* newTable = NEW(CommandTable);
    newTable->mBegin = kBoyiaNull;
    newTable->mEnd = kBoyiaNull;
    return newTable;
}

static LVoid InitFunction(BoyiaFunction* fun)
{
    fun->mParamSize = 0;
    fun->mParams = NEW_ARRAY(BoyiaValue, NUM_FUNC_PARAMS);
    fun->mParamCount = NUM_FUNC_PARAMS;
    ++gBoyiaVM->mEState->mFunSize;
}

static BoyiaValue* CreateFunVal(LUintPtr hashKey, LUint8 type)
{
    // 初始化class类或函数变量
    BoyiaValue* val = &gBoyiaVM->mGlobals[gBoyiaVM->mEState->mGValSize++];
    BoyiaFunction* fun = &gBoyiaVM->mFunTable[gBoyiaVM->mEState->mFunSize];
    val->mValueType = type;
    val->mNameKey = hashKey;
    val->mValue.mObj.mPtr = (LIntPtr)fun;
    val->mValue.mObj.mSuper = 0;
    if (type == BY_CLASS) {
        fun->mFuncBody = (LIntPtr)val;
    }
    // 初始化类属性成员列表
    InitFunction(fun);
    return val;
}

static LInt HandleCreateExecutor(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;

    CommandTable* newTable = NEW(CommandTable);
    newTable->mBegin = &GetVM()->mVMCode->mCode[inst->mOPLeft.mValue];
    newTable->mEnd = &GetVM()->mVMCode->mCode[inst->mOPRight.mValue];

    gBoyiaVM->mFunTable[gBoyiaVM->mEState->mFunSize - 1].mFuncBody = (LIntPtr)newTable;//inst->mOPLeft.mValue;
    return 1;
}

static LVoid BodyStatement(LInt type)
{
    CommandTable* cmds = GetVM()->mEState->mContext;

    CommandTable tmpTable;
    tmpTable.mBegin = kBoyiaNull;
    tmpTable.mEnd = kBoyiaNull;

    Instruction* funInst = kBoyiaNull;
    if (FUN_CREATE == type) {
        // 类成员的创建在主体上下中进行
        //CommandTable* funCmds = CreateExecutor();
        //OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)funCmds };
        funInst = PutInstruction(kBoyiaNull, kBoyiaNull, EXE_CREATE, HandleCreateExecutor);
        GetVM()->mEState->mContext = &tmpTable;
    }

    BlockStatement();
    // 拷贝tmpTable中的offset给instruction
    if (funInst) {
        funInst->mOPLeft.mType = OP_CONST_NUMBER;
        funInst->mOPLeft.mValue = (LIntPtr)(tmpTable.mBegin - GetVM()->mVMCode->mCode);//(LIntPtr)es->mContext->mBegin;
        
        funInst->mOPRight.mType = OP_CONST_NUMBER;
        funInst->mOPRight.mValue = (LIntPtr)(tmpTable.mEnd - GetVM()->mVMCode->mCode);
    }
    GetVM()->mEState->mContext = cmds;
}

static LInt HandleCreateClass(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    if (inst->mOPLeft.mType == OP_NONE) {
        gBoyiaVM->mEState->mClass = kBoyiaNull;
        return 1;
    }
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;
    gBoyiaVM->mEState->mClass = CreateFunVal(hashKey, BY_CLASS);
    return 1;
}

static LInt HandleExtend(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* classVal = FindGlobal((LUintPtr)inst->mOPLeft.mValue);
    BoyiaValue* extendVal = FindGlobal((LUintPtr)inst->mOPRight.mValue);

    // set super pointer
    classVal->mValue.mObj.mSuper = (LIntPtr)extendVal;
    return 1;
}

static LVoid ClassStatement()
{
    NextToken();
    LUintPtr classKey = GenIdentifier(&gToken.mTokenName);
    OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)classKey };
    PutInstruction(&cmd, kBoyiaNull, CLASS_CREATE, HandleCreateClass);
    // 判断继承关系
    NextToken();
    LUintPtr extendKey = 0;
    if (BY_EXTEND == gToken.mTokenValue) {
        NextToken();
        extendKey = GenIdentifier(&gToken.mTokenName);
    } else {
        Putback();
    }
    // 初始化类体
    BodyStatement(CLASS_CREATE);
    // 设置继承成员
    if (extendKey != 0) {
        OpCommand extendCmd = { OP_CONST_NUMBER, (LIntPtr)extendKey };
        PutInstruction(&cmd, &extendCmd, HANDLE_EXTEND, HandleExtend);
    }

    // 执行完后需将CLASS置为kBoyiaNull
    OpCommand cmdEnd = { OP_NONE, 0 };
    PutInstruction(&cmdEnd, kBoyiaNull, CLASS_CREATE, HandleCreateClass);
}

static LInt HandleFunCreate(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;

    if (gBoyiaVM->mEState->mClass) {
        BoyiaFunction* func = (BoyiaFunction*)gBoyiaVM->mEState->mClass->mValue.mObj.mPtr;
        func->mParams[func->mParamSize].mNameKey = hashKey;
        func->mParams[func->mParamSize].mValueType = BY_FUNC;
        func->mParams[func->mParamSize++].mValue.mObj.mPtr = (LIntPtr)&gBoyiaVM->mFunTable[gBoyiaVM->mEState->mFunSize];
        // 初始化函数参数列表
        InitFunction(&gBoyiaVM->mFunTable[gBoyiaVM->mEState->mFunSize]);
    } else {
        CreateFunVal(hashKey, BY_FUNC);
    }

    return 1;
}

static LVoid FunStatement()
{
    NextToken();
    //EngineStrLog("HandlePushParams FunStatement name %s", gToken.mTokenName);
    // 第一步，Function变量
    OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&gToken.mTokenName) };
    PutInstruction(&cmd, kBoyiaNull, FUN_CREATE, HandleFunCreate);
    //EngineStrLog("FunctionName=%s", gToken.mTokenName);
    // 第二步，初始化函数参数
    NextToken(); //   '(', 即LPTR
    InitParams(); //  初始化参数
    // 第三步，函数体内部编译
    BodyStatement(FUN_CREATE);
}

static LVoid DeleteExecutor(CommandTable* table)
{
    Instruction* pc = table->mBegin;
    while (pc != table->mEnd) {
        Instruction* next = pc->mNext;
        VM_DELETE(pc);
        pc = next;
    }

    VM_DELETE(table);
}

// 执行全局的调用
static LVoid ExecuteCode(CommandTable* cmds)
{
    gBoyiaVM->mEState->mContext = cmds;
    gBoyiaVM->mEState->mPC = gBoyiaVM->mEState->mContext->mBegin;
    ExecInstruction();
    // 删除执行体
    //DeleteExecutor(cmds);
    ResetScene(gBoyiaVM);
}

static LInt HandleDeclGlobal(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue val;
    val.mValueType = inst->mOPLeft.mValue;
    val.mNameKey = (LUintPtr)inst->mOPRight.mValue;
    ValueCopy(gBoyiaVM->mGlobals + gBoyiaVM->mEState->mGValSize++, &val);
    return 1;
}

static LVoid GlobalStatement()
{
    LInt type = gToken.mTokenValue;
    do {
        NextToken(); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&gToken.mTokenName) };

        PutInstruction(&cmdLeft, &cmdRight, DECL_GLOBAL, HandleDeclGlobal);
        Putback();
        EvalExpression();
    } while (gToken.mTokenValue == COMMA);

    if (gToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED);
    }
}

// 该函数记录全局变量以及函数接口
static LVoid ParseStatement()
{
    LInt brace = 0; // ‘{’的个数
    gBoyiaVM->mEState->mContext = CreateExecutor();
    do {
        while (brace) {
            NextToken();
            if (gToken.mTokenValue == BLOCK_START) {
                ++brace;
            }

            if (gToken.mTokenValue == BLOCK_END) {
                --brace;
            }
        }

        NextToken();
        if (gToken.mTokenValue == BY_VAR) {
            GlobalStatement();
        } else if (gToken.mTokenValue == BY_FUNC) {
            FunStatement();
        } else if (gToken.mTokenValue == BY_CLASS) {
            ClassStatement();
        } else if (gToken.mTokenType == IDENTIFIER) {
            Putback();
            EvalExpression();

            if (gToken.mTokenValue != SEMI) {
                SntxErrorBuild(SEMI_EXPECTED);
            }
        } else if (gToken.mTokenValue == BLOCK_START) {
            ++brace;
        }
    } while (gToken.mTokenValue != BY_END);

    // 执行全局声明和定义
    ExecuteCode(gBoyiaVM->mEState->mContext);
}

static LInt HandleDeclLocal(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue local;
    local.mValueType = inst->mOPLeft.mValue;
    local.mNameKey = (LUintPtr)inst->mOPRight.mValue;
    LocalPush(&local); // 塞入本地符号表
    return 1;
}

/* Declare a local variable. */
static LVoid LocalStatement()
{
    LInt type = gToken.mTokenValue;
    do {
        NextToken(); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&gToken.mTokenName) };
        //EngineStrLog("value Name=%s", gToken.mTokenName);
        PutInstruction(&cmdLeft, &cmdRight, DECL_LOCAL, HandleDeclLocal);
        Putback();
        EvalExpression();
    } while (gToken.mTokenValue == COMMA);

    if (gToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED);
    }
}

static LInt HandleCallFunction(LVoid* ins)
{
    //EngineLog("HandleFunction begin %d \n", 1);
    // localstack第一个值为函数指针
    LInt start = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos - 1].mLValSize;
    BoyiaValue* value = &gBoyiaVM->mLocals[start];
    BoyiaFunction* func = (BoyiaFunction*)value->mValue.mObj.mPtr;

    gBoyiaVM->mEState->mContext = (CommandTable*)func->mFuncBody;
    gBoyiaVM->mEState->mPC = gBoyiaVM->mEState->mContext->mBegin;
    return 2;
}

static LInt HandlePushParams(LVoid* ins)
{
    // 第一个参数为调用该函数的函数指针
    LInt start = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos - 1].mLValSize;
    BoyiaValue* value = &gBoyiaVM->mLocals[start];
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
            gBoyiaVM->mLocals[idx].mNameKey = vKey;
        }
    }

    return 1;
}

static LInt HandlePop(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    if (inst->mOPLeft.mType != OP_REG0 && inst->mOPLeft.mType != OP_REG1) {
        --gBoyiaVM->mEState->mResultNum;
        return 1;
    }
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &gBoyiaVM->mCpu->mReg0 : &gBoyiaVM->mCpu->mReg1;
    ValueCopy(value, gBoyiaVM->mOpStack + (--gBoyiaVM->mEState->mResultNum));
    return 1;
}

/* Call a function. */
static void CallStatement(OpCommand* objCmd)
{
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdTmpLocal, HandleTempLocalSize);
    // 设置参数
    PushArgStatement();
    // POP CLASS context
    if (objCmd->mType == OP_VAR) {
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPop, HandlePop);
    }
    // 保存对象环境
    PutInstruction(objCmd, kBoyiaNull, kCmdPushObj, HandlePushObj);
    // 保存调用堆栈
    Instruction* pushInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushScene, HandlePushScene);
    // 函数形参名哈希值赋给局部变量
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushParams, HandlePushParams);
    // 执行函数体
    Instruction* funInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdCallFunction, HandleCallFunction);
    //EngineLog("CallStatement=>%d HandleFunction", 1);
    pushInst->mOPLeft.mType = OP_CONST_NUMBER;
    pushInst->mOPLeft.mValue = (LIntPtr)(funInst - pushInst);
}

/* Push the arguments to a function onto the local variable stack. */
static LVoid PushArgStatement()
{
    // push函数指针
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, HandlePushArg);
    NextToken(); // if token == ')' exit
    if (gToken.mTokenValue == RPTR) {
        return;
    } else {
        Putback();
    }

    do {
        // 参数值在R0中
        EvalExpression(); // => R0
        // 将函数实参压栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, HandlePushArg);
        //NextToken();
    } while (gToken.mTokenValue == COMMA);
}

/* Assign a value to a Register 0 or 1. */
static LInt HandleAssignment(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
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
        BoyiaValue* val = FindVal((LUintPtr)inst->mOPRight.mValue);
        if (val) {
            ValueCopyNoName(left, val);
            left->mNameKey = (LUintPtr)val;
        } else {
            return 0;
        }
    } break;
    case OP_REG0: {
        ValueCopyNoName(left, &gBoyiaVM->mCpu->mReg0);
    } break;
    case OP_REG1: {
        ValueCopyNoName(left, &gBoyiaVM->mCpu->mReg1);
    } break;
    }

    return 1;
}

// 执行到ifend证明整个if elseif, else执行完毕，
// 无需检索是否还有elseif，或者else的逻辑判断和内部block
static LInt HandleIfEnd(LVoid* ins)
{
    Instruction* inst = gBoyiaVM->mEState->mPC;
    Instruction* tmpInst = inst->mNext;
    // 查看下一个是否是elseif
    BOYIA_LOG("HandleIfEnd R0=>%d \n", 1);
    while (tmpInst && (tmpInst->mOPCode == kCmdElif || tmpInst->mOPCode == kCmdElse)) {
        inst = (Instruction*)(tmpInst + tmpInst->mOPRight.mValue); // 跳转到elif对应的IFEND
        tmpInst = inst->mNext;
    }
    BOYIA_LOG("HandleIfEnd END R0=>%d \n", 1);
    if (inst) {
        gBoyiaVM->mEState->mPC = inst;
    }
    return 1;
}

static LInt HandleJumpToIfTrue(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* value = &gBoyiaVM->mCpu->mReg0;
    if (!value->mValue.mIntVal) {
        gBoyiaVM->mEState->mPC = inst + inst->mOPRight.mValue;
    }

    return 1;
}

/* Execute an if statement. */
static LVoid IfStatement()
{
    NextToken();
    // token = (
    EvalExpression(); /* check the conditional expression => R0 */
    Instruction* logicInst = PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdJmpTrue, HandleJumpToIfTrue);
    //EngineStrLog("endif last inst name=%s", gToken.mTokenName);
    BlockStatement(); /* if true, interpret */
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdIfEnd, HandleIfEnd);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    //logicInst->mOPRight.mValue = (LIntPtr)endInst; // 最后地址值
    logicInst->mOPRight.mValue = (LIntPtr)(endInst - logicInst); // Compute offset
}

static LInt HandleLoopBegin(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    // push left => loop stack
    gBoyiaVM->mLoopStack[gBoyiaVM->mEState->mLoopSize++] = (LIntPtr)(inst + inst->mOPLeft.mValue);
    return 1;
}

static LInt HandleLoopIfTrue(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* value = &gBoyiaVM->mCpu->mReg0;
    //EngineLog("HandleLoopIfTrue value=%d", value->mValue.mIntVal);
    if (!value->mValue.mIntVal) {
        gBoyiaVM->mEState->mPC = inst + inst->mOPRight.mValue;
        gBoyiaVM->mEState->mLoopSize--;
    }

    return 1;
}

static LInt HandleJumpTo(LVoid* ins)
{
    //EngineLog("HandleJumpTo %d", 1);
    Instruction* inst = (Instruction*)ins;
    if (inst->mOPLeft.mType == OP_CONST_NUMBER) {
        gBoyiaVM->mEState->mPC = inst - inst->mOPLeft.mValue;
    }
    //EngineLog("HandleJumpTo %d", 2);
    return 1;
}

/* Execute a while loop. */
static LVoid WhileStatement()
{
    //EngineLog("WhileStatement %d", 0);
    Instruction* beginInst = PutInstruction(kBoyiaNull, kBoyiaNull, LOOP, HandleLoopBegin);
    NextToken(); // '('
    if (gToken.mTokenValue != LPTR) {
        SntxErrorBuild(LPTR_EXPECTED);
    }
    EvalExpression(); /* check the conditional expression => R0 */
    if (gToken.mTokenValue != RPTR) {
        SntxErrorBuild(RPTR_EXPECTED);
    }
    //EngineStrLog("WhileStatement last inst name=%s", gToken.mTokenName);
    Instruction* logicInst = PutInstruction(&COMMAND_R0, kBoyiaNull, LOOP_TRUE, HandleLoopIfTrue);
    BlockStatement(); /* If true, execute block */
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, JMP, HandleJumpTo);
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
static LVoid DoStatement()
{
    Instruction* beginInst = PutInstruction(kBoyiaNull, kBoyiaNull, LOOP, HandleLoopBegin);
    BlockStatement(); /* interpret loop */
    NextToken();
    if (gToken.mTokenValue != BY_WHILE) {
        SntxErrorBuild(WHILE_EXPECTED);
    }
    EvalExpression(); /* check the loop condition */

    if (gToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED);
    }
    Instruction* logicInst = PutInstruction(&COMMAND_R0, kBoyiaNull, LOOP_TRUE, HandleLoopIfTrue);
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, JMP, HandleJumpTo);
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

static LInt HandleAdd(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
    BoyiaValue* right = GetOpValue(inst, OpRight);
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
        StringAdd(left, right);
        return 1;
    }

    return 0;
}

static LInt HandleSub(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
    BoyiaValue* right = GetOpValue(inst, OpRight);
    if (!left || !right) {
        return 0;
    }

    if (left->mValueType != BY_INT || right->mValueType != BY_INT)
        return 0;

    right->mValue.mIntVal = left->mValue.mIntVal - right->mValue.mIntVal;
    BOYIA_LOG("HandleSub R0=>%d", gBoyiaVM->mCpu->mReg0.mValue.mIntVal);
    return 1;
}

static LInt HandleMul(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
    BoyiaValue* right = GetOpValue(inst, OpRight);

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

static LInt HandleDiv(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
    BoyiaValue* right = GetOpValue(inst, OpRight);

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

static LInt HandleMod(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
    BoyiaValue* right = GetOpValue(inst, OpRight);

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

static LInt HandleRelational(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
    BoyiaValue* right = GetOpValue(inst, OpRight);

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

static LInt HandleLogic(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
    BoyiaValue* right = GetOpValue(inst, OpRight);

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
static LVoid EvalExpression()
{
    NextToken();
    if (!gToken.mTokenName.mLen) {
        SntxErrorBuild(NO_EXP);
        return;
    }

    if (gToken.mTokenValue == SEMI) {
        return;
    }

    EvalAssignment();
}

static LInt HandlePush(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &gBoyiaVM->mCpu->mReg0 : &gBoyiaVM->mCpu->mReg1;
    ValueCopy(gBoyiaVM->mOpStack + (gBoyiaVM->mEState->mResultNum++), value);
    return 1;
}

static LInt HandleAssignVar(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    BoyiaValue* left = GetOpValue(inst, OpLeft);
    BoyiaValue* result = GetOpValue(inst, OpRight);
    if (!left || !result) {
        return 0;
    }
    BoyiaValue* value = (BoyiaValue*)left->mNameKey;
    ValueCopyNoName(value, result);
    ValueCopy(&gBoyiaVM->mCpu->mReg0, value);
    return 1;
}

static LVoid CallNativeStatement(LInt idx)
{
    NextToken();
    if (gToken.mTokenValue != LPTR) // '('
        SntxErrorBuild(PAREN_EXPECTED);

    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdTmpLocal, HandleTempLocalSize);
    do {
        // 参数值在R0中
        EvalExpression(); // => R0
        // 将函数参数压栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPushArg, HandlePushArg);
        // if token == ')' exit
    } while (gToken.mTokenValue == COMMA);

    // 保存obj现场是必不可少的一步
    OpCommand cmdSet = { OP_CONST_NUMBER, 0 };
    PutInstruction(&cmdSet, kBoyiaNull, kCmdPushObj, HandlePushObj);

    Instruction* pushInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPushScene, HandlePushScene);
    OpCommand cmd = { OP_CONST_NUMBER, idx };
    PutInstruction(&cmd, kBoyiaNull, kCmdCallNative, HandleCallInternal);
    Instruction* popInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdPopScene, HandlePopScene);
    pushInst->mOPLeft.mType = OP_CONST_NUMBER;
    pushInst->mOPLeft.mValue = (LIntPtr)(popInst - pushInst);
}

static BoyiaValue* FindObjProp(BoyiaValue* lVal, LUintPtr rVal, Instruction* inst)
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
                InlineCache* cache = inst->mCache ? inst->mCache : (inst->mCache = CreateInlineCache());
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
                    InlineCache* cache = inst->mCache ? inst->mCache : (inst->mCache = CreateInlineCache());
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

static LInt HandleGetProp(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    // r0 -> lVal
    BoyiaValue* lVal = GetOpValue(inst, OpLeft);
    if (!lVal) {
        return 0;
    }

    // fetch value from inline cache
    BoyiaValue* result = GetInlineCache(inst->mCache, lVal);
    if (result) {
        ValueCopyWithKey(&gBoyiaVM->mCpu->mReg0, result);
        return 1;
    }

    LUintPtr rVal = (LUintPtr)inst->mOPRight.mValue;
    result = FindObjProp(lVal, rVal, inst);
    if (result) {
        // maybe function
        ValueCopyWithKey(&gBoyiaVM->mCpu->mReg0, result);
        return 1;
    }

    return 0;
}

// According to reg0, get reg0 obj's prop
static LVoid EvalGetProp()
{
    NextToken();
    if (gToken.mTokenType != IDENTIFIER) {
        return;
    }

    // Push class context for callstatement
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush);
    LUintPtr propKey = GenIdentifier(&gToken.mTokenName);
    OpCommand cmdR = { OP_CONST_NUMBER, (LIntPtr)propKey };
    PutInstruction(&COMMAND_R0, &cmdR, GET_PROP, HandleGetProp);

    // Last must next
    NextToken();
    if (gToken.mTokenValue == LPTR) {
        OpCommand objCmd = { OP_VAR, 0 };
        CallStatement(&objCmd); // result into r0
        NextToken();
        if (gToken.mTokenValue == DOT) {
            // obj.func().prop
            EvalGetProp();
        }
    } else if (gToken.mTokenValue == DOT) {
        OpCommand cmd = { OP_CONST_NUMBER, 0 };
        PutInstruction(&cmd, kBoyiaNull, kCmdPop, HandlePop);
        EvalGetProp();
    } else {
        OpCommand cmd = { OP_CONST_NUMBER, 0 };
        PutInstruction(&cmd, kBoyiaNull, kCmdPop, HandlePop);
    }
}

static LVoid EvalGetValue(LUintPtr objKey)
{
    OpCommand cmdR = { OP_VAR, (LIntPtr)objKey };
    PutInstruction(&COMMAND_R0, &cmdR, ASSIGN, HandleAssignment);
    if (gToken.mTokenValue == DOT) {
        EvalGetProp();
    }
}

static LVoid CopyStringFromToken(BoyiaStr* str)
{
    str->mLen = gToken.mTokenName.mLen - 2;
    str->mPtr = NEW_ARRAY(LInt8, str->mLen);
    LMemcpy(str->mPtr, gToken.mTokenName.mPtr, str->mLen * sizeof(LInt8));
}

static LInt HandleConstString(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    gBoyiaVM->mCpu->mReg0.mValueType = BY_STRING;
    gBoyiaVM->mCpu->mReg0.mValue.mStrVal.mPtr = (LInt8*)inst->mOPLeft.mValue;
    gBoyiaVM->mCpu->mReg0.mValue.mStrVal.mLen = inst->mOPRight.mValue;
    return 1;
}

static LVoid Atom()
{
    switch (gToken.mTokenType) {
    case IDENTIFIER: {
        LInt idx = FindNativeFunc(GenIdentifier(&gToken.mTokenName));
        if (idx != -1) {
            CallNativeStatement(idx);
            NextToken();
        } else {
            LUintPtr key = GenIdentifier(&gToken.mTokenName);
            NextToken();
            if (gToken.mTokenValue == LPTR) {
                OpCommand cmd = { OP_VAR, (LIntPtr)key };
                PutInstruction(&COMMAND_R0, &cmd, ASSIGN, HandleAssignment);
                OpCommand objCmd = { OP_CONST_NUMBER, 0 };
                CallStatement(&objCmd);
                NextToken();
                if (gToken.mTokenValue == DOT) {
                    EvalGetProp();
                }
            } else {
                //EngineLog("Atom var name %u \n", key);
                EvalGetValue(key);
            }
        }
    }
        return;
    case NUMBER: {
        OpCommand cmd = { OP_CONST_NUMBER, STR2_INT(gToken.mTokenName) };
        //EngineLog("Atom NUMBER=%d \n", cmd.mValue);
        PutInstruction(&COMMAND_R0, &cmd, ASSIGN, HandleAssignment);
        NextToken();
    }
        return;
    case STRING_VALUE: {
        // 设置常量字符串， 所有常量hash值为0
        BoyiaStr constStr;
        CopyStringFromToken(&constStr);
        OpCommand lCmd = { OP_CONST_NUMBER, (LIntPtr)constStr.mPtr };
        OpCommand rCmd = { OP_CONST_NUMBER, constStr.mLen };
        PutInstruction(&lCmd, &rCmd, HANDLE_CONST_STR, HandleConstString);
        NextToken();
    }
        return;
    default: {
        if (gToken.mTokenValue == RPTR)
            return;
        else
            SntxErrorBuild(SYNTAX);
    } break;
    }
}

static LVoid EvalSubexpr()
{
    if (gToken.mTokenValue == LPTR) {
        NextToken();
        //EngineStrLog("EvalSubexpr %s", gToken.mTokenName);
        EvalAssignment();
        //EngineStrLog("EvalSubexpr %s", gToken.mTokenName);
        if (gToken.mTokenValue != RPTR) {
            SntxErrorBuild(PAREN_EXPECTED);
        }
        NextToken();
    } else {
        Atom();
    }
}

// 正负数,+1,-1
static LVoid EvalMinus()
{
    LInt8 op = 0;
    if (gToken.mTokenValue == ADD || gToken.mTokenValue == SUB) {
        op = gToken.mTokenValue;
        NextToken();
    }
    EvalSubexpr(); // => R0
    if (op && op == SUB) { // negative must multiply -1
        OpCommand cmd = { OP_CONST_NUMBER, -1 };
        PutInstruction(&COMMAND_R1, &cmd, ASSIGN, HandleAssignment);
        PutInstruction(&COMMAND_R0, &COMMAND_R1, kCmdMul, HandleMul);
    }
}

// 乘除, *,/,%
static LVoid EvalArith()
{
    LInt8 op = 0;
    EvalMinus();

    while ((op = gToken.mTokenValue) == MUL || op == DIV || op == MOD) { // * / %
        // PUSH R0
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush);
        NextToken();
        EvalMinus();
        // POP R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop);
        switch (op) {
        case MUL:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdMul, HandleMul);
            break;
        case DIV:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdDiv, HandleDiv);
            break;
        case MOD:
            PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdMod, HandleMod);
            break;
        }
    }
}

// 加减,+,-
static LVoid EvalAddSub()
{
    LInt8 op = 0;
    EvalArith(); // => R0
    while ((op = gToken.mTokenValue) == ADD || op == SUB) {
        // PUSH
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush);

        NextToken();
        EvalArith();
        // POP R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop);
        // R0 + R1 => R0
        // R1 - R0 => R0
        PutInstruction(&COMMAND_R1, &COMMAND_R0, 
            op == ADD ? kCmdAdd : kCmdSub, 
            op == ADD ? HandleAdd : HandleSub);
    }
}

static LVoid EvalRelationalImpl(LInt8 opToken)
{
    switch (opToken) {
    case NOT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdNotRelation, HandleRelational);
        break;
    case LT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdLtRelation, HandleRelational);
        break;
    case LE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdLeRelation, HandleRelational);
        break;
    case GT:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdGtRelation, HandleRelational);
        break;
    case GE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdGeRelation, HandleRelational);
        break;
    case EQ:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdEqRelation, HandleRelational);
        break;
    case NE:
        PutInstruction(&COMMAND_R1, &COMMAND_R0, kCmdNeRelation, HandleRelational);
        break;
    }
}

// 关系比较判断,>,<,==,!=
static LVoid EvalRelational()
{
    static LInt8 relops[8] = {
        NOT, LT, LE, GT, GE, EQ, NE, 0
    };

    // '<' and '>' etc need both sides of expression
    // ，but '!' only need right side.
    if (gToken.mTokenValue != NOT) {
        EvalAddSub(); // 计算结果 => R0
    }

    LInt8 op = gToken.mTokenValue;
    if (MStrchr(relops, op)) {
        // 计算的结果存入栈中
        if (op != NOT) {
            PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush);
        }

        NextToken(); // 查找标识符或者常量
        EvalAddSub(); // 先执行优先级高的操作 => R0
        // pop R1
        // 上次计算的结果出栈至R1
        if (op != NOT) {
            PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop);
        }

        // 计算R0 OP R1, 结果存入R0中
        //PutInstruction(&COMMAND_R1, &COMMAND_R0, op, HandleRelational);
        EvalRelationalImpl(op);
    }
}

static LVoid EvalLogic()
{
    static LInt8 logicops[3] = {
        AND, OR, 0
    };

    EvalRelational();
    LInt8 op = 0;
    while (MStrchr(logicops, (op = gToken.mTokenValue))) {
        // 计算的结果存入栈中
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush);

        NextToken(); // 查找标识符或者常量
        EvalRelational(); // 先执行优先级高的操作 => R0
        // pop R1
        // 上次计算的结果出栈至R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop);

        // 计算R0 OP R1, 结果存入R0中
        PutInstruction(&COMMAND_R1, &COMMAND_R0, op == AND ? kCmdAndLogic : kCmdOrLogic, HandleLogic);
    }
}

// 赋值,=
static LVoid EvalAssignment()
{
    EvalLogic(); // =>R0
    if (gToken.mTokenValue == ASSIGN) { // '='
        // R0存入栈
        PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush);
        NextToken();
        EvalLogic(); // =>R0
            // 从栈中吐出数据到R1
        PutInstruction(&COMMAND_R1, kBoyiaNull, kCmdPop, HandlePop);
        PutInstruction(&COMMAND_R1, &COMMAND_R0, ASSIGN_VAR, HandleAssignVar);
    }
}

// init global data such as gBoyiaVM->mGlobals, gBoyiaVM->mLocals, gBoyiaVM->mFunTable
static LVoid InitGlobalData()
{
    if (!gBoyiaVM) {
        gBoyiaVM = (BoyiaVM*)InitVM();
    }
}

LVoid InitNativeFun(NativeFunction* funs)
{
    if (!gNativeFunTable) {
        gNativeFunTable = funs;
    }
}

LVoid SetNativeResult(LVoid* result)
{
    BoyiaValue* value = (BoyiaValue*)result;
    ValueCopy(&gBoyiaVM->mCpu->mReg0, value);
}

LVoid* CopyObject(LUintPtr hashKey, LInt size)
{
    return CopyFunction(FindGlobal(hashKey), size);
}

LVoid* GetNativeResult()
{
    return &gBoyiaVM->mCpu->mReg0;
}

LVoid GetLocalStack(LInt* stack, LInt* size)
{
    *stack = (LIntPtr)gBoyiaVM->mLocals;
    *size = gBoyiaVM->mEState->mLValSize;
}

LVoid GetGlobalTable(LInt* table, LInt* size)
{
    *table = (LIntPtr)gBoyiaVM->mGlobals;
    *size = gBoyiaVM->mEState->mGValSize;
}

/*  output function */
LVoid CompileCode(LInt8* code)
{
    InitGlobalData();
    gBoyiaVM->mEState->mProg = code;
    gBoyiaVM->mEState->mLineNum = 1;
    gBoyiaVM->mEState->mTmpLValSize = 0;
    gBoyiaVM->mEState->mResultNum = 0;
    gBoyiaVM->mEState->mLoopSize = 0;
    gBoyiaVM->mEState->mClass = kBoyiaNull;
    ParseStatement(); // 该函数记录全局变量以及函数接口
    ResetScene(gBoyiaVM);
}

LVoid* GetLocalValue(LInt idx)
{
    LInt start = gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos - 1].mLValSize;
    return &gBoyiaVM->mLocals[start + idx];
}

LInt GetLocalSize()
{
    return gBoyiaVM->mEState->mLValSize - gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos - 1].mLValSize;
}

LVoid CallFunction(LInt8* fun, LVoid* ret)
{
    BOYIA_LOG("callFunction=>%d \n", 1);
    gBoyiaVM->mEState->mProg = fun;
    CommandTable* cmds = CreateExecutor();
    gBoyiaVM->mEState->mContext = cmds;
    EvalExpression(); // 解析例如func(a,b,c);
    ExecuteCode(cmds);

    if (ret) {
        BoyiaValue* value = (BoyiaValue*)ret;
        ValueCopy(value, &gBoyiaVM->mCpu->mReg0);
    }
}

LVoid SaveLocalSize()
{
    HandleTempLocalSize(kBoyiaNull);
}

LVoid NativeCall(BoyiaValue* obj)
{
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mClass = gBoyiaVM->mEState->mClass;
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mLValSize = gBoyiaVM->mEState->mTmpLValSize;
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mPC = gBoyiaVM->mEState->mPC;
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos].mContext = gBoyiaVM->mEState->mContext;
    gBoyiaVM->mExecStack[gBoyiaVM->mEState->mFunctos++].mLoopSize = gBoyiaVM->mEState->mLoopSize;
    gBoyiaVM->mEState->mClass = obj;

    HandlePushParams(kBoyiaNull);
    HandleCallFunction(kBoyiaNull);

    ExecInstruction();
}
