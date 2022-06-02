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
#define IS_NATIVE_STRING(fun) (((fun->mParamCount >> 18) & kBoyiaGcMask) == kNativeStringBuffer)
#define IS_BOYIA_STRING(fun) (((fun->mParamCount >> 18) & kBoyiaGcMask) == kBoyiaStringBuffer)

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

extern LVoid DestroyGC(LVoid* vm)
{
    BoyiaGC* gc = (BoyiaGC*)GetGabargeCollect(vm);
    BoyiaRef* ref = gc->mBegin;
    while (ref) {
        BoyiaRef* tmp = ref;
        ref = ref->mNext;
        FAST_DELETE(tmp);
    }

    FAST_DELETE(gc);
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
    BoyiaFunction* fun = kBoyiaNull;
    
    if (value->mValueType == BY_CLASS) {
        fun = (BoyiaFunction*)value->mValue.mObj.mPtr;
    } else if (value->mValueType == BY_PROP_FUNC) {
        // 如果是属性方法，则从super中获取对象地址
        fun = (BoyiaFunction*)value->mValue.mObj.mSuper;
    }
    
    // fun为空，或者对象已经被标记过了
    if (!fun || !IS_OBJECT_INVALID(fun)) {
        return;
    }
    
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
    if (value->mValueType == BY_CLASS || value->mValueType == BY_PROP_FUNC) {
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
        // MarkNativeObject((LIntPtr)ref->mAddress, kBoyiaGcWhite);
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
        //BoyiaStr* buffer = &objBody->mParams[1].mValue.mStrVal;
        BoyiaStr* buffer = GetStringBufferFromBody(objBody);
        // 删除字符串对象中的缓冲数据
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

// 挤压内存
//static LVoid CompactMemory()
//{
//
//}

// 标记boyia对象
static LVoid ResetBoyiaObject(BoyiaFunction* fun)
{
    //BoyiaFunction* fun = (BoyiaFunction*)ref->mAddress;
    // high保留字符串标记，清除掉GC标记
    LInt high = fun->mParamCount >> 18;
    LInt low = GET_FUNCTION_COUNT(fun);
    fun->mParamCount = (high << 18) | low;
    
    //LInt flag = (fun->mParamCount >> 16) & kBoyiaGcMask;
}

// 重置对象内存颜色位白色
static LVoid ResetMemoryColor(BoyiaGC* gc)
{
    // 在GC列表中的都是动态生成的对象
    BoyiaRef* ref = gc->mBegin;
    while (ref) {
        if (ref->mType == BY_NAVCLASS) {
            MarkNativeObject((LIntPtr)ref->mAddress, kBoyiaGcWhite);
        } else if (ref->mType == BY_CLASS) {
            ResetBoyiaObject((BoyiaFunction*)ref->mAddress);
        }
        
        ref = ref->mNext;
    }
    
    // 不在GC列表中的是全局对象，也需要重置
    LIntPtr stackAddr;
    LInt size = 0;
    GetGlobalTable(&stackAddr, &size, gc->mBoyiaVM);
    BoyiaValue* stack = (BoyiaValue*)stackAddr;
    
    LInt idx = 0;
    for (; idx < size; idx++) {
        BoyiaValue* val = stack + idx;
        if (val->mValueType == BY_CLASS) {
            ResetBoyiaObject((BoyiaFunction*)val->mValue.mObj.mPtr);
        }
    }
}

// 标记gcroots中引用的对象，垃圾回收标记清除
extern LVoid GCollectGarbage(LVoid* vm)
{
    BoyiaGC* gc = (BoyiaGC*)GetGabargeCollect(vm);
    // 重置对象内存颜色
    ResetMemoryColor(gc);
    
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
