//
// Created by yanbo on 2017/9/26.
//

#include "PlatformLib.h"
#include "BoyiaMemory.h"
#include "AutoLock.h"
#include "MiniMutex.h"
#include "BoyiaBase.h"
#include "SalLog.h"
#include "IDCreator.h"
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>

#define MAX_INT_LEN 20
#define MINI_ANDROID_LOG
#define MEMORY_SIZE           (LInt)1024*1024*6
static LVoid*        gMemPool = NULL;

extern LVoid GCAppendRef(LVoid* address, LUint8 type);
extern LVoid GCollectGarbage();

extern void jsLog(const char* format, ...) {
	va_list args;
	va_start(args, format);
#ifdef MINI_ANDROID_LOG
	__android_log_vprint(ANDROID_LOG_INFO, "BoyiaVM", format, args);
#else
    printf(format, args);
#endif
    va_end(args);
}

extern LInt Str2Int(LInt8* p, LInt len, LInt radix) {
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

LVoid ChangeMemory(LVoid* mem) {
	gMemPool = mem;
}

static LVoid SystemGC() {
    if (GetUsedMemory(gMemPool) >= MEMORY_SIZE/2) {
        GCollectGarbage();
    }
}

LVoid* BoyiaNew(LInt size) {
	SystemGC();
	LVoid* data = NewData(size, gMemPool);
	PrintPoolSize(gMemPool);
	return data;
}

LVoid BoyiaDelete(LVoid* data) {
    DeleteData(data, gMemPool);
}

LVoid MStrcpy(BoyiaStr* dest, BoyiaStr* src) {
    dest->mPtr = src->mPtr;
    dest->mLen = src->mLen;
}

LVoid InitStr(BoyiaStr* str, LInt8* ptr) {
    str->mLen = 0;
    str->mPtr = ptr;
}

LBool MStrchr(const LInt8* s, LInt8 ch) {
    while (*s && *s != ch) ++s;
    return *s && *s == ch;
}

LBool MStrcmp(BoyiaStr* src, BoyiaStr* dest) {
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

extern LVoid NativeDelete(LVoid* data) {
	delete (boyia::BoyiaBase*) data;
}

// "Hello" + "World"
static LVoid FetchString(BoyiaStr* str, BoyiaValue* value) {
	if (value->mValueType == INT) {
		str->mPtr = NEW_ARRAY(LInt8, MAX_INT_LEN);
		LMemset(str->mPtr, 0, MAX_INT_LEN);
		LInt2StrWithLength(value->mValue.mIntVal, (LUint8*)str->mPtr, 10, &str->mLen);
	} else {
		str->mPtr = value->mValue.mStrVal.mPtr;
		str->mLen = value->mValue.mStrVal.mLen;
	}
}

extern LVoid StringAdd(BoyiaValue* left, BoyiaValue* right) {
	KLOG("StringAdd Begin");
	BoyiaStr leftStr, rightStr;
	LInt8 tmpArray[MAX_INT_LEN];
	leftStr.mPtr = tmpArray;
	rightStr.mPtr = tmpArray;
	FetchString(&leftStr, left);
	FetchString(&rightStr, right);

	LInt len = leftStr.mLen + rightStr.mLen;
	LInt8* str = NEW_ARRAY(LInt8, len);

	LMemcpy(str, leftStr.mPtr, leftStr.mLen);
	LMemcpy(str + leftStr.mLen, rightStr.mPtr, rightStr.mLen);
	right->mValue.mStrVal.mPtr = str;
	right->mValue.mStrVal.mLen = len;
	right->mValueType = STRING;

	GCAppendRef(str, STRING);
	KLOG("StringAdd End");
}

static util::IDCreator* s_byidCreator = NULL;
LUint GenIdentByStr(const LInt8* str, LInt len) {
	if (!s_byidCreator) {
		s_byidCreator = new util::IDCreator();
	}

	return s_byidCreator->genIdentByStr(str, len);
}

LUint GenIdentifier(BoyiaStr* str) {
    if (!s_byidCreator) {
    	s_byidCreator = new util::IDCreator();
    }

    return s_byidCreator->genIdentifier(str);
}
