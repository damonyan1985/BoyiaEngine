/*
============================================================================
 Name        : PlatformLib.h
 Author      : yanbo
 Version     : 1.0
 Copyright   : All Copyright Reserved
 Description : support Original
============================================================================
*/

#ifndef PlatformLib_h
#define PlatformLib_h

namespace util {
#define LConst const
#define LFinal final
#define LOverride override

#ifndef kBoyiaNull
#define kBoyiaNull 0
#endif

#ifndef NULL
#define NULL 0
//#define NULL nullptr
#endif

#define ENABLE(FEATURE) (defined ENABLE_##FEATURE && ENABLE_##FEATURE)
#define UNABLE(FEATURE) (!defined ENABLE_##FEATURE || !ENABLE_##FEATURE)

#define LCHARSW(CHARS) L##CHARS

#define LCHARSA(CHARS) CHARS

#define LCHARS LCHARSA

#define LSTATIC_CAST(typ, object) static_cast<typ>(object)

#define LMax(a, b) ((a) >= (b) ? (a) : (b))

#define LMin(a, b) ((a) >= (b) ? (b) : (a))

#define NEW_BUFFER(type, size) ((type*)malloc(sizeof(type) * (size)))
#define FREE_BUFFER(buffer) (free(buffer))

#define KB (1024)
#define MB (KB * KB)
#define GB (MB * KB)
    
typedef char LInt8;
typedef short int LInt16;
typedef int LInt32;
typedef long long LInt64;

typedef unsigned char LUint8;
typedef unsigned short int LUint16;
typedef unsigned int LUint32;
typedef unsigned long long LUint64;
typedef unsigned long LDWORD;
typedef unsigned short LWORD;

#if defined(__LP64__)
typedef int LInt;
// Windows上不管是64位还是32位long都是4个字节，
// 而在Android 64位平台long是8个字节
// 所以64位系统，指针统一用long long
typedef long long LIntPtr, LOffset;
typedef unsigned long long LUintPtr, LSizeT;
typedef unsigned int LUint;
#else
typedef int LInt;
typedef int LIntPtr, LOffset;
typedef unsigned int LUint;
typedef unsigned int LUintPtr, LSizeT;
#endif

typedef void LVoid;

typedef float LReal32;
typedef double LReal64;

typedef LReal32 LReal;

typedef LInt LBool;

typedef char LCharA;

typedef LCharA LChar;

typedef enum Bool {
    LFalse = 0,
    LTrue
} Bool;

typedef LUint8 LByte;
typedef LUint16 UChar;

typedef LDWORD LCOLORREF;

typedef LByte LBYTE;

#define _WL(a) (const util::LUint16) L##a

// LString<LUint8> str = _CL('b')
#define _CL(a) (const util::LUint8) a

#define LLOBYTE(w) ((LBYTE)(((LDWORD)(w)) & 0xff))

#define LRGB(r, g, b) ((LCOLORREF)(((LBYTE)(r) | ((LDWORD)((LBYTE)(g)) << 8)) | (((LDWORD)(BYTE)(b)) << 16)))

#define LGetAValue(rgb) (LLOBYTE((rgb) >> 24))
#define LGetRValue(rgb) (LLOBYTE((rgb) >> 16))
#define LGetGValue(rgb) (LLOBYTE(((LWORD)(rgb)) >> 8))
#define LGetBValue(rgb) (LLOBYTE(rgb))

#define UALLOC(c) new c
#define UALLOC_ARG1(c, a) new c(a)
#define UALLOC_ARG2(c, a1, a2) new c(a1, a2)
#define UALLOC_ARG3(c, a1, a2, a3) new c(a1, a2, a3)
#define UALLOC_ARG4(c, a1, a2, a3, a4) new c(a1, a2, a3, a4)
#define UALLOCN(c, n) new c[n]
#define UFREE(p) delete p

#define MAX_PATH_SIZE 256

// Function in argument, Only for remark
#define In
// Function out argument, Only for remark
#define Out

LInt LStrlen(const LUint8* aStr);

LInt LStrcmp(const LCharA* src, const LCharA* cmpStr);

LInt LStrncmp(const LCharA* s1, const LCharA* s2, LInt n);

LInt LStrincmp(const LCharA* src, const LCharA* cmpStr, LInt maxlen);

LInt8* LStrcpy(LInt8* aDest, const LInt8* aSrc);

// Memory Operation
LVoid* LMemcpy(LVoid* aDest, const LVoid* aSrc, LInt aLength);
LVoid* LMemmove(LVoid* dest, const LVoid* source, LInt count);
LInt LMemcmp(const LVoid* buffer1, const LVoid* buffer2, LInt count);
LVoid* LMemset(LVoid* aDest, LInt aValue, LInt aLength);

LVoid LStrA2U(const LUint8* aStrA, LUint16* aStrW);
LVoid LStrU2A(LUint8* aStrA, const LUint16* aStrW);

LUint8* LInt2Str(LInt value, LUint8* str, LInt radix);
LUint8* LInt2StrWithLength(LInt value, LUint8* str, LInt radix, LInt* lenPtr);
LInt LStr2Int(const LUint8* str, LInt radix);
LInt LStr2IntWithLength(const LUint8* str, LInt length, LInt radix);

LChar* LStrchr(const LChar* s, LChar c);

LBool LIsSpace(LInt ch);
const LChar* LSkipSpace(const LChar* str);
LChar* LSkipString(LChar* src, const LChar* skip);

LInt LCeil(LReal64 f);
LInt LFloor(LReal64 f);
LReal64 LRound(LReal64 num);

LReal toRadians(LReal angle);

LReal LFabs(LReal f);
LBool LIsDigit(LInt ch);
LBool LIsBigChar(LInt ch);
LBool LIsMinChar(LInt ch);
LBool LIsAlpha(LInt ch);
LUint GenHashCode(const LInt8* ptr, LInt len);
}

using util::GenHashCode;
using util::LBool;
using util::LByte;
using util::LChar;
using util::LCharA;
using util::LFalse;
using util::LInt;
using util::LInt16;
using util::LInt2Str;
using util::LInt2StrWithLength;
using util::LInt32;
using util::LInt64;
using util::LInt8;
using util::LIntPtr;
using util::LIsAlpha;
using util::LIsBigChar;
using util::LIsDigit;
using util::LIsMinChar;
using util::LIsSpace;
using util::LMemcpy;
using util::LMemset;
using util::LReal;
using util::LReal32;
using util::LStr2Int;
using util::LStrcpy;
using util::LStrlen;
using util::LStrncmp;
using util::LTrue;
using util::LUint;
using util::LUint16;
using util::LUint32;
using util::LUint64;
using util::LUint8;
using util::LUintPtr;
using util::LVoid;
using util::LOffset;
using util::LSizeT;

#endif // PLATFORMLIB_H

// End of File
