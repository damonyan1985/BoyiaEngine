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
#include <stdio.h>
#include <stdlib.h>
#if ENABLE(BOYIA_WINDOWS)
#include <varargs.h>
#elif ENABLE(BOYIA_ANDROID)
#include <android/log.h>
#endif

#define MAX_INT_LEN 20
#define MEMORY_SIZE (LInt)1024 * 1024 * 6
#define MAX_INLINE_CACHE 5

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

static LVoid SystemGC(LVoid* vm)
{
    if (GetRuntime(vm)->needCollect()) {
        GCollectGarbage(vm);
    }
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

LVoid BoyiaPreDelete(LVoid* ptr, LVoid* vm)
{
    GetRuntime(vm)->prepareDelete(ptr);
}

LVoid BoyiaDelete(LVoid* data, LVoid* vm)
{
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
    delete static_cast<boyia::BoyiaBase*>(data);
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

    //right->mValue.mStrVal.mPtr = str;
    //right->mValue.mStrVal.mLen = len;
    //right->mValueType = BY_STRING;

    BoyiaFunction* objBody = CreateStringObject(str, len, vm);
    // right其实就是R0
    right->mValueType = BY_CLASS;
    right->mNameKey = kBoyiaString;
    right->mValue.mObj.mPtr = (LIntPtr)objBody;
    right->mValue.mObj.mSuper = kBoyiaNull;

    GCAppendRef(str, BY_CLASS, vm);
    KLOG("StringAdd End");
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
    InlineCache* cache = NEW(InlineCache, vm);
    cache->mSize = 0;
    cache->mItems = NEW_ARRAY(InlineCacheItem, MAX_INLINE_CACHE, vm);
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
    GetRuntime(vm)->idCreator()->getIdentName(key, str);
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
        strTable[i].mPtr = NEW_ARRAY(LInt8, stringTable->elementAt(i).GetLength(), vm);
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
// String class builtin
LUintPtr GetBoyiaClassId(BoyiaValue* obj)
{
    BoyiaFunction* objBody = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    BoyiaValue* clzz = (BoyiaValue*)objBody->mFuncBody;
    return clzz->mNameKey;
}

LVoid BuiltinStringClass(LVoid* vm)
{
    BoyiaValue* classRef = (BoyiaValue*)CreateGlobalClass(kBoyiaString, vm);

    BoyiaFunction* classBody = (BoyiaFunction*)classRef->mValue.mObj.mPtr;

    // first prop is raw string
    classBody->mParams[classBody->mParamSize].mValueType = BY_STRING;
    classBody->mParams[classBody->mParamSize].mNameKey = GenIdentByStr("buffer", 6, vm);
    classBody->mParams[classBody->mParamSize].mValue.mStrVal.mPtr = kBoyiaNull;
    classBody->mParams[classBody->mParamSize++].mValue.mStrVal.mLen = 0;
}

BoyiaStr* GetStringBuffer(BoyiaValue* ref)
{
    BoyiaFunction* strObj = (BoyiaFunction*)ref->mValue.mObj.mPtr;
    return &strObj->mParams[0].mValue.mStrVal;
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

LVoid CreateNativeString(BoyiaValue* value, LInt8* buffer, LInt len, LVoid* vm)
{
    BoyiaFunction* objBody = CreateStringObject(buffer, len, vm);
    value->mValueType = BY_CLASS;
    value->mValue.mObj.mPtr = (LIntPtr)objBody;
    value->mValue.mObj.mSuper = kBoyiaNull;

    // Mark native string
    objBody->mParamCount = objBody->mParamCount | (kNativeStringBuffer << 18);
}

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
    BoyiaFunction* objBody = (BoyiaFunction*)CopyObject(kBoyiaString, 32, vm);
    objBody->mParams[0].mValue.mStrVal.mPtr = buffer;
    objBody->mParams[0].mValue.mStrVal.mLen = len;

    GCAppendRef(objBody, BY_CLASS, vm);
    return objBody;
}
