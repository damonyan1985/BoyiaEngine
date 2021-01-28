#include "AutoLock.h"
#include "BoyiaCore.h"
#include "BoyiaLib.h"
#include "BoyiaMemory.h"
#include "PlatformLib.h"
#include "SalLog.h"
#include <stdio.h>

typedef struct BoyiaRef {
    LVoid* mAddress;
    LUint8 mType;
    BoyiaRef* mNext;
} BoyiaRef;

typedef struct BoyiaGC {
    BoyiaRef* mBegin;
    BoyiaRef* mEnd;
    LInt mSize;
    LVoid* mBoyiaVM;
} BoyiaGC;

enum BoyiaGcColor {
    kBoyiaGcWhite = 0x0,
    kBoyiaGcGray = 0x00010000,
    kBoyiaGcBlack = 0x00020000
};

#define IS_OBJECT_INVALID(fun) ((fun->mParamCount & 0xFFFF0000) == kBoyiaGcWhite)

// 收集器
extern LVoid NativeDelete(LVoid* data);
static LBool CheckValue(BoyiaValue* val, BoyiaRef* ref);

extern LVoid* CreateGC(LVoid* vm)
{
    BoyiaGC* gc = FAST_NEW(BoyiaGC);
    gc->mBegin = kBoyiaNull;
    gc->mEnd = kBoyiaNull;
    gc->mSize = 0;
    gc->mBoyiaVM = vm;
    return gc;
}

static LBool GCCheckObject(BoyiaValue* value, BoyiaRef* ref)
{
    if (value->mValue.mIntVal == (LIntPtr)ref->mAddress) {
        return LTrue;
    }

    BoyiaFunction* fun = (BoyiaFunction*)value->mValue.mIntVal;
    LInt idx = 0;
    for (; idx < fun->mParamSize; ++idx) {
        if (CheckValue(&fun->mParams[idx], ref)) {
            return LTrue;
        }
    }

    return LFalse;
}

static LBool CheckValue(BoyiaValue* value, BoyiaRef* ref)
{
    if (value->mValueType == BY_NAVCLASS
        && value->mValue.mIntVal == (LIntPtr)ref->mAddress) {
        return LTrue;
    }

    if (value->mValueType == BY_STRING
        && (LIntPtr)value->mValue.mStrVal.mPtr == (LIntPtr)ref->mAddress) {
        return LTrue;
    }

    if (value->mValueType == BY_CLASS && GCCheckObject(value, ref)) {
        return LTrue;
    }

    return LFalse;
}

static LVoid DeleteRef(BoyiaRef* ref, BoyiaGC* gc)
{
    switch (ref->mType) {
    case BY_STRING: {
        VM_DELETE(ref->mAddress, gc->mBoyiaVM);
    } break;
    case BY_NAVCLASS: {
        NativeDelete(ref->mAddress);
    } break;
    case BY_CLASS: {
        BoyiaFunction* fun = (BoyiaFunction*)ref->mAddress;
        BoyiaPreDelete(fun, gc->mBoyiaVM);
        VM_DELETE(fun->mParams, gc->mBoyiaVM);
        VM_DELETE(fun, gc->mBoyiaVM);
    } break;
    }

    ref->mAddress = kBoyiaNull;
}

static LBool CheckValueTable(BoyiaRef* ref, BoyiaValue* table, LInt size)
{
    // 正式开始检查是否引用过期
    LInt idx = 0;
    for (; idx < size; idx++) {
        BoyiaValue* value = table + idx;
        if (CheckValue(value, ref)) {
            // 查出引用未过期，跳出循环
            return LTrue;
        }
    }

    return LFalse;
}

// 全栈中查找引用是否过期
static LVoid GCheckNoneRef(BoyiaRef* ref, BoyiaGC* gc)
{
    LInt stackAddr, size;
    GetLocalStack(&stackAddr, &size, gc->mBoyiaVM);
    BoyiaValue* stack = (BoyiaValue*)stackAddr;
    if (CheckValueTable(ref, stack, size)) {
        return;
    }

    GetGlobalTable(&stackAddr, &size, gc->mBoyiaVM);
    stack = (BoyiaValue*)stackAddr;
    if (CheckValueTable(ref, stack, size)) {
        return;
    }
    // 查找结果寄存器，是否有引用过期
    BoyiaValue* val = (BoyiaValue*)GetNativeResult(gc->mBoyiaVM);
    if (CheckValue(val, ref)) {
        return;
    }

    DeleteRef(ref, gc);
}

static LVoid GClearGarbage(BoyiaGC* gc)
{
    BoyiaRef* prev = gc->mBegin;
    while (prev && !prev->mAddress) {
        gc->mBegin = prev->mNext;
        FAST_DELETE(prev);
        --gc->mSize;
        prev = gc->mBegin;
    }

    if (!prev) {
        gc->mEnd = kBoyiaNull;
        return;
    }

    BoyiaRef* current = prev->mNext;
    while (current) {
        if (!current->mAddress) {
            prev->mNext = current->mNext;
            FAST_DELETE(current);
            --gc->mSize;
            current = prev->mNext;
        } else {
            prev = current;
            current = prev->mNext;
        }
    }

    gc->mEnd = prev;
}

extern LVoid GCAppendRef(LVoid* address, LUint8 type, LVoid* vm)
{
    //GCInit();
    BoyiaGC* gc = (BoyiaGC*)GetGabargeCollect(vm);
    BoyiaRef* ref = FAST_NEW(BoyiaRef);
    ref->mAddress = address;
    ref->mType = type;
    ref->mNext = kBoyiaNull;

    if (gc->mBegin) {
        gc->mEnd->mNext = ref;
    } else {
        gc->mBegin = ref;
    }

    gc->mEnd = ref;
    ++gc->mSize;
}

extern LVoid GCollectGarbage(LVoid* vm)
{
    BoyiaGC* gc = (BoyiaGC*)GetGabargeCollect(vm);
    if (!gc) {
        return;
    }

    KFORMATLOG("GCollect begin Size=%d\n", gc->mSize);
    BoyiaRef* ref = gc->mBegin;
    // 开始回收内存
    while (ref) {
        GCheckNoneRef(ref, gc);
        ref = ref->mNext;
    }

    GClearGarbage(gc);

    KFORMATLOG("GCollect end Size=%d\n", gc->mSize);
    //KFORMATLOG("GCollect end CollectSize=%d\n", sGc->mSize);
}

static LVoid MarkValue(BoyiaValue* value);
// 标记对象属性
static LVoid MarkObjectProps(BoyiaValue* value)
{
    BoyiaFunction* fun = (BoyiaFunction*)value->mValue.mIntVal;

    // 标记为灰色
    fun->mParamCount = GET_FUNCTION_COUNT(fun) | kBoyiaGcGray;
    LInt idx = 0;
    for (; idx < fun->mParamSize; ++idx) {
        MarkValue(&fun->mParams[idx]);
    }

    // 标记为黑色
    fun->mParamCount = GET_FUNCTION_COUNT(fun) | kBoyiaGcBlack;
}

// 标记对象
static LVoid MarkValue(BoyiaValue* value)
{
    /*
    if (value->mValueType == BY_NAVCLASS
        && value->mValue.mIntVal) {
        return LTrue;
    }*/

    /*
    if (value->mValueType == BY_STRING
        && (LIntPtr)value->mValue.mStrVal.mPtr == (LIntPtr)ref->mAddress) {
        return LTrue;
    }*/

    if (value->mValueType == BY_CLASS) {
        MarkObjectProps(value);
    }
}

// 标记表中元素
static LVoid MarkValueTable(BoyiaValue* table, LInt size)
{
    // 对引用的对象进行标记
    LInt idx = 0;
    for (; idx < size; idx++) {
        MarkValue(table + idx);
    }
}

static LVoid ClearAllGarbage(BoyiaGC* gc, LVoid* vm)
{
    BoyiaRef* prev = gc->mBegin;
    while (prev && prev->mType == BY_CLASS) {
        BoyiaFunction* fun = (BoyiaFunction*)prev->mAddress;
        if (IS_OBJECT_INVALID(fun)) {
            // 删除对象内存
            VM_DELETE(prev->mAddress, vm);
            // begin标记置为下一个元素
            gc->mBegin = prev->mNext;
            // 删除链表中的元素
            FAST_DELETE(prev);
            --gc->mSize;
            prev = gc->mBegin;
        } else {
            break;
        }
    }

    if (!prev) {
        gc->mEnd = kBoyiaNull;
        return;
    }

    BoyiaRef* current = prev->mNext;
    while (current) {
        BoyiaFunction* fun = (BoyiaFunction*)current->mAddress;
        if (current->mType == BY_CLASS && IS_OBJECT_INVALID(fun)) {
            VM_DELETE(prev->mAddress, vm);
            prev->mNext = current->mNext;
            FAST_DELETE(current);
            --gc->mSize;
            current = prev->mNext;
        } else {
            prev = current;
            current = prev->mNext;
        }
    }

    gc->mEnd = prev;
}

// 标记gcroots中引用的对象，垃圾回收标记清除
extern LVoid GCollectGarbageV2(LVoid* vm)
{
    BoyiaGC* gc = (BoyiaGC*)GetGabargeCollect(vm);
    LInt stackAddr, size;

    // 标记栈
    GetLocalStack(&stackAddr, &size, gc->mBoyiaVM);
    BoyiaValue* stack = (BoyiaValue*)stackAddr;
    MarkValueTable(stack, size);

    // 标记全局区
    GetGlobalTable(&stackAddr, &size, gc->mBoyiaVM);
    stack = (BoyiaValue*)stackAddr;
    MarkValueTable(stack, size);

    // 标记零时引用
    BoyiaValue* val = (BoyiaValue*)GetNativeResult(gc->mBoyiaVM);
    MarkValue(val);

    ClearAllGarbage(gc, vm);
}
