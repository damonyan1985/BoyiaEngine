#include "PlatformLib.h"
#include "MiniCore.h"
#include "MiniLib.h"
#include "MiniMemory.h"
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
static LBool checkValue(MiniValue* val, MiniRef* ref);

static LVoid GCInit() {
	if (sGc == NULL) {
		sGc = new MiniGC;
		sGc->mBegin = NULL;
		sGc->mEnd = NULL;
		sGc->mSize = 0;
	}
}

static LBool GCCheckObject(MiniValue* value, MiniRef* ref) {
	if (value->mValue.mIntVal == (LInt)ref->mAddress) {
		return LTrue;
	}

	MiniFunction* fun = (MiniFunction*) value->mValue.mIntVal;
	LInt idx = 0;
	for (; idx < fun->mParamSize; ++idx) {
		if (checkValue(&fun->mParams[idx], ref)) {
			return LTrue;
		}
	}

	return LFalse;
}

static LBool checkValue(MiniValue* value, MiniRef* ref) {
	if ((value->mValueType == M_NAVCLASS || value->mValueType == M_STRING)
			&& value->mValue.mIntVal == (LInt)ref->mAddress) {
		return LTrue;
	}

    if (value->mValueType == M_CLASS && GCCheckObject(value, ref)) {
    	return LTrue;
    }

    return LFalse;
}

static LVoid deleteRef(MiniRef* ref) {
	switch (ref->mType) {
	case M_STRING: {
		    DELETE(ref->mAddress);
	    }
	    break;
	case M_NAVCLASS: {
		    NativeDelete(ref->mAddress);
	    }
	    break;
	case M_CLASS: {
		    MiniFunction* fun = (MiniFunction*)ref->mAddress;
			DELETE(fun->mParams);
			DELETE(fun);
	    }
	    break;
	}

	ref->mAddress = NULL;
}

static LBool checkValueTable(MiniRef* ref, MiniValue* table, LInt size) {
	// 正式开始检查是否引用过期
	LInt idx = 0;
	for (; idx < size; idx++) {
		MiniValue* value = table + idx;
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
	MiniValue* stack = (MiniValue*)stackAddr;
	if (checkValueTable(ref, stack, size)) {
		return;
	}

	GetGlobalTable(&stackAddr, &size);
	stack = (MiniValue*)stackAddr;
	if (checkValueTable(ref, stack, size)) {
		return;
	}
	// 查找结果寄存器，是否有引用过期
	MiniValue* val = (MiniValue*)GetNativeResult();
	if (checkValue(val, ref)) {
		return;
	}

	deleteRef(ref);
}

static LVoid GClearGarbage() {
	MiniRef* prev = sGc->mBegin;
	while (prev && !prev->mAddress) {
		sGc->mBegin = prev->mNext;
		delete prev;
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
			delete current;
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

	MiniRef* ref = new MiniRef;
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
