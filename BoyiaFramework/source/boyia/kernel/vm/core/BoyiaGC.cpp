#include "BoyiaCore.h"
#include "BoyiaLib.h"
#include "BoyiaMemory.h"
#include "PlatformLib.h"
#include "SalLog.h"
#include <stdio.h>
#include <stdlib.h>

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

// 0000 0000 0000 0011
// 清除高14位所用
const LInt kBoyiaGcMask = 0x0003;

enum BoyiaGcColor {
    kBoyiaGcWhite = 0x0,
    kBoyiaGcGray = 0x1,
    kBoyiaGcBlack = 0x2
};

#define IS_OBJECT_INVALID(fun) (((fun->mParamCount >> 16) & kBoyiaGcMask) == kBoyiaGcWhite)
#define IS_NATIVE_STRING(fun) ((fun->mParamCount >> 18) == kNativeStringBuffer)
#define IS_BOYIA_STRING(fun) ((fun->mParamCount >> 18) == kBoyiaStringBuffer)

// 收集器
extern LVoid NativeDelete(LVoid* data);

extern LVoid* CreateGC(LVoid* vm)
{
    BoyiaGC* gc = FAST_NEW(BoyiaGC);
    gc->mBegin = kBoyiaNull;
    gc->mEnd = kBoyiaNull;
    gc->mSize = 0;
    gc->mBoyiaVM = vm;
    return gc;
}

extern LVoid GCAppendRef(LVoid* address, LUint8 type, LVoid* vm)
{
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

static LVoid MarkValue(BoyiaValue* value);
// 标记对象属性
static LVoid MarkObjectProps(BoyiaValue* value)
{
    BoyiaFunction* fun = (BoyiaFunction*)value->mValue.mObj.mPtr;

    // 标记为灰色
    fun->mParamCount = GET_FUNCTION_COUNT(fun) | (kBoyiaGcGray << 16);
    LInt idx = 0;
    for (; idx < fun->mParamSize; ++idx) {
        MarkValue(&fun->mParams[idx]);
    }

    // 标记为黑色
    fun->mParamCount = GET_FUNCTION_COUNT(fun) | (kBoyiaGcBlack << 16);
}

// 标记对象
static LVoid MarkValue(BoyiaValue* value)
{
    // 标记native对象
    if (value->mValueType == BY_NAVCLASS) {
        MarkNativeObject(value->mValue.mIntVal, kBoyiaGcBlack);
        return;
    }

    // 标记boyia对象
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

static LBool IsInValidObject(BoyiaRef* ref)
{
    if (ref->mType == BY_CLASS) {
        BoyiaFunction* fun = (BoyiaFunction*)ref->mAddress;
        return IS_OBJECT_INVALID(fun);
    }

    if (ref->mType != BY_NAVCLASS) {
        return LFalse;
    }

    if (NativeObjectFlag(ref->mAddress) != kBoyiaGcWhite) {
        // 重置为白色
        MarkNativeObject((LIntPtr)ref->mAddress, kBoyiaGcWhite);
        return LFalse;
    }

    return LTrue;
}

static LVoid DeleteObject(BoyiaRef* ref, LVoid* vm)
{
    if (ref->mType == BY_NAVCLASS) {
        NativeDelete(ref->mAddress);
        return;
    }

    if (ref->mType != BY_CLASS) {
        return;
    }

    BoyiaFunction* objBody = (BoyiaFunction*)ref->mAddress;
    BoyiaValue* kclass = (BoyiaValue*)objBody->mFuncBody;
    LUintPtr classId = kclass ? kclass->mNameKey : kBoyiaNull;
    if (classId == kBoyiaString) {
        BoyiaStr* buffer = &objBody->mParams[0].mValue.mStrVal;
        if (IS_NATIVE_STRING(objBody)) {
            free(buffer->mPtr);
        } else if (IS_BOYIA_STRING(objBody)) {
            VM_DELETE(buffer->mPtr, vm);
        } // 常量字符串不做任何处理
    }

    VM_DELETE(ref->mAddress, vm);
}

// 清除所有需要回收的对象
static LVoid ClearAllGarbage(BoyiaGC* gc, LVoid* vm)
{
    BoyiaRef* prev = gc->mBegin;
    while (prev) {
        if (IsInValidObject(prev)) {
            // 删除对象内存
            DeleteObject(prev, vm);
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
        if (IsInValidObject(current)) {
            DeleteObject(current, vm);
            // 指向下一个引用
            prev->mNext = current->mNext;
            // 删除引用节点
            FAST_DELETE(current);
            // gc中引用数量减一
            --gc->mSize;
            // 切换当前指针
            current = prev->mNext;
        } else {
            prev = current;
            current = current->mNext;
        }
    }

    gc->mEnd = prev;
}

// 标记gcroots中引用的对象，垃圾回收标记清除
extern LVoid GCollectGarbage(LVoid* vm)
{
    BoyiaGC* gc = (BoyiaGC*)GetGabargeCollect(vm);
    LIntPtr stackAddr;
    LInt size = 0;

    // 标记栈
    GetLocalStack(&stackAddr, &size, gc->mBoyiaVM);
    BoyiaValue* stack = (BoyiaValue*)stackAddr;
    MarkValueTable(stack, size);

    // 标记全局区
    GetGlobalTable(&stackAddr, &size, gc->mBoyiaVM);
    stack = (BoyiaValue*)stackAddr;
    MarkValueTable(stack, size);

    ClearAllGarbage(gc, vm);
}
