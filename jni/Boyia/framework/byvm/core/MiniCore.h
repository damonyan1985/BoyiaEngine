/*
============================================================================
 Name        : MiniCore.h
 Author      : yanbo
 Version     : MiniCore v1.0
 Copyright   : All Copyright Reserved
 Date        : 2012-2-5
 Description : Support Original
============================================================================
*/

#ifndef MiniCore_h
#define MiniCore_h

#include "MiniValue.h"

typedef struct {
	LInt             mFuncBody;
	MiniValue*       mParams;
	LInt             mParamSize;
	LInt             mParamCount;
} MiniFunction;

typedef struct {
	LUint mNameKey;
	LInt (*mAddr)();
} NativeFunction;

typedef struct {
	NativeFunction* mFun;
	LInt mSize;
} NativeFunMap;

LVoid CompileCode(char* code, LVoid* fun);
LVoid CallFunction(char* funcCall, LVoid* ret);
LInt GetLocalSize();
LVoid* GetLocalValue(LInt idx);
LVoid SetNativeResult(LVoid* result);
LVoid GetLocalStack(LInt* stack, LInt* size);
LVoid* GetNativeResult();
LVoid* CopyObject(LUint hashKey, LInt size);
//LUint HashCode(MiniStr* str);
LVoid ValueCopy(MiniValue* dest, MiniValue* src);
//LBool MStrcmp(MiniStr* src, MiniStr* dest);
LVoid GetGlobalTable(LInt* table, LInt* size);
LVoid NativeCall(MiniValue* obj);
LVoid LocalPush(MiniValue *value);
LVoid SaveLocalSize();
void* miniNew(int size);
void miniDelete(void* data);

#endif
