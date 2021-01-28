#ifndef BoyiaValue_h
#define BoyiaValue_h

#include "PlatformLib.h"

enum KeyWord {
    BY_ARG = 0,
    BY_CHAR,
    BY_INT,
    BY_STRING,
    BY_FUNC,
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
    BY_END, // 18
};

typedef struct {
    LInt8* mPtr;
    LInt mLen;
} BoyiaStr;

typedef struct {
    LIntPtr mPtr;
    LIntPtr mSuper;
} BoyiaClass;

typedef struct {
    LUintPtr mNameKey; // HASH KEY 用来加快查找速度
    LUint8 mValueType;
    union RealValue {
        LIntPtr mIntVal;
        BoyiaClass mObj; // 类应该存储于方法区
        BoyiaStr mStrVal;
    } mValue;
} BoyiaValue;

typedef struct {
    LIntPtr mFuncBody;
    BoyiaValue* mParams;
    LInt mParamSize;
    LInt mParamCount;
} BoyiaFunction;

typedef struct {
    BoyiaValue* mClass;
    LIntPtr mIndex;
    LInt mType;
} InlineCacheItem;

typedef struct {
    InlineCacheItem* mItems;
    LInt mSize;
} InlineCache;

#define NEW(type, vm) (type*)BoyiaAlloc(sizeof(type), vm)
#define NEW_ARRAY(type, n, vm) (type*)BoyiaAlloc(n * sizeof(type), vm)
#define VM_DELETE(ptr, vm) BoyiaDelete(ptr, vm)
#define D_STR(str, len)  \
    {                    \
        (LInt8*)str, len \
    }

// function count最大不能超过65535
// mParamCount的剩余高16位将用来做对象标记，如GC标记清除
#define GET_FUNCTION_COUNT(function) (function->mParamCount & 0x0000FFFF)

LVoid* BoyiaAlloc(LInt size, LVoid* vm);

LVoid* BoyiaNew(LInt size, LVoid* vm);
LVoid BoyiaDelete(LVoid* data, LVoid* vm);
LVoid BoyiaPreDelete(LVoid* ptr, LVoid* vm);

LVoid* GetGabargeCollect(LVoid* vm);

LVoid InitStr(BoyiaStr* str, LInt8* ptr);
LBool MStrcmp(BoyiaStr* src, BoyiaStr* dest);
LVoid MStrcpy(BoyiaStr* dest, BoyiaStr* src);
LBool MStrchr(const LInt8* s, LInt8 ch);
LVoid StringAdd(BoyiaValue* left, BoyiaValue* right, LVoid* vm);
LUintPtr GenIdentifier(BoyiaStr* str, LVoid* vm);
LUintPtr GenIdentByStr(const LInt8* str, LInt len, LVoid* vm);

InlineCache* CreateInlineCache(LVoid* vm);
LVoid AddPropInlineCache(InlineCache* cache, BoyiaValue* klass, LInt index);
LVoid AddFunInlineCache(InlineCache* cache, BoyiaValue* klass, BoyiaValue* fun);
BoyiaValue* GetInlineCache(InlineCache* cache, BoyiaValue* klass);

LVoid CacheInstuctions(LVoid* instructionBuffer, LInt size);
LVoid CacheStringTable(BoyiaStr* stringTable, LInt size, LVoid* vm);
LVoid CacheInstuctionEntry(LVoid* vmEntryBuffer, LInt size);
LVoid CacheSymbolTable(LVoid* vm);

LVoid LoadVMCode(LVoid* vm);

LInt FindNativeFunc(LUintPtr key, LVoid* vm);
LInt CallNativeFunction(LInt idx, LVoid* vm);

LVoid MarkNativeObject(LIntPtr address, LInt gcFlag);
LInt NativeObjectFlag(LVoid* address);
#endif
