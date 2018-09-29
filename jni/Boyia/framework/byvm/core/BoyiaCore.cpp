/*
* Date: 2011-11-07
* Modify: 2018-9-1
* Author: yanbo
* Description: Boyia interpreter
* Copyright (c) reserved
* As a independent module for game engine
*/
#include "BoyiaCore.h"
#include "BoyiaError.h"
//#include <android/log.h>

#define ENABLE_LOG
extern void jsLog(const char* format, ...);

#ifdef ENABLE_LOG
#define EngineLog(format, ...) jsLog(format, __VA_ARGS__)
#else
#define EngineLog(format, ...)
#endif
#define SntxErrorBuild(error) SntxError(error, gEState.mLineNum)

#ifdef NULL
#undef NULL
#endif

#define NULL 0

/* Type Define Begin */
#define NUM_FUNC         ((LInt)1024)
#define NUM_FUNC_PARAMS  ((LInt)32)
#define NUM_GLOBAL_VARS  ((LInt)512)
#define NUM_LOCAL_VARS   ((LInt)512)
#define NUM_RESULT       ((LInt)128)
#define FUNC_CALLS       ((LInt)32)
#define NUM_PARAMS       ((LInt)32)
#define LOOP_NEST        ((LInt)32)

#define STR2_INT(str) Str2Int(str.mPtr, str.mLen, 10)

extern LInt Str2Int(LInt8* ptr, LInt len, LInt radix);
extern LVoid GCAppendRef(LVoid* address, LUint8 type);
extern LVoid SystemGC();

enum TokenType {
    DELIMITER = 1,
    IDENTIFIER,
    NUMBER,
    KEYWORD,
    TEMP,
    VARIABLE,
    STRING_VALUE,
};

enum LogicValue { AND = END + 1, OR, NOT, LT, LE, GT, GE, EQ, NE }; // 26

enum MathValue { ADD = NE + 1, SUB, MUL, DIV, MOD, POW, ASSIGN }; // 33
// 标点符号
enum DelimiValue { SEMI = ASSIGN + 1, COMMA, QUOTE, DOT }; // 37
// 小括号，中括号，大括号
enum BracketValue { LPTR = DOT + 1, RPTR, ARRAY_BEGIN, ARRAY_END, BLOCK_START, BLOCK_END };
// JMP_TRUE,表示 if true,则跳转
enum OpCodeType {
    ASSIGN_VAR = BLOCK_END + 1, PUSH, POP, CALL, LOOP, LOOP_TRUE,
    JMP, JMP_TRUE, ELIF, IF_END, DECL_LOCAL, DECL_GLOBAL,
    PUSH_ARG, PUSH_PARAMS, GET_PROP, TMP_LOCAL, PUSH_SCENE, POP_SCENE,
    CLASS_CREATE, FUN_CREATE, EXE_CREATE, PARAM_CREATE, PROP_CREATE,
    HANDLE_EXTEND, PUSH_OBJ, HANDLE_CONST_STR, PROPS_SORT,
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

typedef struct {
    LUint8    mType;
    LIntPtr   mValue;
} OpCommand;

static OpCommand COMMAND_R0 = { OP_REG0, 0 };
static OpCommand COMMAND_R1 = { OP_REG1, 0 };

typedef struct Instruction {
    LUint8         mOPCode;
    LInt           mCodeLine;
    OpCommand      mOPLeft;
    OpCommand      mOPRight;
    OPHandler      mHandler;
    Instruction*   mNext;
} Instruction;

typedef struct {
    Instruction*   mBegin;
    Instruction*   mEnd;
} CommandTable;

// 虚拟寄存器模型，其中每个寄存器可以表示为4个32寄存器
// Reg的mNameKey没有任何意义，
// 但是在处理变量时变得有意义，即指向变量地址，
typedef struct {
    BoyiaValue     mReg0;    // result register, 结果寄存器
    BoyiaValue     mReg1;    // help register, 辅助运算寄存器
} VMCpu;

struct KeywordPair {
    BoyiaStr  mName; /* keyword lookup table */
    LUint8    mType;
} gKeywordTable[] = {  /* Commands must be entered lowercase */
        {D_STR("if", 2),       IF},  /* in this table. */
        {D_STR("elif", 4),     ELIF},
        {D_STR("else", 4),     ELSE},
        {D_STR("do", 2),       DO},
        {D_STR("while", 5),    WHILE},
        {D_STR("break", 5),    BREAK},
        {D_STR("fun", 3),      FUNC},
        {D_STR("class", 5),    CLASS},
        {D_STR("extends", 7),  EXTEND},
        {D_STR("prop", 4),     PROP},
        {D_STR("var", 3),      VAR},
        {D_STR("return", 6),   RETURN},
        {D_STR("", 0),         0}  /* mark end of table */
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
    LUint8   mTokenType; /* contains type of token*/
    LUint8   mTokenValue; /* internal representation of token */
} MiniToken;

/* Global value define begin */
static BoyiaFunction*   gFunTable         = NULL; // 堆上取数组
static BoyiaValue*      gGlobalsTable     = NULL;
static BoyiaValue*      gLocalsStack      = NULL;
static BoyiaValue*      gOpStack          = NULL;
static CommandTable*   gCommandTable     = NULL;
static NativeFunction* gNativeFunTable   = NULL;

typedef struct {
    LInt8*           mProg;
    LInt             mLineNum;/* program string position */
    LInt             mLoopSize;
    LInt             mFunctos;
    LInt             mFunSize;  /* index to top of function call stack */
    LInt             mGValSize; /* count of function table */
    LInt             mTmpLValSize;
    LInt             mLValSize; /* count of global variable stack */
    LInt             mResultNum;
    BoyiaValue*      mClass;
    CommandTable*    mContext;
    Instruction*     mPC;       // pc , 指令指针
} ExecState;

typedef struct {
    Instruction*     mPC;
    LInt             mLValSize;
    LInt             mTmpLValSize;
    LInt             mLoopSize;
    CommandTable*    mContext;
    BoyiaValue*      mClass;
} ExecScene;

// 调用堆栈
static ExecScene* gCallStack = NULL;
static LInt* gLoopStack = NULL;

static MiniToken       gToken;
static VMCpu           gVM;
static ExecState       gEState;

static LUint gThis = GenIdentByStr("this", 4);
static LUint gSuper = GenIdentByStr("super", 5);
/* Global value define end */
static LVoid LocalStatement();

static BoyiaValue* GetLeftOpValue(Instruction* inst);

static LVoid IfStatement();

static LVoid PushArgStatement();

static LVoid WhileStatement();

static LVoid DoStatement();

static BoyiaValue* FindVal(LUint key);

static LVoid BlockStatement();

static LVoid FunStatement();

static LInt NextToken();

static LVoid EvalExpression();

static LVoid EvalAssignment();

static LInt HandleAssignment(LVoid* ins);

static BoyiaValue* FindObjProp(BoyiaValue* lVal, LUint rVal);

static Instruction* PutInstruction(
	OpCommand* left,
	OpCommand* right,
	LUint8 op,
	OPHandler handler) {
	Instruction* newIns = NEW(Instruction);
	if (left) {
		newIns->mOPLeft.mType = left->mType;
		newIns->mOPLeft.mValue = left->mValue;
	}

	if (right) {
		newIns->mOPRight.mType = right->mType;
		newIns->mOPRight.mValue = right->mValue;
	}

	newIns->mCodeLine = gEState.mLineNum;
	newIns->mOPCode = op;
	newIns->mHandler = handler;
	newIns->mNext = NULL;
	Instruction* ins = gEState.mContext->mEnd;
	if (!ins) {
		gEState.mContext->mBegin = newIns;
	} else {
		ins->mNext = newIns;
	}

	gEState.mContext->mEnd = newIns;
	return newIns;
}

static LInt HandlePopScene(LVoid* ins) {
	if (gEState.mFunctos > 0) {
		gEState.mLValSize = gCallStack[--gEState.mFunctos].mLValSize;
		gEState.mPC = gCallStack[gEState.mFunctos].mPC;
		gEState.mContext = gCallStack[gEState.mFunctos].mContext;
		gEState.mLoopSize = gCallStack[gEState.mFunctos].mLoopSize;
		gEState.mClass = gCallStack[gEState.mFunctos].mClass;
		gEState.mTmpLValSize = gCallStack[gEState.mFunctos].mTmpLValSize;
	}

	SystemGC();
	return 1;
}

static void ExecPopFunction() {
	// 指令为空，则判断是否处于函数范围中，是则pop，从而取得调用之前的运行环境
	if (!gEState.mPC && gEState.mFunctos > 0) {
		HandlePopScene(NULL);
		if (gEState.mPC) {
			gEState.mPC = gEState.mPC->mNext;
			ExecPopFunction();
		}
	}
}

static void ExecInstruction() {
    // 通过指令寄存器进行计算
    while (gEState.mPC) {
		if (gEState.mPC->mHandler) {
			LInt result = gEState.mPC->mHandler(gEState.mPC);
			if (result == 0) {        // 指令运行出错跳出循环
				break;
			} else if (gEState.mPC && result == 2) { // 函数跳转
				continue;
			} // 指令计算结果为1即为正常情况
		}

		if (gEState.mPC) {
			gEState.mPC = gEState.mPC->mNext;
		}

		ExecPopFunction();
    }
}

static LVoid Putback() {
    gEState.mProg -= gToken.mTokenName.mLen;
}

static LUint8 LookUp(BoyiaStr* name) {
    register LInt i = 0;
    for (; gKeywordTable[i].mName.mLen; ++i) {
        if (MStrcmp(&gKeywordTable[i].mName, name))
            return gKeywordTable[i].mType;
    }

    return 0;
}

static LInt FindNativeFunc(LUint key) {
    LInt idx = -1;
    while (gNativeFunTable[++idx].mAddr) {
        if (gNativeFunTable[idx].mNameKey == key)
            return idx;
    }

    return -1;
}

static BoyiaFunction* CopyFunction(BoyiaValue* clsVal, LInt count) {
    BoyiaFunction* newFunc = NEW(BoyiaFunction);
    // copy function
    EngineLog("HandleCallInternal CreateObject %d", 5);

    BoyiaFunction* func = (BoyiaFunction*) clsVal->mValue.mObj.mPtr;
    EngineLog("HandleCallInternal CreateObject %d", 6);
    newFunc->mParams = NEW_ARRAY(BoyiaValue, NUM_FUNC_PARAMS);
    //EngineLog("HandleCallInternal CreateObject %d", 7);
    newFunc->mParamSize = 0;
    newFunc->mFuncBody = func->mFuncBody;
	newFunc->mParamCount = count;
	//EngineLog("HandleCallInternal CreateObject %d", 7);
	//EngineLog("HandleCallInternal CreateObject mParams mAddr %d",  (int)newFunc->mParams);

    while (clsVal) {
    	BoyiaFunction* func = (BoyiaFunction*) clsVal->mValue.mObj.mPtr;
    	LInt idx = func->mParamSize;
    	while (idx--) {
    	    if (func->mParams[idx].mValueType != FUNC) {
    	    	ValueCopy(newFunc->mParams+newFunc->mParamSize++, func->mParams+idx);
    	    }
    	}

    	clsVal = (BoyiaValue*) clsVal->mValue.mObj.mSuper;
    }

    return newFunc;
}

static LInt CreateObject() {
	EngineLog("HandleCallInternal CreateObject %d", 1);
	BoyiaValue* value = (BoyiaValue*) GetLocalValue(0);
    if (!value || value->mValueType != CLASS) {
        return 0;
    }

    EngineLog("HandleCallInternal CreateObject %d", 2);
    // 获取CLASS的内部实现
    EngineLog("HandleCallInternal CreateObject %d", 3);
    // 指针引用R0
    BoyiaValue* result = &gVM.mReg0;
    // 设置result的值
    ValueCopy(result, value);
    // 拷贝出新的内部实现
    BoyiaFunction* newFunc = CopyFunction(value, NUM_FUNC_PARAMS);
    result->mValue.mObj.mPtr = (LIntPtr) newFunc;
    result->mValue.mObj.mSuper = value->mValue.mObj.mSuper;
    EngineLog("HandleCallInternal CreateObject %d", 4);

    GCAppendRef(newFunc, CLASS);
    return 1;
}

LVoid ValueCopyNoName(BoyiaValue* dest, BoyiaValue* src) {
	dest->mValueType = src->mValueType;
	switch (src->mValueType) {
		case INT:
		case CHAR:
		case NAVCLASS:
			dest->mValue.mIntVal = src->mValue.mIntVal;
			break;
		case FUNC:
			dest->mValue.mObj.mPtr = src->mValue.mObj.mPtr;
			break;
		case CLASS: {
				dest->mValue.mIntVal = src->mValue.mIntVal;
				dest->mValue.mObj.mSuper = src->mValue.mObj.mSuper;
			}
			break;
		case STRING:
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

static LInt HandleBreak(LVoid* ins) {
    EngineLog("HandleBreak mLoopSize=%d \n", gEState.mLoopSize);
    gEState.mPC = (Instruction*) gLoopStack[--gEState.mLoopSize];
    //EngineLog("HandleBreak end=%d \n", gEState.mPC->mNext->mOPCode);
    return 1;
}

static LVoid BreakStatement() {
    EngineLog("BreakStatement inst code=%d \n", 1);
    PutInstruction(NULL, NULL, BREAK, HandleBreak);
}

static LInt HandleCreateProp(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;
	BoyiaFunction* func = (BoyiaFunction*)gEState.mClass->mValue.mObj.mPtr;
	func->mParams[func->mParamSize++].mNameKey = (LUint)inst->mOPLeft.mValue;
	return 1;
}

static LVoid PropStatement() {
    NextToken();
    //EngineStrLog("PropStatement name=%s", gToken.mTokenName);
    if (gToken.mTokenType == IDENTIFIER) {
		OpCommand cmd = {OP_CONST_NUMBER, (LIntPtr) GenIdentifier(&gToken.mTokenName) };
		PutInstruction(&cmd, NULL, PROP_CREATE, HandleCreateProp);
		Putback();
		EvalExpression();

		if (gToken.mTokenValue != SEMI) {
			SntxErrorBuild(SEMI_EXPECTED);
		}
	} else {
		SntxErrorBuild(SYNTAX);
	}
}

LVoid LocalPush(BoyiaValue* value) {
	if (gEState.mLValSize > NUM_LOCAL_VARS) {
		SntxError(TOO_MANY_LVARS, gEState.mPC->mCodeLine);
	}

	ValueCopy(gLocalsStack + (gEState.mLValSize++), value);
}

static BoyiaValue* FindGlobal(LUint key) {
	for (LInt idx = 0; idx < gEState.mGValSize; ++idx) {
		if (gGlobalsTable[idx].mNameKey == key)
			return &gGlobalsTable[idx];
	}

	return NULL;
}

/* Find the value of a variable. */
static BoyiaValue* GetVal(LUint key) {
	/* First, see if has obj scope */
	if (key == gThis) {
		return gEState.mClass;
	}

	if (key == gSuper) {
		return gEState.mClass ? (BoyiaValue*)gEState.mClass->mValue.mObj.mSuper : NULL;
	}

	/* second, see if it's a local variable */
	LInt start = gCallStack[gEState.mFunctos - 1].mLValSize;
	LInt idx = gEState.mLValSize - 1;
	// idx>localLen而不是idx>=localLen，原因则是，第一个元素实际上是函数变量本身
	for (; idx > start; --idx) {
		if (gLocalsStack[idx].mNameKey == key)
			return &gLocalsStack[idx];
	}

	/* otherwise, try global vars */
	BoyiaValue* val = FindGlobal(key);
	if (val) { return val; }

	return FindObjProp(gEState.mClass, key);
}

static BoyiaValue* FindVal(LUint key) {
	BoyiaValue* value = GetVal(key);
	if (!value) {
		SntxError(NOT_VAR, gEState.mPC->mCodeLine);
	}

	return value;
}

static BoyiaValue* GetLeftOpValue(Instruction* inst) {
	BoyiaValue* left = NULL;
	switch (inst->mOPLeft.mType) { // 赋值左值不可能是常量
	case OP_REG0:
		left = &gVM.mReg0;
		break;
	case OP_REG1:
		left = &gVM.mReg1;
		break;
	case OP_VAR:
		left = FindVal((LUint)inst->mOPLeft.mValue);
		break;
	}

	return left;
}

static LInt HandleCallInternal(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;

	LInt idx = inst->mOPLeft.mValue;
	EngineLog("HandleCallInternal Exec idx=%d", idx);
	return (*gNativeFunTable[idx].mAddr)();
}

static LInt HandleTempLocalSize(LVoid* ins) {
	gCallStack[gEState.mFunctos].mTmpLValSize = gEState.mTmpLValSize;
	gEState.mTmpLValSize = gEState.mLValSize;
	return 1;
}

static LInt HandlePushScene(LVoid* ins) {
	if (gEState.mFunctos >= FUNC_CALLS) {
		SntxError(NEST_FUNC, gEState.mPC->mCodeLine);
		return 0;
	}

	Instruction* inst = (Instruction*)ins;
	gCallStack[gEState.mFunctos].mLValSize = gEState.mTmpLValSize;
	gCallStack[gEState.mFunctos].mPC = (Instruction*)inst->mOPLeft.mValue;
	gCallStack[gEState.mFunctos].mContext = gEState.mContext;
	gCallStack[gEState.mFunctos++].mLoopSize = gEState.mLoopSize;

	return 1;
}

static LInt HandlePushArg(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;
	BoyiaValue* value = GetLeftOpValue(inst);
	LocalPush(value);
	return 1;
}

static LInt HandlePushObj(LVoid* ins) {
	gCallStack[gEState.mFunctos].mClass = gEState.mClass;
	Instruction* inst = (Instruction*)ins;

	if (inst->mOPLeft.mType == OP_VAR) {
		LUint objKey = (LUint) inst->mOPLeft.mValue;
		if (objKey != gSuper) {
			gEState.mClass = (BoyiaValue*)gVM.mReg0.mNameKey;
		}
	} else {
		gEState.mClass = NULL;
	}

	return 1;
}

static LVoid ElseStatement() {
	Instruction* logicInst = PutInstruction(NULL, NULL, ELSE, NULL);
	BlockStatement();
	Instruction* endInst = PutInstruction(NULL, NULL, IF_END, NULL);
	logicInst->mOPRight.mType = OP_CONST_NUMBER;
	logicInst->mOPRight.mValue = (LIntPtr) endInst; // 最后地址值
}

static LInt HandleReturn(LVoid* ins) {
    gEState.mPC = gEState.mContext->mEnd;
    return 1;
}

static LVoid ReturnStatement() {
    EvalExpression(); // => R0
    PutInstruction(NULL, NULL, RETURN, HandleReturn);
}

static LVoid BlockStatement() {
    LBool block = LFalse;
    do {
        NextToken();
        //EngineStrLog("BlockStatement name=%s", gToken.mTokenName);
        /* If interpreting single statement, return on first semicolon. */
        /* see what kind of token is up */
        if (gToken.mTokenType == IDENTIFIER) {
            /* Not a keyword, so process expression. */
            Putback();   /* restore token to input stream for further processing by EvalExpression() */
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
        } else if (gToken.mTokenType == KEYWORD) {/* is keyword */
            switch (gToken.mTokenValue) {
                case VAR:
                    LocalStatement();
                    break;
                case FUNC:
                    FunStatement();
                    break;
                case PROP:
                    PropStatement();
                    break;
                case RETURN:         /* return from function call */
                    ReturnStatement();
                    break;
                case IF:             /* process an if statement */
                case ELIF:
                	IfStatement();
                    break;
                case ELSE:           /* process an else statement */
                	ElseStatement();
                	break;
                case WHILE:          /* process a while loop */
                    WhileStatement();
                    break;
                case DO:             /* process a do-while loop */
                    DoStatement();
                    break;
                case BREAK:
                    EngineLog("BREAK BreakStatement %d \n", 1);
                    BreakStatement();
                    break;
            }
        }
    } while (gToken.mTokenValue != END && block);
}

static LVoid SkipComment() {
    if (*gEState.mProg == '/') {
        if (*(gEState.mProg + 1) == '*') { // 多行注释
            gEState.mProg += 2;
            do {
                while (*gEState.mProg != '*') {
					if (*gEState.mProg == '\n') {
						++gEState.mLineNum;
					}
                    ++gEState.mProg;
                }
                ++gEState.mProg;
            } while (*gEState.mProg != '/');
            ++gEState.mProg;
			SkipComment();
        } else if (*(gEState.mProg + 1) == '/') {  //单行注释
			while (*gEState.mProg && *gEState.mProg != '\n') { 
				++gEState.mProg;
			}

			if (*gEState.mProg++ == '\n') {
				++gEState.mLineNum;
			}
			
			SkipComment();
		}
    }
}

static LInt GetLogicValue() {
    if (MStrchr("&|!<>=", *gEState.mProg)) {
        LInt len = 0;
        gToken.mTokenName.mPtr = gEState.mProg;
        switch (*gEState.mProg) {
            case '=':
                if (*(gEState.mProg + 1) == '=') {
                    gEState.mProg += 2;
                    len += 2;
                    gToken.mTokenValue = EQ;
                }
                break;
            case '!':
                if (*(gEState.mProg + 1) == '=') {
                    gEState.mProg += 2;
                    len += 2;
                    gToken.mTokenValue = NE;
                } else {
                	++gEState.mProg;
                    len = 1;
                    gToken.mTokenValue = NOT;
                }
                break;
            case '<':
                if (*(gEState.mProg + 1) == '=') {
                    gEState.mProg += 2;
                    len += 2;
                    gToken.mTokenValue = LE;
                } else {
                    ++gEState.mProg;
                    len = 1;
                    gToken.mTokenValue = LT;
                }
                break;
            case '>':
                if (*(gEState.mProg + 1) == '=') {
                    gEState.mProg += 2;
                    len += 2;
                    gToken.mTokenValue = GE;
                } else {
                    gEState.mProg++;
                    len = 1;
                    gToken.mTokenValue = GT;
                }
                break;
            case '&':
            	if (*(gEState.mProg + 1) == '&') {
            	    gEState.mProg += 2;
            	    len += 2;
            	    gToken.mTokenValue = AND;
            	}
            	break;
            case '|':
            	if (*(gEState.mProg + 1) == '|') {
            	    gEState.mProg += 2;
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

static LInt GetDelimiter() {
    const char* delimiConst = "+-*/%^=;,'.()[]{}";
    LInt op = ADD;
    do {
        if (*delimiConst == *gEState.mProg) {
            gToken.mTokenValue = op;
            gToken.mTokenName.mPtr = gEState.mProg;
            gToken.mTokenName.mLen = 1;
            ++gEState.mProg;
            return (gToken.mTokenType = DELIMITER);
        }
        ++op;
        ++delimiConst;
    } while (*delimiConst);

    return 0;
}

static LInt GetIdentifer() {
    LInt len = 0;
    if (LIsAlpha(*gEState.mProg)) {
        gToken.mTokenName.mPtr = gEState.mProg;
        while (*gEState.mProg == '_' ||
               LIsAlpha(*gEState.mProg) ||
               LIsDigit(*gEState.mProg)) {
            ++gEState.mProg;++len;
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

static LInt GetStringValue() {
    // string
    LInt len = 0;
    if (*gEState.mProg == '"') {
        ++gEState.mProg;
        gToken.mTokenName.mPtr = gEState.mProg;
        while (*gEState.mProg != '"' && *gEState.mProg != '\r') {
        	++gEState.mProg;++len;
        }

        if (*gEState.mProg == '\r') {
        	SntxErrorBuild(SYNTAX);
        }
        ++gEState.mProg;
        // +2 for putback just in case
        gToken.mTokenName.mLen = len+2;
        return (gToken.mTokenType = STRING_VALUE);
    }

    return 0;
}

static LInt GetNumberValue() {
    LInt len = 0;
    if (LIsDigit(*gEState.mProg)) {
        gToken.mTokenName.mPtr = gEState.mProg;

        while (LIsDigit(*gEState.mProg)) {
        	++gEState.mProg;++len;
        }
        gToken.mTokenName.mLen = len;
        return (gToken.mTokenType = NUMBER);
    }

    return 0;
}

static LInt NextToken() {
    gToken.mTokenType = 0;
    gToken.mTokenValue = 0;
    InitStr(&gToken.mTokenName, NULL);
    while (LIsSpace(*gEState.mProg) && *gEState.mProg) {
		if (*gEState.mProg == '\n') {
			++gEState.mLineNum;
		}
        ++gEState.mProg;
    }

    if (*gEState.mProg == '\0') {
        gToken.mTokenValue = END;
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

static LInt HandleCreateParam(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;
	LUint hashKey = (LUint)inst->mOPLeft.mValue;

	BoyiaFunction* function = &gFunTable[gEState.mFunSize-1];
	BoyiaValue* value = &function->mParams[function->mParamSize++];
	value->mNameKey = hashKey;
	return 1;
}

static LVoid InitParams() {
    do {
        NextToken(); // 得到属性名
        if (gToken.mTokenValue != RPTR) {
			OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr) GenIdentifier(&gToken.mTokenName) };
			PutInstruction(&cmd, NULL, PARAM_CREATE, HandleCreateParam);
			NextToken(); // 获取逗号分隔符','
        } else
            break;
    } while (gToken.mTokenValue == COMMA);
    if (gToken.mTokenValue != RPTR)
    	SntxErrorBuild(PAREN_EXPECTED);
}

static CommandTable* CreateExecutor() {
    CommandTable* newTable = NEW(CommandTable);
    newTable->mBegin = NULL;
    newTable->mEnd = NULL;
    return newTable;
}

static LVoid InitFunction(BoyiaFunction* fun) {
    fun->mParamSize = 0;
    fun->mParams = NEW_ARRAY(BoyiaValue, NUM_FUNC_PARAMS);
	fun->mParamCount = NUM_FUNC_PARAMS;
	++gEState.mFunSize;
}

static BoyiaValue* CreateFunVal(LUint hashKey, LUint8 type) {
    // 初始化class类或函数变量
    BoyiaValue* val = &gGlobalsTable[gEState.mGValSize++];
    BoyiaFunction* fun = &gFunTable[gEState.mFunSize];
    val->mValueType = type;
    val->mNameKey = hashKey;
    val->mValue.mObj.mPtr = (LIntPtr) fun;
    val->mValue.mObj.mSuper = 0;
    if (type == CLASS) {
    	fun->mFuncBody = (LIntPtr) val;
    }
    // 初始化类属性成员列表
    InitFunction(fun);
    return val;
}

static LInt HandleCreateExecutor(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;
    gFunTable[gEState.mFunSize-1].mFuncBody = inst->mOPLeft.mValue;
	return 1;
}

static LVoid BodyStatement(LInt type) {
	CommandTable* cmds = gEState.mContext;
	if (FUN_CREATE == type) {
		// 类成员的创建在主体上下中进行
		CommandTable* funCmds = CreateExecutor();
		OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr) funCmds };
		PutInstruction(&cmd, NULL, EXE_CREATE, HandleCreateExecutor);

		gEState.mContext = funCmds;
	}

    BlockStatement();
	gEState.mContext = cmds;
}

static LInt HandleCreateClass(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;
	if (inst->mOPLeft.mType == OP_NONE) {
		gEState.mClass = NULL;
		return 1;
	}
	LUint hashKey = (LUint)inst->mOPLeft.mValue;
	gEState.mClass = CreateFunVal(hashKey, CLASS);
	return 1;
}

static LInt HandleExtend(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;
	BoyiaValue* classVal = FindGlobal((LUint)inst->mOPLeft.mValue);
	BoyiaValue* extendVal = FindGlobal((LUint)inst->mOPRight.mValue);

    // 设置super指针
	classVal->mValue.mObj.mSuper = (LIntPtr) extendVal;
	return 1;
}

static LVoid ClassStatement() {
    NextToken();
	LUint classKey = GenIdentifier(&gToken.mTokenName);
	OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr) classKey };
	PutInstruction(&cmd, NULL, CLASS_CREATE, HandleCreateClass);
    // 判断继承关系
	NextToken();
	LUint extendKey = 0;
	if (EXTEND == gToken.mTokenValue) {
		NextToken();
		extendKey = GenIdentifier(&gToken.mTokenName);
	} else {
		Putback();
	}
	// 初始化类体
    BodyStatement(CLASS_CREATE);
	// 设置继承成员
	if (extendKey != 0) {
		OpCommand extendCmd = { OP_CONST_NUMBER, (LIntPtr) extendKey };
		PutInstruction(&cmd, &extendCmd, HANDLE_EXTEND, HandleExtend);
	}

	// 执行完后需将CLASS置为NULL
	OpCommand cmdEnd = { OP_NONE, 0 };
	PutInstruction(&cmdEnd, NULL, CLASS_CREATE, HandleCreateClass);
}

static LInt HandleFunCreate(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;
	LUint hashKey = (LUint)inst->mOPLeft.mValue;

	if (gEState.mClass) {
		BoyiaFunction* func = (BoyiaFunction*)gEState.mClass->mValue.mObj.mPtr;
		func->mParams[func->mParamSize].mNameKey = hashKey;
		func->mParams[func->mParamSize].mValueType = FUNC;
		func->mParams[func->mParamSize++].mValue.mObj.mPtr = (LIntPtr) &gFunTable[gEState.mFunSize];
		// 初始化函数参数列表
		InitFunction(&gFunTable[gEState.mFunSize]);
	} else {
		CreateFunVal(hashKey, FUNC);
	}

	return 1;
}

static LVoid FunStatement() {
    NextToken();
    //EngineStrLog("HandlePushParams FunStatement name %s", gToken.mTokenName);
    // 第一步，Function变量
	OpCommand cmd = { OP_CONST_NUMBER, (LIntPtr)GenIdentifier(&gToken.mTokenName) };
	PutInstruction(&cmd, NULL, FUN_CREATE, HandleFunCreate);
    //EngineStrLog("FunctionName=%s", gToken.mTokenName);
    // 第二步，初始化函数参数
    NextToken(); //   '(', 即LPTR
    InitParams(); //  初始化参数
    // 第三步，函数体内部编译
    BodyStatement(FUN_CREATE);
}

static LVoid DeleteExecutor(CommandTable* table) {
	Instruction* pc = table->mBegin;
	while (pc != table->mEnd) {
		Instruction* next = pc->mNext;
		DELETE(pc);
		pc = next;
	}

	DELETE(table);
}

// 重置现场
static LVoid ResetScene() {
    gEState.mLValSize = 0;    /* initialize local variable stack index */
    gEState.mFunctos = 0;    /* initialize the CALL stack index */
    gEState.mLoopSize = 0;
    gEState.mResultNum = 0;
	gEState.mTmpLValSize = 0;
    gEState.mClass = NULL;
}

// 执行全局的调用
static LVoid ExecuteCode(CommandTable* cmds) {
	gEState.mContext = cmds;
	gEState.mPC = gEState.mContext->mBegin;
	ExecInstruction();
	// 删除执行体
	DeleteExecutor(cmds);
	ResetScene();
}

static LInt HandleDeclGlobal(LVoid* ins) {
	Instruction* inst = (Instruction*) ins;
	BoyiaValue val;
	val.mValueType = inst->mOPLeft.mValue;
	val.mNameKey = (LUint) inst->mOPRight.mValue;
	ValueCopy(gGlobalsTable + gEState.mGValSize++, &val);
	return 1;
}

static LVoid GlobalStatement() {
    LInt type = gToken.mTokenValue;
    do {
        NextToken(); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr) GenIdentifier(&gToken.mTokenName) };

        PutInstruction(&cmdLeft, &cmdRight, DECL_GLOBAL, HandleDeclGlobal);
		Putback();
		EvalExpression();
    } while (gToken.mTokenValue == COMMA);

    if (gToken.mTokenValue != SEMI) {
    	SntxErrorBuild(SEMI_EXPECTED);
    }
}

// 该函数记录全局变量以及函数接口
static LVoid ParseStatement() {
    LInt brace = 0;   // ‘{’的个数
    gEState.mContext = CreateExecutor();
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
        if (gToken.mTokenValue == VAR) {
            GlobalStatement();
        } else if (gToken.mTokenValue == FUNC) {
			FunStatement();
        } else if (gToken.mTokenValue == CLASS) {
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
    } while (gToken.mTokenValue != END);

	// 执行全局声明和定义
	ExecuteCode(gEState.mContext);
}

static LInt HandleDeclLocal(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue local;
    local.mValueType = inst->mOPLeft.mValue;
    local.mNameKey = (LUint) inst->mOPRight.mValue;
    LocalPush(&local);                              // 塞入本地符号表
    return 1;
}

/* Declare a local variable. */
static LVoid LocalStatement() {
    LInt type = gToken.mTokenValue;
    do {
        NextToken(); /* get ident */
        OpCommand cmdLeft = { OP_CONST_NUMBER, type };
        OpCommand cmdRight = { OP_CONST_NUMBER, (LIntPtr) GenIdentifier(&gToken.mTokenName) };
        //EngineStrLog("value Name=%s", gToken.mTokenName);
        PutInstruction(&cmdLeft, &cmdRight, DECL_LOCAL, HandleDeclLocal);
		Putback();
		EvalExpression();
    } while (gToken.mTokenValue == COMMA);

    if (gToken.mTokenValue != SEMI) {
    	SntxErrorBuild(SEMI_EXPECTED);
    }
}

static LInt HandleCallFunction(LVoid* ins) {
    //EngineLog("HandleFunction begin %d \n", 1);
	// localstack第一个值为函数指针
    LInt start = gCallStack[gEState.mFunctos - 1].mLValSize;
    BoyiaValue* value = &gLocalsStack[start];
    BoyiaFunction* func = (BoyiaFunction*) value->mValue.mObj.mPtr;

    gEState.mContext = (CommandTable*) func->mFuncBody;
    gEState.mPC = gEState.mContext->mBegin;
    return 2;
}

static LInt HandlePushParams(LVoid* ins) {
    // 第一个参数为调用该函数的函数指针
    LInt start = gCallStack[gEState.mFunctos - 1].mLValSize;
    BoyiaValue* value = &gLocalsStack[start];
    EngineLog("HandlePushParams functionName=%u \n", value->mValueType);
    if (value->mValueType == FUNC) {
        BoyiaFunction* func = (BoyiaFunction*) value->mValue.mObj.mPtr;
        if (func->mParamSize <= 0) {
            return 1;
        }
        // 从第二个参数开始，将形参key赋给实参
        LInt idx = start + 1;
        LInt end = idx + func->mParamSize;
        for (; idx < end; ++idx) {
            LUint vKey = func->mParams[idx - start - 1].mNameKey;
            gLocalsStack[idx].mNameKey = vKey;
        }
    }

    return 1;
}

static LInt HandlePop(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
	if (inst->mOPLeft.mType != OP_REG0 && inst->mOPLeft.mType != OP_REG1) {
		--gEState.mResultNum;
		return 1;
	}
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &gVM.mReg0 : &gVM.mReg1;
    ValueCopy(value, gOpStack + (--gEState.mResultNum));
    return 1;
}

/* Call a function. */
static void CallStatement(OpCommand* objCmd) {
    PutInstruction(NULL, NULL, TMP_LOCAL, HandleTempLocalSize);
    // 设置参数
    PushArgStatement();
    // POP CLASS context
	if (objCmd->mType == OP_VAR) {
		PutInstruction(&COMMAND_R0, NULL, POP, HandlePop);
	}
	// 保存对象环境
	PutInstruction(objCmd, NULL, 0, HandlePushObj);
	// 保存调用堆栈
    Instruction* pushInst = PutInstruction(NULL, NULL, PUSH_SCENE, HandlePushScene);
    // 函数形参名哈希值赋给局部变量
    PutInstruction(NULL, NULL, PUSH_PARAMS, HandlePushParams);
    // 执行函数体
    Instruction* funInst = PutInstruction(NULL, NULL, CALL, HandleCallFunction);
    //EngineLog("CallStatement=>%d HandleFunction", 1);
    pushInst->mOPLeft.mType = OP_CONST_NUMBER;
    pushInst->mOPLeft.mValue = (LIntPtr) funInst;
}

/* Push the arguments to a function onto the local variable stack. */
static LVoid PushArgStatement() {
    // push函数指针
    PutInstruction(&COMMAND_R0, NULL, PUSH_ARG, HandlePushArg);
    NextToken();// if token == ')' exit
    if (gToken.mTokenValue == RPTR) {
        return;
    } else {
        Putback();
    }

    do {
        // 参数值在R0中
        EvalExpression(); // => R0
        // 将函数实参压栈
        PutInstruction(&COMMAND_R0, NULL, PUSH_ARG, HandlePushArg);
        //NextToken();
    } while (gToken.mTokenValue == COMMA);
}

static BoyiaValue* GetRightOpValue(Instruction* inst) {
    BoyiaValue* right = NULL;
    switch (inst->mOPRight.mType) { // 赋值左值不可能是常量
        case OP_REG0:
            right = &gVM.mReg0;
            break;
        case OP_REG1:
            right = &gVM.mReg1;
            break;
        case OP_VAR:
            right = FindVal((LUint) inst->mOPRight.mValue);
            break;
    }

    return right;
}

/* Assign a value to a Register 0 or 1. */
static LInt HandleAssignment(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    if (!left) return 0;

    switch (inst->mOPRight.mType) {
        case OP_CONST_STRING: { // 字符串是存在全局表中
            BoyiaValue* val = (BoyiaValue*) inst->mOPRight.mValue;
            ValueCopyNoName(left, val);
        }
            break;
        case OP_CONST_NUMBER: {
            left->mValueType = INT;
            left->mValue.mIntVal = inst->mOPRight.mValue;
        }
            break;
        case OP_VAR: {
            BoyiaValue *val = FindVal((LUint) inst->mOPRight.mValue);
            ValueCopyNoName(left, val);
            left->mNameKey = (LUintPtr) val;
        }
            break;
        case OP_REG0: {
        	ValueCopyNoName(left, &gVM.mReg0);
        }
            break;
        case OP_REG1: {
        	ValueCopyNoName(left, &gVM.mReg1);
        }
            break;
    }

    return 1;
}

// 执行到ifend证明整个if elseif, else执行完毕，
// 无需检索是否还有elseif，或者else的逻辑判断和内部block
static LInt HandleIfEnd(LVoid* ins) {
    Instruction* inst = gEState.mPC;
    Instruction* tmpInst = inst->mNext;
    // 查看下一个是否是elseif
    EngineLog("HandleIfEnd R0=>%d \n", 1);
    while (tmpInst && (tmpInst->mOPCode == ELIF || tmpInst->mOPCode == ELSE)) {
        inst = (Instruction*) tmpInst->mOPRight.mValue; // 跳转到elif对应的IFEND
        tmpInst = inst->mNext;
    }
    EngineLog("HandleIfEnd END R0=>%d \n", 1);
    if (inst) gEState.mPC = inst;
    return 1;
}

static LInt HandleJumpToIfTrue(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* value = &gVM.mReg0;
    if (!value->mValue.mIntVal) {
        gEState.mPC = (Instruction*) inst->mOPRight.mValue;
    }

    return 1;
}

/* Execute an if statement. */
static LVoid IfStatement() {
	NextToken();
    // token = (
    EvalExpression();  /* check the conditional expression => R0 */
    Instruction* logicInst = PutInstruction(&COMMAND_R0, NULL, JMP_TRUE, HandleJumpToIfTrue);
    //EngineStrLog("endif last inst name=%s", gToken.mTokenName);
    BlockStatement();  /* if true, interpret */
    Instruction* endInst = PutInstruction(NULL, NULL, IF_END, HandleIfEnd);
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue= (LIntPtr) endInst; // 最后地址值
}

static LInt HandleLoopBegin(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    // push left => loop stack
    gLoopStack[gEState.mLoopSize++] = inst->mOPLeft.mValue;
    return 1;
}

static LInt HandleLoopIfTrue(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* value = &gVM.mReg0;
    //EngineLog("HandleLoopIfTrue value=%d", value->mValue.mIntVal);
    if (!value->mValue.mIntVal) {
        gEState.mPC = (Instruction*) inst->mOPRight.mValue;
        gEState.mLoopSize--;
    }

    return 1;
}

static LInt HandleJumpTo(LVoid* ins) {
    //EngineLog("HandleJumpTo %d", 1);
    Instruction* inst = (Instruction*) ins;
    if (inst->mOPLeft.mType == OP_CONST_NUMBER) {
        gEState.mPC = (Instruction*) inst->mOPLeft.mValue;
    }
    //EngineLog("HandleJumpTo %d", 2);
    return 1;
}

/* Execute a while loop. */
static LVoid WhileStatement() {
    //EngineLog("WhileStatement %d", 0);
    Instruction* beginInst = PutInstruction(NULL, NULL, LOOP, HandleLoopBegin);
    NextToken(); // '('
    if (gToken.mTokenValue != LPTR) {
    	SntxErrorBuild(LPTR_EXPECTED);
    }
    EvalExpression();  /* check the conditional expression => R0 */
    if (gToken.mTokenValue != RPTR) {
    	SntxErrorBuild(RPTR_EXPECTED);
    }
    //EngineStrLog("WhileStatement last inst name=%s", gToken.mTokenName);
    Instruction* logicInst = PutInstruction(&COMMAND_R0, NULL, LOOP_TRUE, HandleLoopIfTrue);
    BlockStatement();  /* if true, interpret */
    Instruction* endInst = PutInstruction(NULL, NULL, JMP, HandleJumpTo);
    beginInst->mOPLeft.mType = OP_CONST_NUMBER;
    beginInst->mOPLeft.mValue = (LIntPtr) endInst; // 最后地址值
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr) endInst; // 最后地址值
    endInst->mOPLeft.mType = OP_CONST_NUMBER;
    endInst->mOPLeft.mValue = (LIntPtr) beginInst; // LOOP开始地址值
}

/* Execute a do loop. */
static LVoid DoStatement() {
    Instruction* beginInst = PutInstruction(NULL, NULL, LOOP, HandleLoopBegin);
    BlockStatement(); /* interpret loop */
    NextToken();
    if (gToken.mTokenValue != WHILE) {
    	SntxErrorBuild(WHILE_EXPECTED);
    }
    EvalExpression();  /* check the loop condition */

	if (gToken.mTokenValue != SEMI) {
		SntxErrorBuild(SEMI_EXPECTED);
	}
    Instruction* logicInst = PutInstruction(&COMMAND_R0, NULL, LOOP_TRUE, HandleLoopIfTrue);
    Instruction* endInst = PutInstruction(NULL, NULL, JMP, HandleJumpTo);
    beginInst->mOPLeft.mType = OP_CONST_NUMBER;
    beginInst->mOPLeft.mValue = (LIntPtr) endInst; // 最后地址值
    logicInst->mOPRight.mType = OP_CONST_NUMBER;
    logicInst->mOPRight.mValue = (LIntPtr) endInst; // 最后地址值
    endInst->mOPLeft.mType = OP_CONST_NUMBER;
    endInst->mOPLeft.mValue = (LIntPtr) beginInst; // LOOP开始地址值
}

static LInt HandleAdd(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    BoyiaValue* right = GetRightOpValue(inst);
    //EngineLog("HandleAdd left=%d \n", left->mValue.mIntVal);
    //EngineLog("HandleAdd right=%d \n", right->mValue.mIntVal);
	if (left->mValueType == INT && right->mValueType == INT) {
		right->mValue.mIntVal = left->mValue.mIntVal + right->mValue.mIntVal;
		EngineLog("HandleAdd result=%d \n", right->mValue.mIntVal);
		return 1;
	}

	if (left->mValueType == STRING || right->mValueType == STRING) {
		EngineLog("StringAdd Begin %d", 1);
		StringAdd(left, right);
		return 1;
	}
   
    return 0;
}

static LInt HandleSub(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    BoyiaValue* right = GetRightOpValue(inst);

    if (left->mValueType != INT || right->mValueType != INT)
        return 0;

    right->mValue.mIntVal = left->mValue.mIntVal - right->mValue.mIntVal;
    EngineLog("HandleSub R0=>%d \n", gVM.mReg0.mValue.mIntVal);
    return 1;
}

static LInt HandleMul(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    BoyiaValue* right = GetRightOpValue(inst);

    EngineLog("HandleMul left=%d \n", left->mValue.mIntVal);
    EngineLog("HandleMul right=%d \n", right->mValue.mIntVal);
    if (left->mValueType != INT || right->mValueType != INT)
        return 0;

    left->mValue.mIntVal = left->mValue.mIntVal * right->mValue.mIntVal;
    EngineLog("HandleMul result=%d \n", left->mValue.mIntVal);
    return 1;
}

static LInt HandleDiv(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    BoyiaValue* right = GetRightOpValue(inst);

    if (left->mValueType != INT || right->mValueType != INT)
        return 0;

    if (right->mValue.mIntVal == 0)
        return 0;

    left->mValue.mIntVal = left->mValue.mIntVal / right->mValue.mIntVal;
    return 1;
}

static LInt HandleMod(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    BoyiaValue* right = GetRightOpValue(inst);

    if (left->mValueType != INT || right->mValueType != INT)
        return 0;

    if (right->mValue.mIntVal == 0)
        return 0;

    left->mValue.mIntVal = left->mValue.mIntVal % right->mValue.mIntVal;
    return 1;
}

static LInt HandleRelational(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    BoyiaValue* right = GetRightOpValue(inst);

    EngineLog("HandleLogic left=%d \n", left->mValue.mIntVal);
    EngineLog("HandleLogic right=%d \n", right->mValue.mIntVal);

    LInt result = 0;
    switch (inst->mOPCode) {
        case LT:
            result = left->mValue.mIntVal < right->mValue.mIntVal ? 1 : 0;
            break;
        case LE:
            result = left->mValue.mIntVal <= right->mValue.mIntVal ? 1 : 0;
            break;
        case GT:
            result = left->mValue.mIntVal > right->mValue.mIntVal ? 1 : 0;
            break;
        case GE:
            result = left->mValue.mIntVal >= right->mValue.mIntVal ? 1 : 0;
            break;
        case EQ:
            result = left->mValue.mIntVal == right->mValue.mIntVal ? 1 : 0;
            break;
        case NE:
            result = left->mValue.mIntVal != right->mValue.mIntVal ? 1 : 0;
            break;
    }

    right->mValueType = INT;
    right->mValue.mIntVal = result;
    return 1;
}

static LInt HandleLogic(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    BoyiaValue* right = GetRightOpValue(inst);

    LInt result = 0;
    switch (inst->mOPCode) {
        case AND:
            result = left->mValue.mIntVal && right->mValue.mIntVal ? 1 : 0;
            break;
        case OR:
            result = left->mValue.mIntVal || right->mValue.mIntVal ? 1 : 0;
            break;
        case NOT:
            result = left->mValue.mIntVal ? 0 : 1;
            break;
    }

    right->mValueType = INT;
    right->mValue.mIntVal = result;
    return 1;
}

/* parser lib define */
static LVoid EvalExpression() {
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

static LInt HandlePush(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* value = inst->mOPLeft.mType == OP_REG0 ? &gVM.mReg0 : &gVM.mReg1;
    ValueCopy(gOpStack + (gEState.mResultNum++), value);
    return 1;
}

static LInt HandleAssignVar(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
    BoyiaValue* left = GetLeftOpValue(inst);
    BoyiaValue* value = (BoyiaValue*) left->mNameKey;
    BoyiaValue* result = GetRightOpValue(inst);

    ValueCopyNoName(value, result);
    ValueCopy(&gVM.mReg0, value);
    return 1;
}

static LVoid CallNativeStatement(LInt idx) {
    NextToken();
    if (gToken.mTokenValue != LPTR) // '('
    	SntxErrorBuild(PAREN_EXPECTED);

    PutInstruction(NULL, NULL, TMP_LOCAL, HandleTempLocalSize);
    do {
        // 参数值在R0中
        EvalExpression(); // => R0
        // 将函数参数压栈
        PutInstruction(&COMMAND_R0, NULL, PUSH_ARG, HandlePushArg);
        // if token == ')' exit
    } while (gToken.mTokenValue == COMMA);

	// 保存obj现场是必不可少的一步
	OpCommand cmdSet = { OP_CONST_NUMBER, 0 };
	PutInstruction(&cmdSet, NULL, PUSH_OBJ, HandlePushObj);

    Instruction* pushInst = PutInstruction(NULL, NULL, PUSH_SCENE, HandlePushScene);
    OpCommand cmd = { OP_CONST_NUMBER, idx };
    PutInstruction(&cmd, NULL, CALL, HandleCallInternal);
    Instruction* popInst = PutInstruction(NULL, NULL, POP_SCENE, HandlePopScene);
    pushInst->mOPLeft.mType = OP_CONST_NUMBER;
    pushInst->mOPLeft.mValue = (LIntPtr) popInst;
}

static BoyiaValue* FindObjProp(BoyiaValue* lVal, LUint rVal) {
	if (!lVal || lVal->mValueType != CLASS) {
		return NULL;
	}

	// find props, such as obj.prop1.
	BoyiaFunction* fun = (BoyiaFunction*)lVal->mValue.mObj.mPtr;
	LInt idx = 0;
	for (; idx < fun->mParamSize; ++idx) {
		if (fun->mParams[idx].mNameKey == rVal) {
			return fun->mParams + idx;
		}
	}

	// find function, such as obj.func1
	BoyiaValue* cls = (BoyiaValue*) fun->mFuncBody;
	while (cls && cls->mValueType == CLASS) {
		BoyiaFunction* clsMap = (BoyiaFunction*)cls->mValue.mObj.mPtr;
		LInt funIdx = 0;
		for (; funIdx < clsMap->mParamSize; ++funIdx) {
			if (clsMap->mParams[funIdx].mNameKey == rVal) {
				return clsMap->mParams + funIdx;
			}
		}

		cls = (BoyiaValue*) cls->mValue.mObj.mSuper;
	}

	return NULL;
}

static LInt FindProp(BoyiaValue* lVal, LUint rVal) {
	BoyiaValue* propVal = FindObjProp(lVal, rVal);
	// 找到
	if (propVal) {
		gVM.mReg0.mNameKey = (LUint)propVal;
		gVM.mReg0.mValue = propVal->mValue;
		gVM.mReg0.mValueType = propVal->mValueType; // maybe function
		return 1;
	}
	return 0;
}

static LInt HandleGetProp(LVoid* ins) {
    Instruction* inst = (Instruction*) ins;
	BoyiaValue* lVal = GetLeftOpValue(inst);
	LUint rVal = (LUint)inst->mOPRight.mValue;
	return FindProp(lVal, rVal);
}

// According to reg0, get reg0 obj's prop
static LVoid EvalGetProp() {
	NextToken();
	if (gToken.mTokenType != IDENTIFIER) {
		return;
	}

	// Push class context for callstatement
	PutInstruction(&COMMAND_R0, NULL, PUSH, HandlePush);
	LUint propKey = GenIdentifier(&gToken.mTokenName);
	OpCommand cmdR = { OP_CONST_NUMBER, (LIntPtr) propKey };
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
		PutInstruction(&cmd, NULL, POP, HandlePop);
		EvalGetProp();
	} else {
		OpCommand cmd = { OP_CONST_NUMBER, 0 };
		PutInstruction(&cmd, NULL, POP, HandlePop);
	}
}

static LVoid EvalGetValue(LUint objKey) {
    OpCommand cmdL = { OP_VAR, (LIntPtr) objKey };
	PutInstruction(&COMMAND_R0, &cmdL, ASSIGN, HandleAssignment);
    if (gToken.mTokenValue == DOT) {
		EvalGetProp();
    }
}

static LVoid CopyStringFromToken(BoyiaStr* str) {
	str->mLen = gToken.mTokenName.mLen - 2;
	str->mPtr = NEW_ARRAY(LInt8, str->mLen);
	LMemcpy(str->mPtr, gToken.mTokenName.mPtr, str->mLen * sizeof(LInt8));
}

static LInt HandleConstString(LVoid* ins) {
	Instruction* inst = (Instruction*)ins;
	gVM.mReg0.mValueType = STRING;
	gVM.mReg0.mValue.mStrVal.mPtr = (LInt8*) inst->mOPLeft.mValue;
	gVM.mReg0.mValue.mStrVal.mLen = inst->mOPRight.mValue;
	return 1;
}

static LVoid Atom() {
    switch (gToken.mTokenType) {
        case IDENTIFIER: {
            LInt idx = FindNativeFunc(GenIdentifier(&gToken.mTokenName));
            if (idx != -1) {
				CallNativeStatement(idx);
                NextToken();
            } else {
                LUint key = GenIdentifier(&gToken.mTokenName);
                NextToken();
                if (gToken.mTokenValue == LPTR) {
                    OpCommand cmd = { OP_VAR, (LIntPtr) key };
                    PutInstruction(&COMMAND_R0, &cmd, ASSIGN, HandleAssignment);
                    OpCommand objCmd = {OP_CONST_NUMBER, 0};
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
        	OpCommand lCmd = { OP_CONST_NUMBER, (LIntPtr) constStr.mPtr };
        	OpCommand rCmd = { OP_CONST_NUMBER, constStr.mLen };
        	PutInstruction(&lCmd, &rCmd, HANDLE_CONST_STR, HandleConstString);
        	NextToken();
        }
            return;
        default: {
            if (gToken.mTokenValue == RPTR) return;
            else SntxErrorBuild(SYNTAX);
        }
            break;
    }
}

static LVoid EvalSubexpr() {
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
static LVoid EvalMinus() {
	LInt8 op = 0;
	if (gToken.mTokenValue == ADD || gToken.mTokenValue == SUB) {
		op = gToken.mTokenValue;
		NextToken();
	}
	EvalSubexpr(); // => R0
	if (op && op == SUB) {
		OpCommand cmd = { OP_CONST_NUMBER, -1 };
		PutInstruction(&COMMAND_R1, &cmd, ASSIGN, HandleAssignment);
		PutInstruction(&COMMAND_R0, &COMMAND_R1, MUL, HandleMul);
	}
}

// 乘除, *,/,%
static LVoid EvalArith() {
	LInt8 op = 0;
	EvalMinus();

	while ((op = gToken.mTokenValue) == MUL || op == DIV || op == MOD) { // * / %
																		 // PUSH R0
		PutInstruction(&COMMAND_R0, NULL, PUSH, HandlePush);
		NextToken();
		EvalMinus();
		// POP R1
		PutInstruction(&COMMAND_R1, NULL, POP, HandlePop);
		switch (op) {
		case MUL:
			PutInstruction(&COMMAND_R0, &COMMAND_R1, op, HandleMul);
			break;
		case DIV:
			PutInstruction(&COMMAND_R0, &COMMAND_R1, op, HandleDiv);
			break;
		case MOD:
			PutInstruction(&COMMAND_R0, &COMMAND_R1, op, HandleMod);
			break;
		}
	}
}

// 加减,+,-
static LVoid EvalAddSub() {
	LInt8 op = 0;
	EvalArith(); // => R0
	while ((op = gToken.mTokenValue) == ADD || op == SUB) {
		// PUSH
		PutInstruction(&COMMAND_R0, NULL, PUSH, HandlePush);

		NextToken();
		EvalArith();
		// POP R1
		PutInstruction(&COMMAND_R1, NULL, POP, HandlePop);
		// R0 + R1 => R0
		PutInstruction(&COMMAND_R1, &COMMAND_R0, op, op == ADD ? HandleAdd : HandleSub);
	}
}

// 关系比较判断,>,<,==,!=
static LVoid EvalRelational() {
	static LInt8 relops[7] = {
		LT, LE, GT, GE, EQ, NE, 0
	};

	EvalAddSub(); // 计算结果 => R0

	LInt8 op = gToken.mTokenValue;
	if (MStrchr(relops, op)) {
		// 计算的结果存入栈中
		PutInstruction(&COMMAND_R0, NULL, PUSH, HandlePush);

		NextToken(); // 查找标识符或者常量
		EvalAddSub(); // 先执行优先级高的操作 => R0
		// pop R1
		// 上次计算的结果出栈至R1
		PutInstruction(&COMMAND_R1, NULL, POP, HandlePop);

		// 计算R0 OP R1, 结果存入R0中
		PutInstruction(&COMMAND_R1, &COMMAND_R0, op, HandleRelational);
	}
}

static LVoid EvalLogic() {
	static LInt8 logicops[4] = {
		AND, OR, NOT, 0
	};

	EvalRelational();
	LInt8 op = 0;
	while (MStrchr(logicops, (op = gToken.mTokenValue))) {
		// 计算的结果存入栈中
	    PutInstruction(&COMMAND_R0, NULL, PUSH, HandlePush);

		NextToken(); // 查找标识符或者常量
		EvalRelational(); // 先执行优先级高的操作 => R0
		// pop R1
		// 上次计算的结果出栈至R1
		PutInstruction(&COMMAND_R1, NULL, POP, HandlePop);

		// 计算R0 OP R1, 结果存入R0中
		PutInstruction(&COMMAND_R1, &COMMAND_R0, op, HandleLogic);
	}
}

// 赋值,=
static LVoid EvalAssignment() {
	EvalLogic(); // =>R0
	if (gToken.mTokenValue == ASSIGN) { // '='
		// R0存入栈
		PutInstruction(&COMMAND_R0, NULL, PUSH, HandlePush);
		NextToken();
		EvalLogic(); // =>R0
					 // 从栈中吐出数据到R1
		PutInstruction(&COMMAND_R1, NULL, POP, HandlePop);
		PutInstruction(&COMMAND_R1, &COMMAND_R0, ASSIGN_VAR, HandleAssignVar);
	}
}

// init global data such as gGlobalsTable, gLocalsStack, gFunTable
static LVoid InitGlobalData() {
	if (!gGlobalsTable) {
		// 初始化MJS内存分配池
		CreateMiniMemory();
		/* 一个页面只允许最多NUM_GLOBAL_VARS个函数 */
		gGlobalsTable = NEW_ARRAY(BoyiaValue, NUM_GLOBAL_VARS);
		//LMemset(gGlobalsTable, 0, NUM_GLOBAL_VARS * sizeof(BoyiaValue));

		gLocalsStack = NEW_ARRAY(BoyiaValue, NUM_LOCAL_VARS);
		//LMemset(gLocalsStack, 0, NUM_LOCAL_VARS * sizeof(BoyiaValue));

		gFunTable = NEW_ARRAY(BoyiaFunction, NUM_FUNC);
		//LMemset(gFunTable, 0, NUM_FUNC * sizeof(BoyiaFunction));

		gOpStack = NEW_ARRAY(BoyiaValue, NUM_RESULT);

		gCallStack = NEW_ARRAY(ExecScene, FUNC_CALLS);
		gLoopStack = NEW_ARRAY(LInt, LOOP_NEST);

		gEState.mGValSize = 0;
		gEState.mFunSize = 0;
	}
}

static LVoid InitNativeFun(LVoid* map) {
	if (!gNativeFunTable) {
		NativeFunMap* nativeMap = (NativeFunMap*)map;
		gNativeFunTable = NEW_ARRAY(NativeFunction, (nativeMap->mSize + 2));
		gNativeFunTable[0] = { GenIdentByStr("new", 3), CreateObject };
		gNativeFunTable[nativeMap->mSize + 1] = { 0,  NULL };

		if (nativeMap) {
			LInt idx = 0;
			for (; idx < nativeMap->mSize; ++idx) {
				gNativeFunTable[idx + 1] = nativeMap->mFun[idx];
			}
		}
	}
}

LVoid SetNativeResult(LVoid* result) {
	BoyiaValue* value = (BoyiaValue*)result;
	ValueCopy(&gVM.mReg0, value);
}

LVoid* CopyObject(LUint hashKey, LInt size) {
	return CopyFunction(FindGlobal(hashKey), size);
}

LVoid* GetNativeResult() {
	return &gVM.mReg0;
}

LVoid GetLocalStack(LInt* stack, LInt* size) {
	*stack = (LIntPtr) gLocalsStack;
	*size = gEState.mLValSize;
}

LVoid GetGlobalTable(LInt* table, LInt* size) {
	*table = (LIntPtr) gGlobalsTable;
	*size = gEState.mGValSize;
}

/*  output function */
LVoid CompileCode(LInt8* code, LVoid* map) {
    InitGlobalData();
	InitNativeFun(map);
    gEState.mProg        = code;
    gEState.mLineNum     = 0;
	gEState.mTmpLValSize = 0;
	gEState.mResultNum   = 0;
    gEState.mLoopSize    = 0;
    gEState.mClass       = NULL;
	ParseStatement(); // 该函数记录全局变量以及函数接口
	ResetScene();
}

LVoid* GetLocalValue(LInt idx) {
    LInt start = gCallStack[gEState.mFunctos - 1].mLValSize;
	return &gLocalsStack[start + idx];
}

LInt GetLocalSize() {
    return gEState.mLValSize - gCallStack[gEState.mFunctos - 1].mLValSize;
}

LVoid CallFunction(LInt8* fun, LVoid* ret) {
    EngineLog("callFunction=>%d \n", 1);
    gEState.mProg = fun;
	CommandTable* cmds = CreateExecutor();
	gEState.mContext = cmds;
	EvalExpression(); // 解析例如func(a,b,c);
	ExecuteCode(cmds);

	if (ret) {
		BoyiaValue* value = (BoyiaValue*)ret;
		ValueCopy(value, &gVM.mReg0);
	}
}

LVoid SaveLocalSize() {
	HandleTempLocalSize(NULL);
}

LVoid NativeCall(BoyiaValue* obj) {
	gCallStack[gEState.mFunctos].mClass = gEState.mClass;
	gCallStack[gEState.mFunctos].mLValSize = gEState.mTmpLValSize;
	gCallStack[gEState.mFunctos].mPC = gEState.mPC;
	gCallStack[gEState.mFunctos].mContext = gEState.mContext;
	gCallStack[gEState.mFunctos++].mLoopSize = gEState.mLoopSize;
	gEState.mClass = obj;

	HandlePushParams(NULL);
	HandleCallFunction(NULL);

	ExecInstruction();
}
