#ifndef MiniUtil_h
#define MiniUtil_h

#include "PlatformLib.h"

enum KeyWord {
    ARG = 1,
    CHAR,
    INT,
    STRING,
    FUNC,
    NAVCLASS,
    CLASS,
    EXTEND,
    VAR,
    PROP,
    IF,
    ELSE,
    DO,
    WHILE,
    BREAK,
    RETURN,
    END, // 17
};

typedef struct {
	LInt8*     mPtr;
	LInt       mLen;
} MiniStr;

typedef struct {
	LInt mPtr;
	LInt mSuper;
} MiniClass;

typedef struct {
	LUint   mNameKey; // HASH KEY 用来加快查找速度
	LUint8  mValueType;
	union RealValue {
		LInt       mIntVal;
		MiniClass  mObj; // 类应该存储于方法区
		MiniStr    mStrVal;
	} mValue;
} MiniValue;

#define NEW(type) (type*)MiniNew(sizeof(type))
#define NEW_ARRAY(type, n) (type*)MiniNew(n*sizeof(type))
#define DELETE(ptr) MiniDelete(ptr);
#define D_STR(str, len) {(LInt8*)str, len}

LVoid CreateMiniMemory();
LVoid* MiniNew(LInt size);
LVoid MiniDelete(LVoid* data);
//LUint HashCode(MiniStr* str);
LVoid InitStr(MiniStr* str, LInt8* ptr);
LBool MStrcmp(MiniStr* src, MiniStr* dest);
LVoid MStrcpy(MiniStr* dest, MiniStr* src);
LBool MStrchr(const LInt8 *s, LInt8 ch);
LVoid StringAdd(MiniValue* left, MiniValue* right);
LUint GenIdentifier(MiniStr* str);
LUint GenIdentByStr(const LInt8* str, LInt len);
#endif
