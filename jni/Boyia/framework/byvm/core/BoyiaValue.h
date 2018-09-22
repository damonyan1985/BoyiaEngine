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
} BoyiaStr;

typedef struct {
	LIntPtr mPtr;
	LIntPtr mSuper;
} BoyiaClass;

typedef struct {
    LUintPtr   mNameKey; // HASH KEY 用来加快查找速度
    LUint8     mValueType;
    union RealValue {
        LIntPtr     mIntVal;
        BoyiaClass  mObj; // 类应该存储于方法区
        BoyiaStr    mStrVal;
    } mValue;
} BoyiaValue;

#define NEW(type) (type*)BoyiaNew(sizeof(type))
#define NEW_ARRAY(type, n) (type*)BoyiaNew(n*sizeof(type))
#define DELETE(ptr) BoyiaDelete(ptr);
#define D_STR(str, len) {(LInt8*)str, len}

LVoid CreateMiniMemory();
LVoid* BoyiaNew(LInt size);
LVoid BoyiaDelete(LVoid* data);
//LUint HashCode(BoyiaStr* str);
LVoid InitStr(BoyiaStr* str, LInt8* ptr);
LBool MStrcmp(BoyiaStr* src, BoyiaStr* dest);
LVoid MStrcpy(BoyiaStr* dest, BoyiaStr* src);
LBool MStrchr(const LInt8* s, LInt8 ch);
LVoid StringAdd(BoyiaValue* left, BoyiaValue* right);
LUint GenIdentifier(BoyiaStr* str);
LUint GenIdentByStr(const LInt8* str, LInt len);
#endif
