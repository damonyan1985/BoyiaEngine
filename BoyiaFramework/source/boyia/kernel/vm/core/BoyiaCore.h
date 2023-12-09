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

// Boyia Native Interface定义
typedef LInt (*NativePtr)(LVoid* vm);

typedef struct {
    LUintPtr mNameKey;
    NativePtr mAddr;
} NativeFunction;

typedef struct {
    NativeFunction* mFun;
    LInt mSize;
} NativeFunMap;

// 编译Boyia代码
LVoid CompileCode(LInt8* code, LVoid* vm);
LVoid CallFunction(LInt8* funcCall, LVoid* ret, LVoid* vm);

// 获取当前Scope中的调用栈大小
LInt GetLocalSize(LVoid* vm);
// 根据索引获取当前Scope中临时变量
LVoid* GetLocalValue(LInt idx, LVoid* vm);
// 计算完设置结果给结果虚拟寄存器reg0
LVoid SetNativeResult(LVoid* result, LVoid* vm);
// 获取整个栈地址，及其大小
LVoid GetLocalStack(LIntPtr* stack, LInt* size, LVoid* vm);
// 获取结果虚拟寄存器地址
LVoid* GetNativeResult(LVoid* vm);
// 获取辅助虚拟寄存器地址
LVoid* GetNativeHelperResult(LVoid* vm);
// 使用类的key来创建对象
LVoid* CopyObject(LUintPtr hashKey, LInt size, LVoid* vm);
LVoid ValueCopy(BoyiaValue* dest, BoyiaValue* src);
// 获取全局表地址及其大小
LVoid GetGlobalTable(LIntPtr* table, LInt* size, LVoid* vm);
// 调用Boyia函数
LVoid NativeCall(BoyiaValue* obj, LVoid* vm);
// Push函数参数
LVoid LocalPush(BoyiaValue* value, LVoid* vm);
// 保存当前scope调用栈大小
LVoid SaveLocalSize(LVoid* vm);
// 初始化虚拟机
LVoid* InitVM(LVoid* creator);
LVoid DestroyVM(LVoid* vm);
LInt CreateObject(LVoid* vm);
LVoid CacheVMCode(LVoid* vm);
// 加载字符串表
LVoid LoadStringTable(BoyiaStr* stringTable, LInt size, LVoid* vm);
// 加载执行指令
LVoid LoadInstructions(LVoid* buffer, LInt size, LVoid* vm);
// 加载执行入口指令
LVoid LoadEntryTable(LVoid* buffer, LInt size, LVoid* vm);
LVoid* GetVMCreator(LVoid* vm);
// 执行指令
LVoid ExecuteGlobalCode(LVoid* vm);

// 创建全局类引用
LVoid* CreateGlobalClass(LUintPtr key, LVoid* vm);

// 添加一个微任务
LVoid* PushMicroTask(LVoid* vmPtr);

// 恢复微任务
LVoid ResumeMicroTask(LVoid* taskPtr, BoyiaValue* value);

// 消费微任务
LVoid ConsumeMicroTask(LVoid* vmPtr);

#endif
