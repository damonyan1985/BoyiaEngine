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

// Setting byte alignment with one byte
// MiniMemory allocator data must byte alignment
//#pragma pack(4)

namespace util
{
// feature begin
#define ENABLE_ANDROID_PORTING 1
//#define ENABLE_BROWSER_KERNEL 1
// feature end
// 
#define LCONST const

#ifndef NULL
#define NULL 0
#endif
//
#define ENABLE(FEATURE) (defined ENABLE_##FEATURE && ENABLE_##FEATURE)
#define UNABLE(FEATURE) (defined ENABLE_##FEATURE && !ENABLE_##FEATURE)

// 
#define LCHARSW(CHARS) L##CHARS

//
#define LCHARSA(CHARS) CHARS

//
#define LCHARS LCHARSA

#define LSTATIC_CAST(typ, object) static_cast<tpy>(object)

#define LMax(a, b) (a>=b)?a:b

#define LMin(a, b) (a>=b)?b:a


//
//using LInt8 = char;
typedef char LInt8;
typedef short int LInt16;
typedef int LInt32;
typedef long long LInt64;

// 
typedef unsigned char LUint8;
typedef unsigned short int LUint16;
typedef unsigned int LUint32;
typedef unsigned long long LUint64;
typedef unsigned long LDWORD;
typedef unsigned short LWORD;

#if defined(__LP64__)
typedef long LInt;
typedef long LIntPtr;
typedef unsigned long LUintPtr;
typedef unsigned long LUint;
#else
typedef int LInt;
typedef int LIntPtr;
typedef unsigned int LUint;
typedef unsigned int LUintPtr;
#endif
// 
typedef void LVoid;

// 
typedef float LReal32;
typedef double LReal64;

typedef LReal32 LReal;
// 
typedef LInt LBool;

typedef char LCharA;
//typedef unsigned char LCharA;

//#ifndef __WINS__
//typedef wchar_t LCharW;
//#else
//typedef LUint16 LCharW;
//#endif

typedef LCharA LChar;

typedef enum Bool
{
	LFalse = 0,
	LTrue
} Bool;
//const LBool LFalse = 0;
//const LBool LTrue  = 1;
 
// 
typedef LUint8 LByte;
typedef LUint16 UChar;

typedef LDWORD LCOLORREF;

typedef LByte LBYTE;

#define _WL(a) (const util::LUint16)L##a

// LString<LUint8> str = _CL('b')
#define _CL(a) (const util::LUint8)a

#define LLOBYTE(w)           ((LBYTE)(((LDWORD)(w)) & 0xff))

#define LRGB(r,g,b) ((LCOLORREF)(((LBYTE)(r)|((LDWORD)((LBYTE)(g))<<8))|(((LDWORD)(BYTE)(b))<<16)))

#define LGetAValue(rgb)      (LLOBYTE((rgb) >> 24))
#define LGetRValue(rgb)      (LLOBYTE((rgb) >> 16))
#define LGetGValue(rgb)      (LLOBYTE(((LWORD)(rgb)) >> 8))
#define LGetBValue(rgb)      (LLOBYTE(rgb))

#define UALLOC(c)                       new c
#define UALLOC_ARG1(c, a)               new c(a)
#define UALLOC_ARG2(c, a1, a2)          new c(a1, a2)
#define UALLOC_ARG3(c, a1, a2, a3)      new c(a1, a2, a3)
#define UALLOC_ARG4(c, a1, a2, a3, a4)  new c(a1, a2, a3, a4)
#define UALLOCN(c, n)                   new c[n]
#define UFREE(p)                        delete p

// 
LInt LStrlen(const LUint8* aStr);

LInt LStrcmp(const LCharA* src, const LCharA* cmpStr);

LInt LStrncmp(const LCharA* s1, const LCharA* s2, LInt n);

LInt LStrincmp(const LCharA* src, const LCharA* cmpStr, LInt maxlen);

LInt8* LStrcpy(LInt8* aDest, const LInt8* aSrc);

// 
LVoid* LMemcpy(LVoid* aDest, const LVoid* aSrc, LInt aLength);
LVoid* LMemmove(LVoid* dest, const LVoid* source, LInt count);
LInt LMemcmp(const LVoid *buffer1, const LVoid *buffer2, LInt count);
LVoid* LMemset(LVoid* aDest, LInt aValue, LInt aLength);

// 
LVoid LStrA2U(const LUint8* aStrA, LUint16* aStrW);
LVoid LStrU2A(LUint8* aStrA, const LUint16* aStrW);

// 
LUint8 *LInt2Str(LInt value, LUint8 *str, LInt radix);
LUint8* LInt2StrWithLength(LInt value, LUint8* str, LInt radix, LInt* lenPtr);
LInt LStr2Int(const LUint8* str, LInt radix);

LChar* LStrchr(const LChar *s, LChar c);
    
// 
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

using util::LBool;
using util::LFalse;
using util::LTrue;
using util::LInt;
using util::LInt16;
using util::LInt32;
using util::LInt64;
using util::LInt8;
using util::LUint8;
using util::LUint;
using util::LMemset;
using util::LMemcpy;
using util::LUint16;
using util::LUint32;
using util::LByte;
using util::LChar;
using util::LCharA;
using util::LReal;
using util::LVoid;
using util::LStr2Int;
using util::LIsDigit;
using util::LIsBigChar;
using util::LIsMinChar;
using util::LIsAlpha;
using util::LStrcpy;
using util::LInt2Str;
using util::LStrncmp;
using util::LIsSpace;
using util::LStrlen;
using util::GenHashCode;
using util::LInt2StrWithLength;
using util::LIntPtr;
using util::LUintPtr;

#endif // PLATFORMLIB_H

// End of File
