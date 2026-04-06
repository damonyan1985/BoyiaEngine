/*
 ============================================================================
 Name        : PlatformLib.cpp
 Author      : yanbo
 Version     :
 Copyright   : Your copyright notice
 Description : PlatformLib.cpp - source file
 ============================================================================
 */

// INCLUDE FILES
// Class include
#include "PlatformLib.h"
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

#define USE_SYSTEM 1

#if USE_SYSTEM
#include <string.h>
#endif

// ================= MEMBER FUNCTIONS =======================
namespace util {
const double PI = 3.1415926535897931;

static LUint8* LItoa(LInt value, LUint8* str, LInt radix, LInt* len)
{
    static LUint8 szMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
    LInt nCount = -1, nIndex = 0;
    LUint8 *pStr = str, nTemp = 0;
    if (radix >= 2 && radix <= 36) {
        if (value < 0 && radix == 10) {
            *pStr++ = '-';
            value = -1 * value;
        }

        LUint nValue = (LUint)value;
        do {
            pStr[++nCount] = szMap[nValue % radix];
            nValue /= radix;
        } while (nValue > 0);

        nIndex = (nCount + 1) / 2;
        while (nIndex-- > 0) {
            nTemp = pStr[nIndex];
            pStr[nIndex] = pStr[nCount - nIndex];
            pStr[nCount - nIndex] = nTemp;
        }
    }
    pStr[nCount + 1] = '\0';
    if (len) {
        *len = nCount + 1;
    }
    return str;
}

LInt LStrlen(const LUint8* aStr)
{
#if USE_SYSTEM
    return (LInt)strlen((const char*)aStr);
#else
    LInt nLen = 0;
    while (*(aStr + nLen) != '\0') {
        ++nLen;
    }

    return nLen;
#endif
}

LInt LStrcmp(const LCharA* src, const LCharA* cmpStr)
{
    while (*src && *cmpStr) {
        if (*src > *cmpStr) {
            return 1;
        }
        if (*src < *cmpStr) {
            return -1;
        }

        ++src;
        ++cmpStr;
    }

    return *src ? 1 : (*cmpStr ? -1 : 0);
}

LInt LStrncmp(const LCharA* s1, const LCharA* s2, LInt n)
{
    LCharA* ss1 = (LCharA*)s1;
    LCharA* ss2 = (LCharA*)s2;
    if (!n) {
        return 0;
    }

    while (--n && *ss1 && *ss1 == *ss2) {
        ++ss1, ++ss2;
    }
    return (*(LUint8*)ss1 - *(LUint8*)ss2);
}

// case-insensitive
LInt LStrincmp(const LCharA* src, const LCharA* cmpStr, LInt maxlen)
{
    LCharA ch1, ch2;
    for (int i = 0; i < maxlen; ++i) {
        ch1 = src[i];

        if (ch1 >= 'a' && ch1 <= 'z') {
            ch1 -= 'a' - 'A';
        }

        ch2 = cmpStr[i];
        if (ch2 >= 'a' && ch2 <= 'z') {
            ch2 -= 'a' - 'A';
        }

        if (ch1 != ch2) {
            if (ch1 > ch2)
                return 1;
            else
                return -1;
        } else {
            if (ch1 == 0)
                return 0;
            else
                continue;
        }
    }

    return 0;
}

LInt8* LStrcpy(LInt8* aDest, const LInt8* aSrc)
{
    while (*aSrc != '\0') {
        *(aDest++) = *(aSrc++);
    }

    *aDest = '\0';
    return aDest;
}

LVoid* LMemset(LVoid* dest, LInt value, LInt len)
{
#if USE_SYSTEM
    return memset(dest, value, len);
#else
    LUint8* p = (LUint8*)dest;
    while (len--) {
        *(p + len) = value;
    }
    
    return dest;
#endif
}

LVoid* LMemcpy(LVoid* dest, const LVoid* src, LInt len)
{
#if USE_SYSTEM
    return memcpy(dest, src, len);
#else
    LUint8* d = (LUint8*)dest;
    LUint8* s = (LUint8*)src;
    while (len--) {
        *(d + len) = *(s + len);
    }
    return dest;
#endif
}

LVoid* LMemmove(LVoid* dest, const LVoid* source, LInt count)
{
    LUint8* d = (LUint8*)dest;
    LUint8* s = (LUint8*)source;
    if (d <= s || d >= (s + count)) {
        //Non-Overlapping Buffers
        //copy from lower addresses to higher addresses
        while (count--)
            *d++ = *s++;
    } else {
        //Overlapping Buffers
        //copy from higher addresses to lower addresses
        d += count - 1;
        s += count - 1;
        while (count--)
            *d-- = *s--;
    }

    return dest;
}

LVoid LStrA2U(const LUint8* aStrA, LUint16* aStrW)
{
    LUint8* strA = (LUint8*)aStrA;
    LUint16* strW = aStrW;
    while (*strA != '\0') {
        *(strW++) = *(strA++);
    }

    *strW = 0;
}

LVoid LStrU2A(LUint8* aStrA, const LUint16* aStrW)
{
    LUint8* strA = aStrA;
    LUint16* strW = (LUint16*)aStrW;
    while (*strW != 0) {
        *strA++ = (LUint8)((*strW++) & 0xFF);
    }
}

LUint8* LInt2Str(LInt value, LUint8* str, LInt radix)
{
    return LItoa(value, str, radix, kBoyiaNull);
}

LUint8* LInt2StrWithLength(LInt value, LUint8* str, LInt radix, LInt* lenPtr)
{
    return LItoa(value, str, radix, lenPtr);
}

LBool LIsDigit(LInt ch)
{
    if (ch <= '9' && ch >= '0') {
        return LTrue;
    }

    return LFalse;
}

// only to judge the hex
LBool LIsBigChar(LInt ch)
{
    if (ch <= 'F' && ch >= 'A') {
        return LTrue;
    }

    return LFalse;
}

LBool LIsMinChar(LInt ch)
{
    if (ch <= 'f' && ch >= 'a') {
        return LTrue;
    }

    return LFalse;
}

LInt LStr2Int(const LUint8* str, LInt radix)
{
    // LInt total = 0;
    // LInt sign = 1;

    // if (*str == '-') {
    //     sign = -1;
    //     ++str;
    // } else if (*str == '+') {
    //     ++str;
    // }

    // while (*str) {
    //     LInt ch = 0;
    //     if (LIsDigit(*str)) {
    //         ch = *str - '0';
    //     } else if (LIsBigChar(*str)) {
    //         ch = *str - 'A' + 10;
    //     } else if (LIsMinChar(*str)) {
    //         ch = *str - 'a' + 10;
    //     }

    //     total = total * radix + ch;
    //     ++str;
    // }

    // return total * sign;
    return LStr2IntWithLength(str, -1, radix);
}

LInt LStr2IntWithLength(const LUint8* str, LInt length, LInt radix) {
    LInt total = 0;
    LBool withLength = length != -1;
    if (withLength && length < 1) {
        return total;
    }

    LInt sign = 1;
    const LUint8* start = str;
    if (*str == '-') {
        sign = -1;
        ++str;
    } else if (*str == '+') {
        ++str;
    }

    while (*str) {
        LInt ch = 0;
        if (LIsDigit(*str)) {
            ch = *str - '0';
        } else if (LIsBigChar(*str)) {
            ch = *str - 'A' + 10;
        } else if (LIsMinChar(*str)) {
            ch = *str - 'a' + 10;
        }

        total = total * radix + ch;
        ++str;
        if (withLength && str - start >= length) {
            break;
        }
    }

    return total * sign;
}

LBool LIsSpace(LInt ch)
{
    //	if(ch == ' ' || ch == '\t'
    //			|| ch == '\r' || ch == '\n'
    //			|| ch == '\v' || ch == '\f')
    //	{
    //	    return LTrue;
    //	}
    //
    //	return LFalse;
    return (unsigned int)(ch - 9) < 5 || ch == ' ';
}

const LChar* LSkipSpace(const LChar* str)
{
    while (*str && LIsSpace((LInt)(*str))) {
        ++str;
    }

    return str;
}

LChar* LSkipString(LChar* src, const LChar* skip)
{
    int i = 0;

    while (skip[i]) {
        if (!src[i]) {
            i = 0;
            break;
        }

        if (src[i] != skip[i]) {
            i = 0;
            break;
        }

        ++i;
    }

    src = src + i;
    return src;
}

LChar* LStrchr(const LChar* s, LChar c)
{
    LChar* ptr = (LChar*)s;
    while (*ptr) {
        if ((*ptr) == c) {
            return ptr;
        }
    }

    return kBoyiaNull;
}

LInt LMemcmp(const LVoid* buffer1, const LVoid* buffer2, LInt count)
{
    if (!count)
        return (0);

    while (--count && *(LInt8*)buffer1 == *(LInt8*)buffer2) {
        buffer1 = (LInt8*)buffer1 + 1;
        buffer2 = (LInt8*)buffer2 + 1;
    }

    return (*((LUint8*)buffer1) - *((LUint8*)buffer2));
}

LInt LCeil(LReal64 f)
{
    if (f > 0.0f) {
        return (LInt)f + 1;
    } else {
        return (LInt)f;
    }
}

LInt LFloor(LReal64 f)
{
    if (f > 0.0f) {
        return (LInt)f;
    } else {
        return (LInt)f - 1;
    }
}

LReal64 LRound(LReal64 num)
{
    LReal64 integer = LCeil(num);
    if (num > 0)
        return integer - num > 0.5 ? integer - 1.0 : integer;
    return integer - num >= 0.5 ? integer - 1.0 : integer;
}

LReal toRadians(LReal angle)
{
    return (angle * PI) / 180;
}

LReal LFabs(LReal f)
{
    return (f > 0) ? f : (-f);
}

LBool LIsAlpha(LInt ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return LTrue;
    }

    if (ch >= 'A' && ch <= 'Z') {
        return LTrue;
    }

    return LFalse;
}

// BKDRHash hash算法
LUint GenHashCode1(const LInt8* ptr, LInt len)
{
    LUint seed = 131313; //也可以乘以31、131、1313、13131、131313..
    LUint hash = 0;
    LInt pos = 0;
    while (pos < len) {
        hash = hash * seed + (LUint)(*(ptr + pos++));
    }
    return hash % 0x7FFFFFFF; //MAX代表hash表长度
}

// This hash produces the fewest collisions of any function that we've seen
// so far, and works well on both numbers and strings.
LUint GenHashCode(const LInt8* key, LInt len)
{
    const LInt8* end = key + len;
    LUint hash = 0;
    for (; key < end; ++key) {
        hash *= 16777619;
        hash ^= (LUint)(*key);
    }
    return hash;
}

LReal64 Str2Real64(const LInt8* s, LInt len)
{
    if (!s || len == 0)
        return 0.0;

    LInt i = 0;

    // 1. 跳过前导空白
    while (i < len && isspace((unsigned char)s[i]))
        i++;

    if (i == len)
        return 0.0;  // 全空白

    // 2. 处理符号
    LReal64 sign = 1.0;
    if (s[i] == '+') {
        i++;
    }
    else if (s[i] == '-') {
        sign = -1.0;
        i++;
    }

    // 3. 解析整数部分
    LReal64 integer_part = 0.0;
    LBool has_integer = LFalse;
    while (i < len && isdigit((unsigned char)s[i])) {
        integer_part = integer_part * 10.0 + (s[i] - '0');
        has_integer = LTrue;
        i++;
    }

    // 4. 解析小数部分
    LReal64 fraction_part = 0.0;
    LInt frac_digits = 0;
    if (i < len && s[i] == '.') {
        i++;
        while (i < len && isdigit((unsigned char)s[i])) {
            fraction_part = fraction_part * 10.0 + (s[i] - '0');
            frac_digits++;
            i++;
        }
    }

    // 如果既没有整数部分也没有小数部分，非法
    if (!has_integer && frac_digits == 0)
        return 0.0;

    // 合并整数与小数部分
    LReal64 value = integer_part + fraction_part / pow(10.0, frac_digits);

    // 5. 解析指数部分
    if (i < len && (s[i] == 'e' || s[i] == 'E')) {
        i++;
        if (i == len) return sign * value;  // 只有 'e' 没有指数，忽略

        LReal64 exp_sign = 1.0;
        if (s[i] == '+') {
            i++;
        }
        else if (s[i] == '-') {
            exp_sign = -1.0;
            i++;
        }

        LInt exponent = 0;
        LBool has_exp_digit = LFalse;
        while (i < len && isdigit((unsigned char)s[i])) {
            exponent = exponent * 10 + (s[i] - '0');
            has_exp_digit = LTrue;
            i++;
        }
        if (!has_exp_digit) return sign * value;  // 指数无效，忽略

        // 应用指数（注意防止过大导致溢出）
        LReal64 exp_val = pow(10.0, exp_sign * exponent);
        value *= exp_val;
    }

    // 可选：跳过尾随空白（通常不关心）
    // while (i < len && isspace(s[i])) i++;
    // 如果 i != len，说明字符串末尾有非数字字符，根据需求可视为非法
    // 此处为了简单，直接返回已解析的值

    return sign * value;
}
}

// End of File
