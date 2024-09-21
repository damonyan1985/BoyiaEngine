//
// Created by yanbo on 2017/9/26.
//

#include "AutoLock.h"
#include "BoyiaBase.h"
#include "BoyiaMemory.h"
#include "BoyiaPtr.h"
#include "BoyiaRuntime.h"
#include "FileUtil.h"
#include "IDCreator.h"
#include "PlatformBridge.h"
#include "PlatformLib.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "SystemUtil.h"
#include "BoyiaCore.h"
#include "BoyiaError.h"
#include <stdio.h>
#include <stdlib.h>
#if ENABLE(BOYIA_WINDOWS)
#include <varargs.h>
#elif ENABLE(BOYIA_ANDROID)
#include <android/log.h>
#endif

#define MAX_INT_LEN 20
#define MEMORY_SIZE (LInt)1024 * 1024 * 6

#define GEN_ID(key, vm) GenIdentByStr(key, StringUtils::StringSize(key), vm)

enum InlineCacheType {
    CACHE_PROP = 1,
    CACHE_METHOD
};

extern LVoid GCAppendRef(LVoid* address, LUint8 type, LVoid* vm);
extern LVoid GCollectGarbage(LVoid* vm);
extern LVoid BoyiaLog(const char* format, ...)
{
    va_list args;
    va_start(args, format);
#if ENABLE(BOYIA_ANDROID)
    __android_log_vprint(ANDROID_LOG_INFO, "BoyiaVM", format, args);
#else
    printf(format, args);
#endif
    va_end(args);
}

extern LInt Str2Int(LInt8* p, LInt len, LInt radix)
{
    //LUint8 *p = (LUint8*) ptr;
    LInt total = 0;
    LInt sign = 1;
    LInt pos = 0;

    if (*p == '-') {
        sign = -1;
        ++pos;
    } else if (*p == '+') {
        ++pos;
    }

    while (pos < len) {
        LInt ch = 0;
        if (LIsDigit(*(p + pos))) {
            ch = *(p + pos) - '0';
        } else if (LIsBigChar(*(p + pos))) {
            ch = *(p + pos) - 'A' + 10;
        } else if (LIsMinChar(*(p + pos))) {
            ch = *(p + pos) - 'a' + 10;
        }

        total = total * radix + ch;
        ++pos;
    }

    return total * sign;
}

static boyia::BoyiaRuntime* GetRuntime(LVoid* vm)
{
    return static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm));
}

LVoid* GetGabargeCollect(LVoid* vm)
{
    return GetRuntime(vm)->garbageCollect();
}

LVoid MarkNativeObject(LIntPtr address, LInt gcFlag)
{
    reinterpret_cast<boyia::BoyiaBase*>(address)->setGcFlag(gcFlag);
}

LInt NativeObjectFlag(LVoid* address)
{
    return static_cast<boyia::BoyiaBase*>(address)->gcFlag();
}

LVoid SystemGC(LVoid* vm)
{
    if (GetRuntime(vm)->needCollect()) {
        GetRuntime(vm)->setGcRuning(LTrue);
        GetRuntime(vm)->collectGarbage();
    }
    //GCollectGarbage(vm);
}

LVoid* BoyiaAlloc(LInt size, LVoid* vm)
{
    SystemGC(vm);
    return BoyiaNew(size, vm);
}

// Alloc Data
LVoid* BoyiaNew(LInt size, LVoid* vm)
{
    PrintPoolSize(GetRuntime(vm)->memoryPool());
    return NewData(size, GetRuntime(vm)->memoryPool());
}

LVoid* CreateRuntimeToMemory(LVoid* vm)
{
    return GetRuntime(vm)->createMemoryBackup();
}

LVoid* MigrateRuntimeMemory(LVoid* addr, LVoid* pool, LVoid* vm)
{
    return MigrateMemory(addr, GetRuntime(vm)->memoryPool(), pool);
}

LVoid UpdateRuntimeMemory(LVoid* pool, LVoid* vm)
{
    GetRuntime(vm)->changeMemoryPool(pool);
}

// 删除挂载在对象上的所有事件
LVoid BoyiaPreDelete(LVoid* ptr, LVoid* vm)
{
    GetRuntime(vm)->prepareDelete(ptr);
}

LVoid BoyiaDelete(LVoid* data, LVoid* vm)
{
    // 先清除挂载在对象上的所有事件
    BoyiaPreDelete(data, vm);
    // 然后再清除对象
    return DeleteData(data, GetRuntime(vm)->memoryPool());
}

LVoid MStrcpy(BoyiaStr* dest, BoyiaStr* src)
{
    dest->mPtr = src->mPtr;
    dest->mLen = src->mLen;
}

LVoid InitStr(BoyiaStr* str, LInt8* ptr)
{
    str->mLen = 0;
    str->mPtr = ptr;
}

LBool MStrchr(const LInt8* s, LInt8 ch)
{
    while (*s && *s != ch)
        ++s;
    return *s && *s == ch;
}

LBool MStrcmp(BoyiaStr* src, BoyiaStr* dest)
{
    if (src->mLen != dest->mLen) {
        return LFalse;
    }

    // 地址一样直接返回true
    if (src->mPtr == dest->mPtr) {
        return LTrue;
    }

    LInt len = src->mLen;
    while (len--) {
        if (*(src->mPtr + len) != *(dest->mPtr + len)) {
            return LFalse;
        }
    }

    return LTrue;
}

extern LVoid NativeDelete(LVoid* data)
{
    static_cast<boyia::BoyiaBase*>(data)->release();
}

// "Hello" + "World"
static LVoid FetchString(BoyiaStr* str, BoyiaValue* value, LVoid* vm)
{
    if (value->mValueType == BY_INT) {
        str->mPtr = NEW_ARRAY(LInt8, MAX_INT_LEN, vm);
        LMemset(str->mPtr, 0, MAX_INT_LEN);
        LInt2StrWithLength(value->mValue.mIntVal, (LUint8*)str->mPtr, 10, &str->mLen);
    } else {
        // String Object
        BoyiaStr* buffer = GetStringBuffer(value);
        str->mPtr = buffer->mPtr;
        str->mLen = buffer->mLen;
    }
}

extern LVoid StringAdd(BoyiaValue* left, BoyiaValue* right, LVoid* vm)
{
    KLOG("StringAdd Begin");
    BoyiaStr leftStr, rightStr;
    LInt8 tmpArray[MAX_INT_LEN];
    leftStr.mPtr = tmpArray;
    rightStr.mPtr = tmpArray;
    FetchString(&leftStr, left, vm);
    FetchString(&rightStr, right, vm);

    LInt len = leftStr.mLen + rightStr.mLen;
    LInt8* str = NEW_ARRAY(LInt8, len, vm);

    LMemcpy(str, leftStr.mPtr, leftStr.mLen);
    LMemcpy(str + leftStr.mLen, rightStr.mPtr, rightStr.mLen);

    BoyiaFunction* objBody = CreateStringObject(str, len, vm);
    // right其实就是R0
    right->mValueType = BY_CLASS;
    right->mNameKey = kBoyiaString;
    right->mValue.mObj.mPtr = (LIntPtr)objBody;
    right->mValue.mObj.mSuper = kBoyiaNull;

    //GCAppendRef(str, BY_CLASS, vm);
    KLOG("StringAdd End");
}

LUintPtr GenIdentByStr(const LInt8* str, LVoid* vm)
{
    return GetRuntime(vm)->idCreator()->genIdentByStr(str, LStrlen(_CS(str)));
}

LUintPtr GenIdentByStr(const LInt8* str, LInt len, LVoid* vm)
{
    return GetRuntime(vm)->idCreator()->genIdentByStr(str, len);
}

LUintPtr GenIdentifier(BoyiaStr* str, LVoid* vm)
{
    return GetRuntime(vm)->idCreator()->genIdentifier(str);
}

InlineCache* CreateInlineCache(LVoid* vm)
{
    InlineCache* cache = FAST_NEW(InlineCache);
    cache->mSize = 0;
    return cache;
}

LVoid AddPropInlineCache(InlineCache* cache, BoyiaValue* klass, LInt index)
{
    if (!cache) {
        return;
    }

    if (cache->mSize < MAX_INLINE_CACHE) {
        cache->mItems[cache->mSize].mClass = klass;
        cache->mItems[cache->mSize].mIndex = index;
        cache->mItems[cache->mSize++].mType = CACHE_PROP;
    }
}

LVoid AddFunInlineCache(InlineCache* cache, BoyiaValue* klass, BoyiaValue* fun)
{
    if (!cache) {
        return;
    }

    if (cache->mSize < MAX_INLINE_CACHE) {
        cache->mItems[cache->mSize].mClass = klass;
        cache->mItems[cache->mSize].mIndex = (LIntPtr)fun;
        cache->mItems[cache->mSize++].mType = CACHE_METHOD;
    }
}

BoyiaValue* GetInlineCache(InlineCache* cache, BoyiaValue* obj)
{
    yanbo::TimeAnalysis analysis("GetInlineCache");
    if (!cache) {
        return kBoyiaNull;
    }

    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    BoyiaValue* klass = (BoyiaValue*)fun->mFuncBody;

    LInt index = 0;
    while (index < cache->mSize) {
        if (cache->mItems[index].mClass == klass) {
            switch (cache->mItems[index].mType) {
            case CACHE_PROP:
                return fun->mParams + cache->mItems[index].mIndex;
            case CACHE_METHOD:
                return (BoyiaValue*)cache->mItems[index].mIndex;
            default:
                break;
            }
        }

        ++index;
    }

    return kBoyiaNull;
}

extern LVoid GetIdentName(LUintPtr key, BoyiaStr* str, LVoid* vm)
{
    GetRuntime(vm)->idCreator()->getIdentName((LUint)key, str);
}

LVoid CacheInstuctions(LVoid* instructionBuffer, LInt size)
{
    FileUtil::writeFile(
        _CS(yanbo::PlatformBridge::getInstructionCachePath()),
        String(_CS(instructionBuffer), LFalse, size));
}

const LUint8* kStringTableSplitFlag = _CS("@boyia@stringtable@");

LVoid CacheStringTable(BoyiaStr* stringTable, LInt size, LVoid* vm)
{
    LInt length = 0;
    LInt flagLen = LStrlen(kStringTableSplitFlag);
    for (LInt i = 0; i < size; i++) {
        length += stringTable[i].mLen;
    }

    // String should add flag size
    length += (size - 1) * flagLen;

    LUint8* buffer = NEW_BUFFER(LUint8, length);
    // 添加StringTable数据
    LInt index = 0;
    for (LInt i = 0; i < size; i++) {
        LMemcpy(buffer + index, stringTable[i].mPtr, stringTable[i].mLen);
        index += stringTable[i].mLen;
        if (i < size - 1) {
            LMemcpy(buffer + index, kStringTableSplitFlag, flagLen);
            index += flagLen;
        }
    }
    FileUtil::writeFile(
        _CS(yanbo::PlatformBridge::getStringTableCachePath()),
        String(buffer, LFalse, length));
}

LVoid CacheInstuctionEntry(LVoid* vmEntryBuffer, LInt size)
{
    FileUtil::writeFile(
        _CS(yanbo::PlatformBridge::getInstructionEntryPath()),
        String(_CS(vmEntryBuffer), LFalse, size));
}

LVoid CacheSymbolTable(LVoid* vm)
{
    util::IDCreator* idCreator = GetRuntime(vm)->idCreator();

    OwnerPtr<String> ownerString = idCreator->idsToString();
    FileUtil::writeFile(
        _CS(yanbo::PlatformBridge::getSymbolTablePath()),
        *ownerString.get());
}

static LVoid LoadSymbolTable(LVoid* vm)
{
    // Load SymbolTable
    String content;
    FileUtil::readFile(_CS(yanbo::PlatformBridge::getSymbolTablePath()), content);
    OwnerPtr<KVector<String>> symbolTable = StringUtils::split(content, _CS("\n"));

    for (LInt i = 0; i < symbolTable->size(); i++) {
        OwnerPtr<KVector<String>> ids = StringUtils::split(symbolTable->elementAt(i), _CS(":"));
        LUint id = StringUtils::stringToInt(ids->elementAt(1));
        GetRuntime(vm)->idCreator()->appendIdentify(ids->elementAt(0), id);
    }
}

LVoid LoadVMCode(LVoid* vm)
{
    // Load StringTable
    String content;
    FileUtil::readFile(_CS(yanbo::PlatformBridge::getStringTableCachePath()), content);
    OwnerPtr<KVector<String>> stringTable = StringUtils::split(content, kStringTableSplitFlag);

    BoyiaStr* strTable = new BoyiaStr[stringTable->size()];
    for (LInt i = 0; i < stringTable->size(); i++) {
        strTable[i].mPtr = FAST_NEW_ARRAY(LInt8, stringTable->elementAt(i).GetLength());
        strTable[i].mLen = stringTable->elementAt(i).GetLength();
        LMemcpy(strTable[i].mPtr, stringTable->elementAt(i).GetBuffer(), strTable[i].mLen);
    }

    LoadStringTable(strTable, stringTable->size(), vm);
    delete[] strTable;

    LoadSymbolTable(vm);

    // Load Instructions
    FileUtil::readFile(_CS(yanbo::PlatformBridge::getInstructionCachePath()), content);
    LoadInstructions(content.GetBuffer(), content.GetLength(), vm);

    // Load EntryTable
    FileUtil::readFile(_CS(yanbo::PlatformBridge::getInstructionEntryPath()), content);
    LoadEntryTable(content.GetBuffer(), content.GetLength(), vm);
}

LInt FindNativeFunc(LUintPtr key, LVoid* vm)
{
    return GetRuntime(vm)->findNativeFunc(key);
}

LInt CallNativeFunction(LInt idx, LVoid* vm)
{
    return GetRuntime(vm)->callNativeFunction(idx);
}

// Boyia builtins
// 创建Builtin类的方法
LVoid GenBuiltinClassFunction(LUintPtr key, NativePtr func, BoyiaFunction* classBody, LVoid* vm)
{
    BoyiaFunction* function = NEW(BoyiaFunction, vm);
    function->mParams = kBoyiaNull;
    function->mParamSize = 0;
    // 实际调用的函数
    function->mFuncBody = (LIntPtr)func;
    
    BoyiaValue* putFuncVal = &classBody->mParams[classBody->mParamSize++];
    putFuncVal->mValueType = BY_NAV_FUNC; // 内置类的函数类型
    putFuncVal->mNameKey = key; // GenIdentByStr("put", vm);
    putFuncVal->mValue.mObj.mPtr = (LIntPtr)function;
}

LVoid GenBuiltinClassPropFunction(LUintPtr key, NativePtr func, BoyiaFunction* classBody, LVoid* vm)
{
    BoyiaFunction* function = NEW(BoyiaFunction, vm);
    function->mParams = kBoyiaNull;
    function->mParamSize = 0;
    // 实际调用的函数
    function->mFuncBody = (LIntPtr)func;

    BoyiaValue* putFuncVal = &classBody->mParams[classBody->mParamSize++];
    putFuncVal->mValueType = BY_NAV_PROP; // 内置类的函数类型
    putFuncVal->mNameKey = key; // GenIdentByStr("put", vm);
    putFuncVal->mValue.mObj.mPtr = (LIntPtr)function;
    putFuncVal->mValue.mObj.mSuper = (LIntPtr)classBody;
}

// 获取唯一标识class的class id
LUintPtr GetBoyiaClassId(BoyiaValue* obj)
{
    BoyiaFunction* objBody = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    BoyiaValue* clzz = (BoyiaValue*)objBody->mFuncBody;
    return clzz->mNameKey;
}

// String class builtin
LInt BoyiaStringLength(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 索引0为函数指针
    // 最后一个索引是调用时添加的对象
    // 0与size-1之间是函数的参数
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);
    BoyiaStr* str = GetStringBuffer(obj);
    
    BoyiaValue value;
    value.mValueType = BY_INT;
    value.mValue.mIntVal = str->mLen;
    SetNativeResult(&value, vm);
    return kOpResultSuccess;
}

LInt BoyiaStringEqual(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 索引0为函数指针
    // 最后一个索引是调用时添加的对象
    // 0与size-1之间是函数的参数
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);
    BoyiaStr* str = GetStringBuffer(obj);
    LIntPtr strHash = GetStringHash(obj);
    
    BoyiaValue* cmpStrVal = (BoyiaValue*)GetLocalValue(1, vm);
    BoyiaStr* cmpStr = GetStringBuffer(cmpStrVal);
    LIntPtr cmpStrHash = GetStringHash(cmpStrVal);
    
    BOYIA_LOG("BoyiaStringEqual hash1=%lld and hash2=%lld", strHash, cmpStrHash);
    
    BoyiaValue value;
    value.mValueType = BY_INT;
    value.mValue.mIntVal = strHash == cmpStrHash && MStrcmp(str, cmpStr);
    SetNativeResult(&value, vm);
    return kOpResultSuccess;
}

// 内置Boyia的String类
LVoid BuiltinStringClass(LVoid* vm)
{
    BoyiaValue* classRef = (BoyiaValue*)CreateGlobalClass(kBoyiaString, vm);
    // 没有父类
    classRef->mValue.mObj.mSuper = kBoyiaNull;

    BoyiaFunction* classBody = (BoyiaFunction*)classRef->mValue.mObj.mPtr;

    // first prop is raw string
    {
        classBody->mParams[classBody->mParamSize].mValueType = BY_STRING;
        // 第一个成员是buffer
        classBody->mParams[classBody->mParamSize].mNameKey = GEN_ID("buffer", vm); //GenIdentByStr("buffer", 6, vm);
        classBody->mParams[classBody->mParamSize].mValue.mStrVal.mPtr = kBoyiaNull;
        classBody->mParams[classBody->mParamSize++].mValue.mStrVal.mLen = 0;
    }
    
    // second prop is hash
    {
        classBody->mParams[classBody->mParamSize].mValueType = BY_INT;
        // 第一个成员是buffer
        classBody->mParams[classBody->mParamSize].mNameKey = GEN_ID("hash", vm); //GenIdentByStr("buffer", 6, vm);
        classBody->mParams[classBody->mParamSize++].mValue.mIntVal = 0; // 默认是0
    }
    
    // function length
    {
        // put function implementation begin
//        BoyiaFunction* function = NEW(BoyiaFunction, vm);
//        function->mParams = kBoyiaNull;
//        function->mParamSize = 0;
//        // 实际调用的函数
//        function->mFuncBody = (LIntPtr)BoyiaStringLength;
//
//        BoyiaValue* lengthFuncVal = &classBody->mParams[classBody->mParamSize++];
//        lengthFuncVal->mValueType = BY_NAV_FUNC; // 内置类的函数类型
//        lengthFuncVal->mNameKey = GEN_ID("length", vm);
//        lengthFuncVal->mValue.mObj.mPtr = (LIntPtr)function;
        
        GenBuiltinClassFunction(GEN_ID("length", vm), BoyiaStringLength, classBody, vm);
        // put function implementation end
    }
    
    // function equal
    {
        // put function implementation begin
//        BoyiaFunction* function = NEW(BoyiaFunction, vm);
//        function->mParams = kBoyiaNull;
//        function->mParamSize = 0;
//        // 实际调用的函数
//        function->mFuncBody = (LIntPtr)BoyiaStringEqual;
//
//        BoyiaValue* lengthFuncVal = &classBody->mParams[classBody->mParamSize++];
//        lengthFuncVal->mValueType = BY_NAV_FUNC; // 内置类的函数类型
//        lengthFuncVal->mNameKey = GEN_ID("equal", vm);
//        lengthFuncVal->mValue.mObj.mPtr = (LIntPtr)function;
        GenBuiltinClassFunction(GEN_ID("equal", vm), BoyiaStringEqual, classBody, vm);
        // put function implementation end
    }
}

LIntPtr GetStringHash(BoyiaValue* ref)
{
    BoyiaFunction* strObj = (BoyiaFunction*)ref->mValue.mObj.mPtr;
    return strObj->mParams[0].mValue.mIntVal;
}

BoyiaStr* GetStringBufferFromBody(BoyiaFunction* body)
{
    return &body->mParams[1].mValue.mStrVal;
}

BoyiaStr* GetStringBuffer(BoyiaValue* ref)
{
    BoyiaFunction* strObj = (BoyiaFunction*)ref->mValue.mObj.mPtr;
    return GetStringBufferFromBody(strObj);
}

LVoid CreateStringValue(BoyiaValue* value, LInt8* buffer, LInt len, LVoid* vm)
{
    BoyiaFunction* objBody = CreateStringObject(buffer, len, vm);
    value->mValueType = BY_CLASS;
    value->mValue.mObj.mPtr = (LIntPtr)objBody;
    value->mValue.mObj.mSuper = kBoyiaNull;
}

LVoid SetStringResult(LInt8* buffer, LInt len, LVoid* vm)
{
    BoyiaValue val;
    CreateStringValue(&val, buffer, len, vm);
    SetNativeResult(&val, vm);
}

// 分配在系统堆上的字符串
LVoid CreateNativeString(BoyiaValue* value, LInt8* buffer, LInt len, LVoid* vm)
{
    BoyiaFunction* objBody = CreateStringObject(buffer, len, vm);
    value->mValueType = BY_CLASS;
    value->mValue.mObj.mPtr = (LIntPtr)objBody;
    value->mValue.mObj.mSuper = kBoyiaNull;

    // Mark native string
    objBody->mParamCount = objBody->mParamCount | (kNativeStringBuffer << 18);
}

// 常量字符串
LVoid CreateConstString(BoyiaValue* value, LInt8* buffer, LInt len, LVoid* vm)
{
    BoyiaFunction* objBody = CreateStringObject(buffer, len, vm);
    
    value->mValueType = BY_CLASS;
    value->mValue.mObj.mPtr = (LIntPtr)objBody;
    value->mValue.mObj.mSuper = kBoyiaNull;

    // Mark const string
    objBody->mParamCount = objBody->mParamCount | (kConstStringBuffer << 18);
}

BoyiaFunction* CreateStringObject(LInt8* buffer, LInt len, LVoid* vm)
{
    // copy object后字段的顺序变成倒序，索引获取的时候要反着来
    BoyiaFunction* objBody = (BoyiaFunction*)CopyObject(kBoyiaString, 32, vm);
    objBody->mParams[1].mValue.mStrVal.mPtr = buffer;
    objBody->mParams[1].mValue.mStrVal.mLen = len;
    
    objBody->mParams[0].mValue.mIntVal = GenHashCode(buffer, len);
    return objBody;
}

// 内置Map Class builtin
LInt BoyiaMapPut(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 索引0为put函数指针
    // 索引1，2为两个传入的参数key,value，
    // 最后一个索引是调用时添加的对象
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);
    BoyiaValue* key = (BoyiaValue*)GetLocalValue(1, vm);
    BoyiaStr* keyStr = GetStringBuffer(key);
    
    BoyiaValue* value = (BoyiaValue*)GetLocalValue(2, vm);
    
    // Map对象地址
    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    // 设置key和value
    ValueCopy(fun->mParams + fun->mParamSize, value);
    fun->mParams[fun->mParamSize++].mNameKey = GenIdentifier(keyStr, vm);
    
    return kOpResultSuccess;
}

// map get方法
LInt BoyiaMapGet(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 索引0为get函数指针
    // 索引1为两个传入的参数key，
    // 最后一个索引是调用时添加的对象
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);
    BoyiaValue* key = (BoyiaValue*)GetLocalValue(1, vm);
    LUintPtr keyId = GenIdentifier(GetStringBuffer(key), vm);
    // Map对象地址
    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    for (LInt i = 0; i < fun->mParamSize; i++) {
        if (keyId == fun->mParams[i].mNameKey) {
            SetNativeResult(&fun->mParams[i], vm);
            return kOpResultSuccess;
        }
    }
    
    BoyiaValue val;
    val.mValue.mIntVal = kBoyiaNull;
    val.mValueType = BY_INT;
    SetNativeResult(&val, vm);
    return kOpResultSuccess;
}

// map remove方法
LInt BoyiaMapRemove(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 索引0为remove函数指针
    // 索引1为两个传入的参数key，
    // 最后一个索引是调用时添加的对象
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);
    BoyiaValue* key = (BoyiaValue*)GetLocalValue(1, vm);
    LUintPtr keyId = GenIdentifier(GetStringBuffer(key), vm);
    // Map对象地址
    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    
    LInt idx = -1;
    for (LInt i = 0; i < fun->mParamSize; i++) {
        if (keyId == fun->mParams[i].mNameKey) {
            idx = i;
            break;
        }
    }
    
    // 如果没有找到，则直接返回
    if (idx == -1) {
        return kOpResultSuccess;
    }
    
    for (LInt i = idx; i < fun->mParamSize - 1; ++i) {
        ValueCopy(fun->mParams + i, fun->mParams + i + 1);
    }
    
    --fun->mParamSize;
    return kOpResultSuccess;
}

// map clear方法
LInt BoyiaMapClear(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 索引0为clear函数指针
    // 最后一个索引是调用时添加的对象
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);
    // Map对象地址
    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    fun->mParamSize = 0;

    return kOpResultSuccess;
}

// TODO map的map方法
LInt BoyiaMapMap(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 索引0为map函数指针
    // 第一个参数为方法，属性方法或者是函数
    BoyiaValue* cb = (BoyiaValue*)GetLocalValue(1, vm);
    // 最后一个索引是调用时添加的对象
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);
    // Map对象地址
    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    if (cb->mValueType == BY_PROP_FUNC) {
        for (LInt i = 0; i < fun->mParamSize; i++) {
            // 构造回调对象引用
            BoyiaValue cbObj;
            cbObj.mValueType = BY_CLASS;
            cbObj.mValue.mObj.mPtr = cb->mValue.mObj.mSuper;

            BoyiaFunction* cbFun = (BoyiaFunction*)cb->mValue.mObj.mPtr;
            BoyiaValue args[2];
            ValueCopy(&args[0], cb);
            ValueCopy(&args[1], &fun->mParams[i]);
            args[1].mNameKey = cbFun->mParams[0].mNameKey;


            // 调用callback函数
            NativeCallImpl(args, 2, &cbObj, vm);
        }
    }
    
    //fun->mParamSize = 0;

    return kOpResultSuccess;
}

LVoid BuiltinMapClass(LVoid* vm)
{
    BoyiaValue* classRef = (BoyiaValue*)CreateGlobalClass(kBoyiaMap, vm);
    // 没有父类
    classRef->mValue.mObj.mSuper = kBoyiaNull;
    
    BoyiaFunction* classBody = (BoyiaFunction*)classRef->mValue.mObj.mPtr;

    // map api
    {
        // put function implementation begin
        GenBuiltinClassFunction(GEN_ID("put", vm), BoyiaMapPut, classBody, vm);
        // put function implementation end
        
        // get function implementation begin
        GenBuiltinClassFunction(GEN_ID("get", vm), BoyiaMapGet, classBody, vm);
        // get function implementation end
        
        // remove function implementation begin
        GenBuiltinClassFunction(GEN_ID("remove", vm), BoyiaMapRemove, classBody, vm);
        // remove function implementation end
        
        // clear function implementation begin
        GenBuiltinClassFunction(GEN_ID("clear", vm), BoyiaMapClear, classBody, vm);
        // clear function implementation end
        
        // map function implementation begin
        GenBuiltinClassFunction(GEN_ID("map", vm), BoyiaMapMap, classBody, vm);
        // map function implementation end
    }
}

BoyiaFunction* CreatMapObject(LVoid* vm)
{
    return (BoyiaFunction*)CopyObject(kBoyiaMap, 32, vm);
}
// end map builtin

// begin microtask builtin
LInt BoyiaMicroTaskResolve(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 最后一个索引是调用时添加的对象
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);
    // 索引0为函数指针
    // 第一个参数是微任务执行完毕后，返回的结果
    BoyiaValue* result = (BoyiaValue*)GetLocalValue(1, vm);
    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    
    // 设置恢复微任务标记
    ResumeMicroTask((LVoid*)fun->mParams[1].mValue.mIntVal, result, vm);

    return kOpResultSuccess;
}

LInt BoyiaMicroTaskInit(LVoid* vm)
{
    LInt size = GetLocalSize(vm);
    // 索引0为init函数指针
    // 第一个参数是worker
    // 最后一个索引是调用时添加的对象
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(size - 1, vm);

    // Such as: fun worker(resolve) { dosomething(resolve); }
    BoyiaValue* worker = (BoyiaValue*)GetLocalValue(1, vm);
    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;

    LVoid* task = CreateMicroTask(vm);
    fun->mParams[1].mValueType = BY_INT;
    fun->mParams[1].mValue.mIntVal = (LIntPtr)task;

    // 执行worker
    // 保存当前栈
    //SaveLocalSize(vm);
    // worker压栈
    //LocalPush(worker, vm);
    // fun->mParams[0]是resolve函数
    // 参数压栈,将resolve属性函数作为参数压栈
    //LocalPush(&fun->mParams[0], vm);

    BoyiaValue args[2];
    ValueCopy(&args[0], worker);
    ValueCopy(&args[1], &fun->mParams[0]);

    PrintValueKey(&fun->mParams[0], vm);
    // 构造回调对象引用
    BoyiaValue cbObj;
    cbObj.mValueType = BY_CLASS;
    cbObj.mValue.mObj.mPtr = worker->mValue.mObj.mSuper;

    // 调用callback函数
    return NativeCallImpl(args, 2, &cbObj, vm);
}


LVoid BuiltinMicroTaskClass(LVoid* vm)
{
    BoyiaValue* classRef = (BoyiaValue*)CreateGlobalClass(kBoyiaMicroTask, vm);
    // 没有父类
    classRef->mValue.mObj.mSuper = kBoyiaNull;

    BoyiaFunction* classBody = (BoyiaFunction*)classRef->mValue.mObj.mPtr;

    // microtask prop
    {
        classBody->mParams[classBody->mParamSize].mValueType = BY_INT;
        // 第一个成员是task, 用于保存创建的c++ microtask
        classBody->mParams[classBody->mParamSize].mNameKey = GEN_ID("task", vm);
        classBody->mParams[classBody->mParamSize++].mValue.mIntVal = kBoyiaNull;
    }

    // microtask api
    {
        // init function implementation begin， init函数
        GenBuiltinClassFunction(GEN_ID("init", vm), BoyiaMicroTaskInit, classBody, vm);
        // init function implementation end
    }
    {
        // resume prop function implementation begin， 唤醒函数
        GenBuiltinClassPropFunction(GEN_ID("resolve", vm), BoyiaMicroTaskResolve, classBody, vm);
        // resume prop function implementation end
    }
}

BoyiaFunction* CreateMicroTaskObject(LVoid* vm)
{
    return (BoyiaFunction*)CopyObject(kBoyiaMicroTask, 32, vm);
}
// end microtask builtin


// 内置Array Class builtin
BoyiaFunction* CreateArrayObject(LVoid* vm)
{
    return (BoyiaFunction*)CopyObject(kBoyiaArray, 32, vm);
}
