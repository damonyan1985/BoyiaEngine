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
} BoyiaGC;

// 收集器
static BoyiaGC* sGc = NULL;

extern LVoid NativeDelete(LVoid* data);
static LBool CheckValue(BoyiaValue* val, BoyiaRef* ref);

extern LVoid* CreateGC()
{
    BoyiaGC* gc = FAST_NEW(BoyiaGC);
    gc->mBegin = NULL;
    gc->mEnd = NULL;
    gc->mSize = 0;
    return gc;
}

extern LVoid ChangeGC(LVoid* gc)
{
    sGc = (BoyiaGC*)gc;
}

static LVoid GCInit()
{
    if (!sGc) {
        sGc = (BoyiaGC*)CreateGC();
    }
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

static LVoid DeleteRef(BoyiaRef* ref)
{
    switch (ref->mType) {
    case BY_STRING: {
		VM_DELETE(ref->mAddress);
    } break;
    case BY_NAVCLASS: {
        NativeDelete(ref->mAddress);
    } break;
    case BY_CLASS: {
        BoyiaFunction* fun = (BoyiaFunction*)ref->mAddress;
		VM_DELETE(fun->mParams);
		VM_DELETE(fun);
    } break;
    }

    ref->mAddress = NULL;
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
static LVoid GCheckNoneRef(BoyiaRef* ref)
{
    LInt stackAddr, size;
    GetLocalStack(&stackAddr, &size);
    BoyiaValue* stack = (BoyiaValue*)stackAddr;
    if (CheckValueTable(ref, stack, size)) {
        return;
    }

    GetGlobalTable(&stackAddr, &size);
    stack = (BoyiaValue*)stackAddr;
    if (CheckValueTable(ref, stack, size)) {
        return;
    }
    // 查找结果寄存器，是否有引用过期
    BoyiaValue* val = (BoyiaValue*)GetNativeResult();
    if (CheckValue(val, ref)) {
        return;
    }

    DeleteRef(ref);
}

static LVoid GClearGarbage()
{
    BoyiaRef* prev = sGc->mBegin;
    while (prev && !prev->mAddress) {
        sGc->mBegin = prev->mNext;
        FAST_DELETE(prev);
        --sGc->mSize;
        prev = sGc->mBegin;
    }

    if (!prev) {
        sGc->mEnd = NULL;
        return;
    }

    BoyiaRef* current = prev->mNext;
    while (current) {
        if (!current->mAddress) {
            prev->mNext = current->mNext;
            FAST_DELETE(current);
            --sGc->mSize;
            current = prev->mNext;
        } else {
            prev = current;
            current = prev->mNext;
        }
    }

    sGc->mEnd = prev;
}

extern LVoid GCAppendRef(LVoid* address, LUint8 type)
{
    GCInit();

    BoyiaRef* ref = FAST_NEW(BoyiaRef);
    ref->mAddress = address;
    ref->mType = type;
    ref->mNext = NULL;

    if (sGc->mBegin == NULL) {
        sGc->mBegin = ref;
    } else {
        sGc->mEnd->mNext = ref;
    }

    sGc->mEnd = ref;
    ++sGc->mSize;
}

extern LVoid GCollectGarbage()
{
    if (!sGc) {
        return;
    }

    KFORMATLOG("GCollect begin Size=%d\n", sGc->mSize);
    BoyiaRef* ref = sGc->mBegin;
    // 开始回收内存
    while (ref) {
        GCheckNoneRef(ref);
        ref = ref->mNext;
    }

    GClearGarbage();

    KFORMATLOG("GCollect end Size=%d\n", sGc->mSize);
    //KFORMATLOG("GCollect end CollectSize=%d\n", sGc->mSize);
}
