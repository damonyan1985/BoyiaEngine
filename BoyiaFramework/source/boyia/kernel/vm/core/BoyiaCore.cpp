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

#define SntxErrorBuild(error, cs) SntxError(error, cs->mLineNum)

#ifndef kBoyiaNull
#define kBoyiaNull 0
#endif

#define kInvalidInstruction (-1)
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
#define CONST_CAPACITY ((LInt)1024)
#define ENTRY_CAPACITY ((LInt)1024)

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
    VMStrTable* mStrTable;
    VMEntryTable* mEntry;
    OPHandler* mHandlers;
} BoyiaVM;

typedef struct {
    LInt8* mProg;
    LInt mLineNum; // Current Line num of source code
    BoyiaToken mToken;
    BoyiaVM* mVm;
} CompileState;

static NativeFunction* gNativeFunTable = kBoyiaNull;
static BoyiaVM* gBoyiaVM = kBoyiaNull;

static LUintPtr gThis = GenIdentByStr("this", 4);
static LUintPtr gSuper = GenIdentByStr("super", 5);
/* Global value define end */
static LVoid LocalStatement(CompileState* cs);

static LVoid IfStatement(CompileState* cs);

static LVoid PushArgStatement(CompileState* cs);

static LVoid WhileStatement(CompileState* cs);

static LVoid DoStatement(CompileState* cs);

static BoyiaValue* FindVal(LUintPtr key);

static LVoid BlockStatement(CompileState* cs);

static LVoid FunStatement(CompileState* cs);

static LInt NextToken(CompileState* cs);

static LVoid EvalExpression(CompileState* cs);

static LVoid EvalAssignment(CompileState* cs);

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

static LInt HandleLoopIfTrue(LVoid* ins);

static LInt HandleJumpTo(LVoid* ins);

static LInt HandleGetProp(LVoid* ins);

static LInt HandleConstString(LVoid* ins);

static LInt HandleAssignment(LVoid* ins);

static LInt HandleAssignVar(LVoid* ins);

static LInt HandleLoopBegin(LVoid* ins);

static LInt HandleCreateClass(LVoid* ins);

static LInt HandleExtend(LVoid* ins);

static LInt HandleDeclGlobal(LVoid* ins);

static LInt HandleDeclLocal(LVoid* ins);

static LInt HandleFunCreate(LVoid* ins);

static LInt HandleCreateExecutor(LVoid* ins);

static LInt HandleCreateParam(LVoid* ins);

static LInt HandleReturn(LVoid* ins);

static LInt HandleBreak(LVoid* ins);

static LInt HandleCreateProp(LVoid* ins);
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

static VMStrTable* CreateVMStringTable()
{
    VMStrTable* table = NEW(VMStrTable);
    table->mSize = 0;
    return table;
}

static VMEntryTable* CreateVMEntryTable()
{
    VMEntryTable* table = NEW(VMEntryTable);
    table->mSize = 0;
    return table;
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
    vm->mStrTable = CreateVMStringTable();
    vm->mEntry = CreateVMEntryTable();

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

static Instruction* AllocateInstruction(BoyiaVM* vm)
{
    BOYIA_LOG("AllocateInstruction size=%d", gBoyiaVM->mVMCode->mSize);
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

static Instruction* NextInstruction(Instruction* instruction) {
    if (instruction->mNext == kInvalidInstruction) {
        return kBoyiaNull;
    }

    return GetVM()->mVMCode->mCode + instruction->mNext;
}

static LVoid ExecPopFunction()
{
    // 指令为空，则判断是否处于函数范围中，是则pop，从而取得调用之前的运行环境
    if (!gBoyiaVM->mEState->mPC && gBoyiaVM->mEState->mFunctos > 0) {
        HandlePopScene(kBoyiaNull);
        if (gBoyiaVM->mEState->mPC) {
            gBoyiaVM->mEState->mPC = NextInstruction(gBoyiaVM->mEState->mPC);//gBoyiaVM->mEState->mPC->mNext;
            ExecPopFunction();
        }
    }
}

static LVoid ExecInstruction()
{
    // 通过指令寄存器进行计算
    ExecState* es = gBoyiaVM->mEState;
    while (es->mPC) {
        //OPHandler handler = es->mPC->mOPCode >= 100 ? es->mPC->mHandler : gBoyiaVM->mHandlers[es->mPC->mOPCode];
        OPHandler handler = gBoyiaVM->mHandlers[es->mPC->mOPCode];
        /*
        if (!handler) {
           handler = es->mPC->mHandler;
        }*/
        if (handler) {
            LInt result = handler(es->mPC);
            if (result == 0) { // 指令运行出错跳出循环
                break;
            } else if (es->mPC && result == 2) { // 函数跳转
                continue;
            } // 指令计算结果为1即为正常情况
        }

        if (es->mPC) {
            es->mPC = NextInstruction(es->mPC);//es->mPC->mNext;
        }

        ExecPopFunction();
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

static LVoid BreakStatement(CompileState* cs)
{
    BOYIA_LOG("BreakStatement inst code=%d \n", 1);
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdBreak, HandleBreak, cs);
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

static LVoid PropStatement(CompileState* cs)
{
    NextToken(cs);
    //EngineStrLog("PropStatement name=%s", cs->mToken.mTokenName);
    if (cs->mToken.mTokenType == IDENTIFIER) {
        OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName) };
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
        return 1;
    }

    return 0;
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

static LVoid ElseStatement(CompileState* cs)
{
    Instruction* logicInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdElse, kBoyiaNull, cs);
    BlockStatement(cs);
    Instruction* endInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdElEnd, kBoyiaNull, cs);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr) (endInst - logicInst); // 最后地址值
}

static LInt HandleReturn(LVoid* ins)
{
    gBoyiaVM->mEState->mPC = gBoyiaVM->mEState->mContext->mEnd;
    return 1;
}

static LVoid ReturnStatement(CompileState* cs)
{
    EvalExpression(cs); // => R0
    PutInstruction(kBoyiaNull, kBoyiaNull, kCmdReturn, HandleReturn, cs);
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

static LInt HandleCreateParam(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    LUintPtr hashKey = (LUintPtr)inst->mOPLeft.mValue;

    BoyiaFunction* function = &gBoyiaVM->mFunTable[gBoyiaVM->mEState->mFunSize - 1];
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

        OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName) };
        PutInstruction(&cmd, kBoyiaNull, kCmdParamCreate, HandleCreateParam, cs);
        NextToken(cs); // 获取逗号分隔符','
    } while (cs->mToken.mTokenValue == COMMA);
    if (cs->mToken.mTokenValue != RPTR)
        SntxErrorBuild(PAREN_EXPECTED, cs);
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

static LVoid BodyStatement(CompileState* cs, LBool isFunction)
{
    CommandTable* cmds = GetVM()->mEState->mContext;

    CommandTable tmpTable;
    tmpTable.mBegin = kBoyiaNull;
    tmpTable.mEnd = kBoyiaNull;

    Instruction* funInst = kBoyiaNull;
    if (isFunction) {
        // 类成员的创建在主体上下中进行
        //CommandTable* funCmds = CreateExecutor();
        //OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)funCmds };
        funInst = PutInstruction(kBoyiaNull, kBoyiaNull, kCmdExecCreate, HandleCreateExecutor, cs);
        GetVM()->mEState->mContext = &tmpTable;
    }

    BlockStatement(cs);
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

static LVoid ClassStatement(CompileState* cs)
{
    NextToken(cs);
    LUintPtr classKey = GenIdentifier(&cs->mToken.mTokenName);
    OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)classKey };
    PutInstruction(&cmd, kBoyiaNull, kCmdCreateClass, HandleCreateClass, cs);
    // 判断继承关系
    NextToken(cs);
    LUintPtr extendKey = 0;
    if (BY_EXTEND == cs->mToken.mTokenValue) {
        NextToken(cs);
        extendKey = GenIdentifier(&cs->mToken.mTokenName);
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

static LVoid FunStatement(CompileState* cs)
{
    NextToken(cs);
    //EngineStrLog("HandlePushParams FunStatement name %s", cs->mToken.mTokenName);
    // 第一步，Function变量
    OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName) };
    PutInstruction(&cmd, kBoyiaNull, kCmdCreateFunction, HandleFunCreate, cs);
    //EngineStrLog("FunctionName=%s", cs->mToken.mTokenName);
    // 第二步，初始化函数参数
    NextToken(cs); //   '(', 即LPTR
    InitParams(cs); //  初始化参数
    // 第三步，函数体内部编译
    BodyStatement(cs, LTrue);
}

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

static LVoid GlobalStatement(CompileState* cs)
{
    LInt type = cs->mToken.mTokenValue;
    do {
        NextToken(cs); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName) };

        PutInstruction(&cmdLeft, &cmdRight, kCmdDeclGlobal, HandleDeclGlobal, cs);
        Putback(cs);
        EvalExpression(cs);
    } while (cs->mToken.mTokenValue == COMMA);

    if (cs->mToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED, cs);
    }
}

static LVoid AppendEntry(CommandTable* table) 
{
    // Entry的begin为空，证明没有指令
    if (!table || !table->mBegin) {
        return;
    }

    // Entry起始位置获取在VMCode中的索引
    GetVM()->mEntry->mTable[GetVM()->mEntry->mSize++] = table->mBegin - GetVM()->mVMCode->mCode;
}

// 该函数记录全局变量以及函数接口
static LVoid ParseStatement(CompileState* cs)
{
    LInt brace = 0; // ‘{’的个数
    GetVM()->mEState->mContext = CreateExecutor();
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

    AppendEntry(GetVM()->mEState->mContext);
    // 执行全局声明和定义
    ExecuteCode(GetVM()->mEState->mContext);
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
static LVoid LocalStatement(CompileState* cs)
{
    LInt type = cs->mToken.mTokenValue;
    do {
        NextToken(cs); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&cs->mToken.mTokenName) };
        //EngineStrLog("value Name=%s", cs->mToken.mTokenName);
        PutInstruction(&cmdLeft, &cmdRight, kCmdDeclLocal, HandleDeclLocal, cs);
        Putback(cs);
        EvalExpression(cs);
    } while (cs->mToken.mTokenValue == COMMA);

    if (cs->mToken.mTokenValue != SEMI) {
        SntxErrorBuild(SEMI_EXPECTED, cs);
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
        if (!val) {
            return 0;
        }

        ValueCopyNoName(left, val);
        left->mNameKey = (LUintPtr)val;
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
    Instruction* tmpInst = NextInstruction(inst);//inst->mNext;
    // 查看下一个是否是elseif
    BOYIA_LOG("HandleIfEnd R0=>%d \n", 1);
    while (tmpInst && (tmpInst->mOPCode == kCmdElif || tmpInst->mOPCode == kCmdElse)) {
        inst = (Instruction*)(tmpInst + tmpInst->mOPRight.mValue); // 跳转到elif对应的IFEND
        tmpInst = NextInstruction(inst);//inst->mNext;
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
    Instruction* logicInst = PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdLoopTrue, HandleLoopIfTrue, cs);
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
    Instruction* logicInst = PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdLoopTrue, HandleLoopIfTrue, cs);
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
static LVoid EvalGetProp(CompileState* cs)
{
    NextToken(cs);
    if (cs->mToken.mTokenType != IDENTIFIER) {
        return;
    }

    // Push class context for callstatement
    PutInstruction(&COMMAND_R0, kBoyiaNull, kCmdPush, HandlePush, cs);
    LUintPtr propKey = GenIdentifier(&cs->mToken.mTokenName);
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
    str->mPtr = NEW_ARRAY(LInt8, str->mLen);
    LMemcpy(str->mPtr, cs->mToken.mTokenName.mPtr, str->mLen * sizeof(LInt8));
}

static LInt HandleConstString(LVoid* ins)
{
    Instruction* inst = (Instruction*)ins;
    gBoyiaVM->mCpu->mReg0.mValueType = BY_STRING;
    BoyiaStr* constStr = &GetVM()->mStrTable->mTable[inst->mOPLeft.mValue];
    gBoyiaVM->mCpu->mReg0.mValue.mStrVal.mPtr = constStr->mPtr;
    gBoyiaVM->mCpu->mReg0.mValue.mStrVal.mLen = constStr->mLen;
    return 1;
}

static LVoid Atom(CompileState* cs)
{
    switch (cs->mToken.mTokenType) {
    case IDENTIFIER: {
        LInt idx = FindNativeFunc(GenIdentifier(&cs->mToken.mTokenName));
        if (idx != -1) {
            CallNativeStatement(cs, idx);
            NextToken(cs);
        } else {
            LUintPtr key = GenIdentifier(&cs->mToken.mTokenName);
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
        BoyiaStr* constStr = &cs->mVm->mStrTable->mTable[GetVM()->mStrTable->mSize];
        CopyStringFromToken(cs, constStr);
        OpCommand lCmd = { OP_CONST_NUMBER,  GetVM()->mStrTable->mSize++ };
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
    GetVM()->mEState->mTmpLValSize = 0;
    GetVM()->mEState->mResultNum = 0;
    GetVM()->mEState->mLoopSize = 0;
    GetVM()->mEState->mClass = kBoyiaNull;
    CompileState cs;
    cs.mProg = code;
    cs.mLineNum = 1;
    cs.mVm = gBoyiaVM;
    ParseStatement(&cs); // 该函数记录全局变量以及函数接口
    ResetScene(GetVM());
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
    //gBoyiaVM->mEState->mProg = fun;
    CommandTable* cmds = CreateExecutor();
    gBoyiaVM->mEState->mContext = cmds;
    CompileState cs;
    cs.mProg = fun;
    cs.mLineNum = 1;
    cs.mVm = gBoyiaVM;
    EvalExpression(&cs); // 解析例如func(a,b,c);
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

LVoid CacheVMCode()
{
    CacheStringTable(GetVM()->mStrTable->mTable, GetVM()->mStrTable->mSize);
    CacheInstuctionEntry(GetVM()->mEntry->mTable, sizeof(LInt) * GetVM()->mEntry->mSize);
    CacheInstuctions(GetVM()->mVMCode->mCode, sizeof(Instruction) * GetVM()->mVMCode->mSize);
}

LVoid LoadStringTable(BoyiaStr* stringTable, LInt size)
{
    for (LInt i = 0; i < size; i++) {
        GetVM()->mStrTable->mTable[i].mPtr = stringTable[i].mPtr;
        GetVM()->mStrTable->mTable[i].mLen = stringTable[i].mLen;
    }

    GetVM()->mStrTable->mSize = size;
}

LVoid LoadInstructions(LVoid* buffer, LInt size)
{
    GetVM()->mVMCode->mSize = size / sizeof(Instruction);
    LMemcpy(GetVM()->mVMCode->mCode, buffer, size);
}

LVoid LoadEntryTable(LVoid* buffer, LInt size)
{
    GetVM()->mEntry->mSize = size / sizeof(LInt);
    LMemcpy(GetVM()->mEntry->mTable, buffer, size);

    InitGlobalData();
    GetVM()->mEState->mGValSize = 0;
    GetVM()->mEState->mFunSize = 0;
    GetVM()->mEState->mLoopSize = 0;
    ResetScene(GetVM());
    CommandTable cmds;
    for (LInt i = 0; i < GetVM()->mEntry->mSize; i++) {
        cmds.mBegin = GetVM()->mVMCode->mCode + GetVM()->mEntry->mTable[i];
        ExecuteCode(&cmds);
    }
}