#ifndef BoyiaValue_h
#define BoyiaValue_h

#include "PlatformLib.h"

// BY_PROP_FUNC，属性方法类型，一般用于对象回调
enum KeyWord {
    BY_ARG = 0,
    BY_CHAR,
    BY_INT,
    BY_STRING,
    BY_FUNC,
    BY_PROP_FUNC,
    BY_NAV_FUNC,
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

enum OpHandleResult {
    // Handle执行失败
    kOpResultFail = 0,
    // Handle执行成功
    kOpResultSuccess,
    // Handle跳转到函数进行执行
    kOpResultJumpFun
};

enum BuiltinId {
    kBoyiaThis = 1,
    kBoyiaSuper,
    kBoyiaString,
    kBoyiaArray,
    kBoyiaMap,
};

// 字符串类型含义
// kBoyiaStringBuffer表示Boyia内存池创建的buffer
// kNativeStringBuffer表示String类创建的buffer
// kConstStringBuffer表示是从Boyia常量表中获取的
enum StringBufferType {
    kBoyiaStringBuffer = 0x0,
    kNativeStringBuffer = 0x1,
    kConstStringBuffer = 0x2
};

typedef struct {
    LInt8* mPtr;
    LInt mLen;
} BoyiaStr;

// 如果是对象，mPtr指向当前对象的地址，mSuper指向其父类，没有父类mSuper为NULL
// 如果是属性方法，mPtr指向当前方法的地址，mSuper指向对象地址
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
#define NEW_ARRAY(type, n, vm) (type*)BoyiaAlloc((n) * sizeof(type), vm)
#define VM_DELETE(ptr, vm) BoyiaDelete(ptr, vm)
#define D_STR(str, len)  \
    {                    \
        (LInt8*)str, len \
    }

// function count最大不能超过65535
// mParamCount的剩余高16位将用来做对象标记：
// 17，18表示GC标记清除；19，20位表示不同字符串类型的标记
// 其中高16位前两位表示GC标记清除状态
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

// Builtins class
LUintPtr GetBoyiaClassId(BoyiaValue* obj);
LVoid BuiltinStringClass(LVoid* vm);
BoyiaStr* GetStringBufferFromBody(BoyiaFunction* body);
BoyiaStr* GetStringBuffer(BoyiaValue* ref);
LIntPtr GetStringHash(BoyiaValue* ref);
BoyiaFunction* CreateStringObject(LInt8* buffer, LInt len, LVoid* vm);

LVoid CreateConstString(BoyiaValue* value, LInt8* buffer, LInt len, LVoid* vm);
LVoid CreateStringValue(BoyiaValue* value, LInt8* buffer, LInt len, LVoid* vm);
LVoid CreateNativeString(BoyiaValue* value, LInt8* buffer, LInt len, LVoid* vm);
LVoid SetStringResult(LInt8* buffer, LInt len, LVoid* vm);

LVoid BuiltinMapClass(LVoid* vm);
BoyiaFunction* CreatMapObject(LVoid* vm);

// 创建数组对象
BoyiaFunction* CreatArrayObject(LVoid* vm);

LVoid SystemGC(LVoid* vm);
#endif
