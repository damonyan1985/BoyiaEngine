#include "BoyiaCore.h"
#include "BoyiaLib.h"
#include "BoyiaMemory.h"
#include "PlatformLib.h"
#include "SalLog.h"
#include <stdio.h>
#include <stdlib.h>

#define MIGRATE_SIZE (6*1024)
#define kBoyiaRefPageSize (64 * 1024)

typedef struct BoyiaRef {
    LVoid* mAddress;
    LUint8 mType;
    BoyiaRef* mNext;
} BoyiaRef;

typedef struct UsedRefs {
    BoyiaRef* mBegin;
    BoyiaRef* mEnd;
} UsedRefs;

typedef struct BoyiaGc {
    UsedRefs mUsedRefs;
    //LInt mSize;

    //LInt mUseIndex;
    //BoyiaRef* mRefs;
    //BoyiaRef* mFreeRefs;
    LVoid* mRefCache;
    LVoid* mBoyiaVM;
    LVoid* mMigrates[MIGRATE_SIZE];
} BoyiaGc;

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

// 计算迁移标记,后20位为迁移标记
#define MIGRATE_FLAG(fun) ((fun->mParamCount >> 20) & 0x1)
// 设置迁移标记
#define SET_MIGRATE_FLAG(fun) (fun->mParamCount |= (0x1 << 20))

static BoyiaRef* AllocateRef(BoyiaGc* gc)
{
    BoyiaRef* ref = ALLOC_CHUNK(BoyiaRef, gc->mRefCache);
    if (ref) {
        ref->mNext = kBoyiaNull;
    }

    return ref;
}

static LVoid FreeRef(BoyiaRef* ref, BoyiaGc* gc)
{
    FREE_CHUNK(ref, gc);
}

// 收集器
extern LVoid NativeDelete(LVoid* data);

extern LVoid* CreateGC(LVoid* vm)
{
    BoyiaGc* gc = FAST_NEW(BoyiaGc);

    gc->mUsedRefs.mBegin = kBoyiaNull;
    gc->mUsedRefs.mEnd = kBoyiaNull;

    gc->mRefCache = CREATE_MEMCACHE(BoyiaRef, kBoyiaRefPageSize);
    return gc;
}

extern LVoid DestroyGC(LVoid* vm)
{
    BoyiaGc* gc = (BoyiaGc*)GetGabargeCollect(vm);
    DESTROY_MEMCACHE(gc->mRefCache);
    FAST_DELETE(gc);
}

extern LVoid GCAppendRef(LVoid* address, LUint8 type, LVoid* vm)
{
    BoyiaGc* gc = (BoyiaGc*)GetGabargeCollect(vm);
    BoyiaRef* ref = AllocateRef(gc);

    ref->mAddress = address;
    ref->mType = type;
    ref->mNext = kBoyiaNull;

    UsedRefs* refs = &gc->mUsedRefs;

    if (refs->mBegin) {
        refs->mEnd->mNext = ref;
    } else {
        refs->mBegin = ref;
    }

    refs->mEnd = ref;
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
            FREE_BUFFER(buffer->mPtr);
        } else if (IS_BOYIA_STRING(objBody)) {
            VM_DELETE(buffer->mPtr, vm);
        } // 常量字符串不做任何处理
    }

    VM_DELETE(ref->mAddress, vm);
}

// 清除所有需要回收的对象
static LVoid ClearAllGarbage(BoyiaGc* gc, LVoid* vm)
{
    UsedRefs* refs = &gc->mUsedRefs;
    BoyiaRef* prev = refs->mBegin;
    while (prev) {
        if (IsInValidObject(prev)) {
            // 删除对象内存
            DeleteObject(prev, vm);
            // begin标记置为下一个元素
            refs->mBegin = prev->mNext;
            // 删除链表中的元素
            //FAST_DELETE(prev);
            FreeRef(prev, gc);
            //--gc->mSize;
            prev = refs->mBegin;
        } else {
            break;
        }
    }

    if (!prev) {
        refs->mEnd = kBoyiaNull;
        return;
    }

    BoyiaRef* current = prev->mNext;
    while (current) {
        if (IsInValidObject(current)) {
            DeleteObject(current, vm);
            // 指向下一个引用
            prev->mNext = current->mNext;
            // 删除引用节点
            //FAST_DELETE(current);
            FreeRef(current, gc);
            // gc中引用数量减一
            //--gc->mSize;
            // 切换当前指针
            current = prev->mNext;
        } else {
            prev = current;
            current = current->mNext;
        }
    }

    refs->mEnd = prev;
}

static LVoid ResetMigrateAddress(BoyiaValue* value, LInt* migrateIndexPtr, LVoid* toPool, BoyiaFunction* fun, BoyiaGc* gc)
{
    LInt index = MIGRATE_FLAG(fun);
    LIntPtr address = (LIntPtr)gc->mMigrates[index];

    if (value->mValueType == BY_CLASS) {
        value->mValue.mObj.mPtr = address;
        BoyiaValue* kclass = (BoyiaValue*)fun->mFuncBody;
        LUintPtr classId = kclass ? kclass->mNameKey : kBoyiaNull;
        // 如果是字符串对象
        if (classId == kBoyiaString) {
            BoyiaStr* buffer = GetStringBufferFromBody(fun);
            if (IS_BOYIA_STRING(fun)) {
                buffer->mPtr = (LInt8*)MigrateRuntimeMemory(buffer->mPtr, toPool, gc->mBoyiaVM);
            }
            // TODO 常量字符串表需要改造，不处理迁移
        }
    } else {
        value->mValue.mObj.mSuper = address;
    }
}

// 返回值表示是否需要遍历对象
static LVoid MigrateObject(BoyiaValue* value, LInt* migrateIndexPtr, LVoid* toPool, BoyiaGc* gc)
{
    BoyiaFunction* fun = (BoyiaFunction*)(value->mValueType == BY_CLASS 
        ? value->mValue.mObj.mPtr
        : value->mValue.mObj.mSuper);
    if (!MIGRATE_FLAG(fun)) {
        gc->mMigrates[*migrateIndexPtr] = MigrateRuntimeMemory(fun, toPool, gc->mBoyiaVM);
        SET_MIGRATE_FLAG(fun);
        (*migrateIndexPtr)++;
    }

    ResetMigrateAddress(value, migrateIndexPtr, toPool, fun, gc);
}

static LVoid MigrateValue(BoyiaValue* value, LInt* migrateIndexPtr, LVoid* toPool, BoyiaGc* gc)
{
    BoyiaFunction* fun = kBoyiaNull;
    if (value->mValueType == BY_CLASS) {
        fun = (BoyiaFunction*)value->mValue.mObj.mPtr;
    } else if (value->mValueType == BY_PROP_FUNC) {
        fun = (BoyiaFunction*)value->mValue.mObj.mSuper;
    }

    if (!fun) {
        return;
    }

    // 如果没有标记才需要迁移属性
    if (!MIGRATE_FLAG(fun)) {
        // 迁移对象属性
        LInt pIdx = 0;
        for (; pIdx < fun->mParamSize; ++pIdx) {
            MigrateValue(&fun->mParams[pIdx], migrateIndexPtr, toPool, gc);
        }
    }
    
    // 迁移对象
    // 1,如果对象没有被迁移，则迁移对象，但需要标记迁移过对象对应的引用
    // 2,判断标记，如果对象被迁移过了，则根据标记取出全局引用对应的新的对象地址
    MigrateObject(value, migrateIndexPtr, toPool, gc);
    
}

static LVoid MigrateValueTable(BoyiaValue* table, LInt size, LInt* migrateIndexPtr, LVoid* toPool, BoyiaGc* gc)
{
    LInt idx = 0;
    for (; idx < size; idx++) {
        MigrateValue(table + idx, migrateIndexPtr, toPool, gc);
    }
}

static LVoid ResetGCRef(BoyiaGc* gc)
{
    BoyiaRef* ref = gc->mUsedRefs.mBegin;
    while (ref) {
        if (ref->mType == BY_CLASS) {
            BoyiaFunction* fun = (BoyiaFunction*)ref->mAddress;
            if (MIGRATE_FLAG(fun)) {
                LInt index = MIGRATE_FLAG(fun);
                ref->mAddress = gc->mMigrates[index];
            }
        }

        ref = ref->mNext;
    }
}

// 内存碎片整理
// 1，当剩余内存不足以分配空间时，gc一次，如果还不行，则执行次操作
// 2，如果碎片整理后还无法分配内存，则VM将报OOM
static LVoid CompactMemory(BoyiaGc* gc)
{
    LInt migrateIndex = 0;
    LMemset(gc->mMigrates, 0, MIGRATE_SIZE);
    // 切换from与to内存空间
    LVoid* toPool = CreateRuntimeToMemory(gc->mBoyiaVM);

    LIntPtr tableAddr;
    LInt size = 0;

    // 获取全局对象引用
    GetGlobalTable(&tableAddr, &size, gc->mBoyiaVM);
    BoyiaValue* global = (BoyiaValue*)tableAddr;
    MigrateValueTable(global, size, &migrateIndex, toPool, gc);

    // 标记栈
    LVoid* ptr = gc->mBoyiaVM;
    do {
        ptr = GetLocalStack(&tableAddr, &size, gc->mBoyiaVM, ptr);
        BoyiaValue* stack = (BoyiaValue*)tableAddr;
        MigrateValueTable(global, size, &migrateIndex, toPool, gc);
    } while (ptr);
    
    UpdateRuntimeMemory(toPool, gc->mBoyiaVM);
    ResetGCRef(gc);
}

// 标记boyia对象
static LVoid ResetBoyiaObject(BoyiaFunction* fun)
{
    // high保留字符串标记，清除掉GC标记
    LInt high = fun->mParamCount >> 18;
    LInt low = GET_FUNCTION_COUNT(fun);
    fun->mParamCount = (high << 18) | low;
}

// 重置对象内存颜色位白色
static LVoid ResetMemoryColor(BoyiaGc* gc)
{
    // 在GC列表中的都是动态生成的对象
    BoyiaRef* ref = gc->mUsedRefs.mBegin;
    while (ref) {
        if (ref->mType == BY_NAVCLASS) {
            MarkNativeObject((LIntPtr)ref->mAddress, kBoyiaGcWhite);
        } else if (ref->mType == BY_CLASS) {
            ResetBoyiaObject((BoyiaFunction*)ref->mAddress);
        }
        
        ref = ref->mNext;
    }
    
    // 不在GC列表中的是全局对象，也需要重置
    // 全局对象的地址在方法区，而不是动态生成的，不在gc列表中
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
    BoyiaGc* gc = (BoyiaGc*)GetGabargeCollect(vm);
    // 重置对象内存颜色
    ResetMemoryColor(gc);
    
    LIntPtr stackAddr;
    LInt size = 0;

    // 标记全局区
    GetGlobalTable(&stackAddr, &size, gc->mBoyiaVM);
    BoyiaValue* stack = (BoyiaValue*)stackAddr;
    MarkValueTable(stack, size);

    // 标记栈
    LVoid* ptr = gc->mBoyiaVM;
    do {
        ptr = GetLocalStack(&stackAddr, &size, gc->mBoyiaVM, ptr);
        stack = (BoyiaValue*)stackAddr;
        MarkValueTable(stack, size);
    } while (ptr);

    // 标记微任务
    ptr = gc->mBoyiaVM;
    do {
        BoyiaValue* value;
        ptr = IterateMicroTask(&value, gc->mBoyiaVM, ptr);
        if (value) {
            MarkValue(value);
        }
    } while (ptr);
    
    ClearAllGarbage(gc, vm);
}
