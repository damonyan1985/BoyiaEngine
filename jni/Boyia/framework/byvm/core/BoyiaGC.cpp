#include "PlatformLib.h"
#include "BoyiaCore.h"
#include "BoyiaLib.h"
#include "BoyiaMemory.h"
#include "AutoLock.h"
#include "SalLog.h"
#include <stdio.h>

typedef struct MiniRef {
	LVoid*    mAddress;
	LUint8    mType;
	MiniRef*  mNext;
} MiniRef;

typedef struct MiniGC {
	MiniRef* mBegin;
	MiniRef* mEnd;
	LInt     mSize;
} MiniGC;

// 收集器
static MiniGC* sGc = NULL;

extern LVoid NativeDelete(LVoid* data);
static LBool checkValue(BoyiaValue* val, MiniRef* ref);

static LVoid GCInit() {
	if (sGc == NULL) {
		sGc = FAST_NEW(MiniGC);
		sGc->mBegin = NULL;
		sGc->mEnd = NULL;
		sGc->mSize = 0;
	}
}

static LBool GCCheckObject(BoyiaValue* value, MiniRef* ref) {
	if (value->mValue.mIntVal == (LIntPtr)ref->mAddress) {
		return LTrue;
	}

	BoyiaFunction* fun = (BoyiaFunction*) value->mValue.mIntVal;
	LInt idx = 0;
	for (; idx < fun->mParamSize; ++idx) {
		if (checkValue(&fun->mParams[idx], ref)) {
			return LTrue;
		}
	}

	return LFalse;
}

static LBool checkValue(BoyiaValue* value, MiniRef* ref) {
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

static LVoid deleteRef(MiniRef* ref) {
	switch (ref->mType) {
	case BY_STRING: {
		    DELETE(ref->mAddress);
	    }
	    break;
	case BY_NAVCLASS: {
		    NativeDelete(ref->mAddress);
	    }
	    break;
	case BY_CLASS: {
		    BoyiaFunction* fun = (BoyiaFunction*)ref->mAddress;
			DELETE(fun->mParams);
			DELETE(fun);
	    }
	    break;
	}

	ref->mAddress = NULL;
}

static LBool checkValueTable(MiniRef* ref, BoyiaValue* table, LInt size) {
	// 正式开始检查是否引用过期
	LInt idx = 0;
	for (; idx < size; idx++) {
		BoyiaValue* value = table + idx;
		if (checkValue(value, ref)) {
			// 查出引用未过期，跳出循环
			return LTrue;
		}
	}

	return LFalse;
}

// 全栈中查找引用是否过期
static LVoid GCheckNoneRef(MiniRef* ref) {
	LInt stackAddr, size;
	GetLocalStack(&stackAddr, &size);
	BoyiaValue* stack = (BoyiaValue*)stackAddr;
	if (checkValueTable(ref, stack, size)) {
		return;
	}

	GetGlobalTable(&stackAddr, &size);
	stack = (BoyiaValue*)stackAddr;
	if (checkValueTable(ref, stack, size)) {
		return;
	}
	// 查找结果寄存器，是否有引用过期
	BoyiaValue* val = (BoyiaValue*)GetNativeResult();
	if (checkValue(val, ref)) {
		return;
	}

	deleteRef(ref);
}

static LVoid GClearGarbage() {
	MiniRef* prev = sGc->mBegin;
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

	MiniRef* current = prev->mNext;
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

extern LVoid GCAppendRef(LVoid* address, LUint8 type) {
	GCInit();

	MiniRef* ref = FAST_NEW(MiniRef);
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

extern LVoid GCollectGarbage() {
	if (!sGc) {
		return;
	}

	KFORMATLOG("GCollect begin Size=%d\n", sGc->mSize);
	MiniRef* ref = sGc->mBegin;
	// 开始回收内存
	while (ref) {
		GCheckNoneRef(ref);
		ref = ref->mNext;
	}

	GClearGarbage();

	KFORMATLOG("GCollect end Size=%d\n", sGc->mSize);
	//KFORMATLOG("GCollect end CollectSize=%d\n", sGc->mSize);
}
