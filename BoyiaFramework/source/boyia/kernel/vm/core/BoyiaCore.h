/*
============================================================================
 Name        : BoyiaCore.h
 Author      : yanbo
 Version     : BoyiaCore v0.1
 Copyright   : All Copyright Reserved
 Date        : 2012-2-5
 Description : Support Original
============================================================================
*/

#ifndef BoyiaCore_h
#define BoyiaCore_h

#include "BoyiaValue.h"

typedef LInt (*NativePtr)(LVoid* vm);

typedef struct {
    LUintPtr mNameKey;
    //LInt (*mAddr)(LVoid* vm);
    NativePtr mAddr;
} NativeFunction;

typedef struct {
    NativeFunction* mFun;
    LInt mSize;
} NativeFunMap;

LVoid CompileCode(LInt8* code, LVoid* vm);
LVoid CallFunction(LInt8* funcCall, LVoid* ret, LVoid* vm);
LInt GetLocalSize(LVoid* vm);
LVoid* GetLocalValue(LInt idx, LVoid* vm);
LVoid SetNativeResult(LVoid* result, LVoid* vm);
LVoid GetLocalStack(LInt* stack, LInt* size, LVoid* vm);
LVoid* GetNativeResult(LVoid* vm);
LVoid* CopyObject(LUintPtr hashKey, LInt size, LVoid* vm);
LVoid ValueCopy(BoyiaValue* dest, BoyiaValue* src);
LVoid GetGlobalTable(LInt* table, LInt* size, LVoid* vm);
LVoid NativeCall(BoyiaValue* obj, LVoid* vm);
LVoid LocalPush(BoyiaValue* value, LVoid* vm);
LVoid SaveLocalSize(LVoid* vm);
LVoid* InitVM(LVoid* creator);
LVoid DestroyVM(LVoid* vm);
LInt CreateObject(LVoid* vm);
LVoid CacheVMCode(LVoid* vm);
LVoid LoadStringTable(BoyiaStr* stringTable, LInt size, LVoid* vm);
LVoid LoadInstructions(LVoid* buffer, LInt size, LVoid* vm);
LVoid LoadEntryTable(LVoid* buffer, LInt size, LVoid* vm);
LVoid* GetVMCreator(LVoid* vm);
LVoid ExecuteGlobalCode(LVoid* vm);

// 创建全局类引用
LVoid* CreateGlobalClass(LUintPtr key, LVoid* vm);

#endif
