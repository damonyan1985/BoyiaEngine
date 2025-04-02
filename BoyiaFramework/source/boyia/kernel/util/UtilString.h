/*
============================================================================
 Name        : UtilString_h
 Author      : yanbo
 Version     : 1.0
 Copyright   : All Copyright Reserved
 Description : support Original 本字符串封装包含
               浅拷贝字符串，如果m_owner为LFalse
               则表示该字符串为浅拷贝，浅拷贝字符串只能
               用作临时字符串进行比较查询使用，用来提高
               程序效率，不可作为打印和显示的正式字符串使用，
               浅拷贝字符串不会析构new所申请的内存
============================================================================
*/

#ifndef UtilString_h
#define UtilString_h

#include "BoyiaRef.h"
#include "PlatformLib.h"
#include "SalLog.h"
#include <wchar.h>

namespace util {
template <class T>
class LStringPolicy;

template <>
class LStringPolicy<LCharA> {
public:
    typedef LCharA* LChars;
    
    static inline LInt CountString(const LCharA* str)
    {
        return LStrlen((const LUint8*)str);
    }
};

template <>
class LStringPolicy<LUint8> {
public:
    typedef LUint8* LChars;
    
    static inline LInt CountString(const LUint8* str)
    {
        return LStrlen(str);
    }
};

template <>
class LStringPolicy<LUint16> {
public:
    typedef LUint16* LChars;
    
    static inline LInt CountString(const LUint16* str)
    {
        return -1;
    }
};

template <>
class LStringPolicy<wchar_t> {
public:
    typedef wchar_t* LChars;
    
    static inline LInt CountString(const wchar_t* str)
    {
        return wcslen(str);
    }
};

template <class T>
class LString {
public:
    typedef LStringPolicy<T> Policy;

public:
    LString();
    virtual ~LString();
    LString(const LString<T>& stringSrc, Bool owner = LTrue);
    LString(const T* lpsz, Bool owner = LTrue, LInt size = -1);
    LString(T ch, LInt nRepeat = 1);
    LString(const T* lpsz, LInt nLength, Bool tmp = LFalse);
    LVoid AllocBuffer(LInt nLen);
    LVoid Copy(const T* lpsz, Bool owner = LTrue, LInt size = -1);
    LVoid Move(LString<T>& src);

protected:
    LVoid StrAssignment(const T* lpsz);
    LVoid StrAssignment(const T* lpsz, LInt len);
    LVoid StrPlus(const T* lpsz, LInt len);
    LInt CountString(const T* lpsz) const;
    LVoid ResetBuffer();

public:
    const LString<T>& operator=(const LString<T>& stringSrc);
    const LString<T>& operator=(const T* lpsz);
    const LString<T>& operator=(const T ch);

    const LString<T>& operator+=(const LString<T>& stringSrc);
    const LString<T>& operator+=(const T* lpsz);
    const LString<T>& operator+=(const T ch);

    LBool operator==(const LString<T>& str) const;
    LBool operator==(const T* lpsz) const;
    LBool operator!=(const LString<T>& str) const;
    LBool operator>=(const LString<T>& str) const;
    LBool operator<=(const LString<T>& str) const;
    LBool operator>(const LString<T>& str) const;
    LBool operator<(const LString<T>& str) const;

    LString<T> Mid(LInt nPos, LInt nLength = -1) const;
    LString<T> Left(LInt nLength) const;
    LString<T> Right(LInt nLength) const;

    LString<T>& ToLower();
    LString<T>& ToUpper();

    operator const T*() const;
    T& operator[](LInt nIndex);
    const T& operator[](LInt nIndex) const;

    typename Policy::LChars LCharString() const;

    LInt Find(const LString<T>& str, LInt nPos = 0) const;
    LInt FindNoCase(const LString<T>& str, LInt nPos = 0) const;
    LInt ReverseFind(const LString<T>& str) const;
    LInt ReverseFindNoCase(const LString<T>& str) const;
    
    LInt QuickFind(const LString<T>& pattern, LInt nPos = 0) const;

    LBool EndWith(const LString<T>& sEnd) const;
    LBool EndWithNoCase(const LString<T>& sEnd);
    LBool StartWith(const LString<T>& sStart) const;
    LBool StartWithNoCase(const LString<T>& sStart) const;

    T CharAt(LInt nIndex) const;
    LVoid DeleteCharAt(LInt nIndex);

    LBool CompareNoCase(const LString<T>& str) const;
    LBool CompareCase(const LString<T>& str) const;

    void Replace(const LString<T>& strSrc, const LString<T>& strReplace, int nRep = -1);
    void ReplaceNoCase(const LString<T>& strSrc, const LString<T>& strReplace, int nRep = -1);

public:
    LInt GetLength() const;
    T* GetBuffer() const;
    LVoid ReleaseBuffer();
    LVoid ClearBuffer();

protected:
    // heap alloc size, m_capacity is not the size of the string
    LInt m_capacity;
    LInt m_size;
    T* m_buffer;
    Bool m_owner;
};

typedef LString<LUint8> StringA;
typedef LString<LUint16> StringW;
typedef LString<LCharA> CString;

typedef StringA String;

// For example
// LString<LUint16> str = _WS("hello") OR LString<LUint16> str = L"hello"
#define _WS(a) util::StringW((const util::LUint16*)L##a)

// For example
// LString<LUint8> str = _CS("hello")
//#define _CS(a) util::StringA((const util::LUint8*)a)
#define _CS(a) ((const util::LUint8*)a)
// For example
// LString<char> str = _DSTR("hello")
#define _DSTR(a) util::String(a)

#define GET_STR(str) ((const char*)str.GetBuffer())

template <class T>
LString<T>::LString()
    : m_buffer(kBoyiaNull)
    , m_capacity(0)
    , m_size(0)
    , m_owner(LTrue)
{
}

template <class T>
LString<T>::~LString()
{
    ResetBuffer();
}

template <class T>
LString<T>::LString(const T* lpsz, Bool owner, LInt size)
    : m_buffer(kBoyiaNull)
{
    LString<T>::Copy(lpsz, owner, size);
}

template <class T>
LString<T>::LString(const LString<T>& stringSrc, Bool owner)
{
    if (owner) {
        int nLen = stringSrc.GetLength();
        AllocBuffer(nLen);
        LMemcpy(m_buffer, stringSrc.GetBuffer(), nLen * sizeof(T));
        m_size = nLen;
    } else {
        m_buffer = stringSrc.m_buffer;
        m_capacity = stringSrc.m_capacity;
        m_size = stringSrc.m_size;
    }

    m_owner = owner;
}

template <class T>
LString<T>::LString(const T* lpsz, LInt nLen, Bool tmp)
{
    if (tmp) {
        m_buffer = (T*)lpsz;
        m_capacity = nLen + 1;
        m_owner = LFalse;
    } else {
        AllocBuffer(nLen);
        LMemcpy(m_buffer, lpsz, (nLen) * sizeof(T));
        m_owner = LTrue;
    }
    m_size = nLen;
}

template <class T>
LString<T>::LString(T ch, LInt nRepeat)
{
    AllocBuffer(nRepeat);
    if (ch != 0) {
        LMemset(m_buffer, ch, nRepeat * sizeof(T));
    }
    m_size = ch == 0 ? 0 : nRepeat;
    m_owner = LTrue;
}

template <class T>
LVoid LString<T>::Copy(const T* lpsz, Bool owner, LInt size)
{
    if (!lpsz) {
        ResetBuffer();
        return;
    }

    if (size == -1) {
        LInt nLen = CountString(lpsz);
        AllocBuffer(nLen);
        LMemcpy(m_buffer, lpsz, nLen * sizeof(T));
        m_size = nLen;
    } else {
        //KFORMATLOG("String not deep STR=%s and size=%d", (const char*)lpsz, size);
        m_size = size;
        m_capacity = m_size + 1;
        m_buffer = (T*)lpsz;
    }

    m_owner = owner;
}

template <class T>
LVoid LString<T>::Move(LString<T>& src)
{
    m_buffer = src.m_buffer;
    m_owner = src.m_owner;
    m_capacity = src.m_capacity;
    m_size = src.m_size;
    src.ReleaseBuffer();
}

template <class T>
LVoid LString<T>::ResetBuffer()
{
    if (m_buffer && m_owner) {
        //delete[] m_buffer;
        FREE_BUFFER(m_buffer);
    }

    m_buffer = kBoyiaNull;
    m_capacity = 0;
    m_size = 0;
}

template <class T>
const LString<T>& LString<T>::operator=(const LString<T>& stringSrc)
{
    if (stringSrc.GetBuffer() == m_buffer) {
        return *this;
    }

    T* src = stringSrc.GetBuffer();
    if (src) {
        StrAssignment(src, stringSrc.GetLength());
    } else {
        ResetBuffer();
    }

    return *this;
}

template <class T>
const LString<T>& LString<T>::operator=(const T* lpsz)
{
    if (lpsz) {
        StrAssignment(lpsz);
    } else {
        ResetBuffer();
    }

    return *this;
}

template <class T>
const LString<T>& LString<T>::operator=(const T ch)
{
    T str[2] = { ch, 0 };
    StrAssignment(str);
    return *this;
}

template <class T>
const LString<T>& LString<T>::operator+=(const LString<T>& stringSrc)
{
    StrPlus(stringSrc.GetBuffer(), stringSrc.GetLength());
    return *this;
}

template <class T>
const LString<T>& LString<T>::operator+=(const T* lpsz)
{
    StrPlus(lpsz, 0);
    return *this;
}

template <class T>
const LString<T>& LString<T>::operator+=(const T ch)
{
    T str[2] = { ch, 0 };
    StrPlus(str, 1);
    return *this;
}

template <class T>
T& LString<T>::operator[](LInt nIndex)
{
    return m_buffer[nIndex];
}

template <class T>
const T& LString<T>::operator[](LInt nIndex) const
{
    return m_buffer[nIndex];
}

template <class T>
T* LString<T>::GetBuffer() const
{
    return m_buffer;
}

template <class T>
LInt LString<T>::GetLength() const
{
    return m_size;
}

template <class T>
LVoid LString<T>::AllocBuffer(LInt nLen)
{
    m_capacity = nLen + 1;
    m_buffer = NEW_BUFFER(T, m_capacity);
    LMemset(m_buffer, 0, m_capacity * sizeof(T));
}

template <class T>
LVoid LString<T>::StrAssignment(const T* lpsz, LInt nLen)
{
    if (m_capacity <= nLen) {
        ResetBuffer();
        AllocBuffer(nLen);
    } else {
        LMemset(m_buffer, 0, m_capacity * sizeof(T));
    }

    LMemcpy(m_buffer, lpsz, nLen * sizeof(T));
    m_size = nLen;
}

template <class T>
LVoid LString<T>::StrAssignment(const T* lpsz)
{
    LInt nLen = CountString(lpsz);
    StrAssignment(lpsz, nLen);
}

template <class T>
LVoid LString<T>::StrPlus(const T* lpsz, LInt len)
{
    int plusSize = len > 0 ? len : CountString(lpsz);
    LInt nLen = GetLength() + plusSize;
    if (m_capacity <= nLen) {
        if (m_buffer) {
            T* pOldData = m_buffer;
            LInt nOldDataLen = m_size;
            AllocBuffer(nLen);
            LMemcpy(m_buffer, pOldData, nOldDataLen * sizeof(T));
            LMemcpy(m_buffer + nOldDataLen, lpsz, plusSize * sizeof(T));
            //delete[] pOldData;
            FREE_BUFFER(pOldData);
        } else {
            AllocBuffer(nLen);
            LMemcpy(m_buffer, lpsz, plusSize * sizeof(T));
        }
    } else {
        LMemcpy(m_buffer + GetLength(), lpsz, plusSize * sizeof(T));
    }

    m_size = nLen;
}

template <class T>
LBool LString<T>::operator==(const T* lpsz) const
{
    T* src = GetBuffer();
    while (*src && *lpsz) {
        if (*src++ != *lpsz++) {
            return LFalse;
        }
    }

    return *src || *lpsz ? LFalse : LTrue;
}

template <class T>
LBool LString<T>::operator==(const LString<T>& str1) const
{
    return CompareCase(str1);
}

template <class T>
LBool LString<T>::CompareCase(const LString<T>& str) const
{
    LInt nLen = GetLength();
    LInt nComparedLen = str.GetLength();
    if (nLen != nComparedLen) {
        return LFalse;
    }

    LInt ipos = 0;
    for (; ipos < nLen; ++ipos) {
        if (m_buffer[ipos] != str[ipos]) {
            return LFalse;
        }
    }

    return LTrue;
}

template <class T>
LBool LString<T>::CompareNoCase(const LString<T>& str) const
{
    LInt nLen = GetLength();
    LInt nComparedLen = str.GetLength();
    if (nLen != nComparedLen) {
        return LFalse;
    }

    LString<T> str1 = *this;
    LString<T> str2 = str;
    str1.ToLower();
    str2.ToLower();
    return str1.CompareCase(str2);
}

template <class T>
LBool LString<T>::operator!=(const LString<T>& str) const
{
    return !(*this == str);
}

template <class T>
LBool LString<T>::operator<(const LString<T>& str) const
{
    LInt nLen = GetLength();
    LInt nComparedLen = str.GetLength();
    LInt nLowerLen = 0;
    LBool bLargeORequal = LFalse;
    if (nLen >= nComparedLen) {
        nLowerLen = nLen;
        bLargeORequal = LTrue;
    } else {
        nLowerLen = nComparedLen;
    }

    LInt ipos = 0;
    for (; ipos < nLowerLen; ++ipos) {
        if (m_buffer[ipos] < str[ipos]) {
            return LTrue;
        }
        if (m_buffer[ipos] > str[ipos]) {
            return LFalse;
        }
    }

    return !bLargeORequal;
}

template <class T>
LBool LString<T>::operator<=(const LString<T>& str) const
{
    return *this < str || *this == str;
}

template <class T>
LBool LString<T>::operator>(const LString<T>& str) const
{
    return !(*this <= str);
}

template <class T>
LBool LString<T>::operator>=(const LString<T>& str) const
{
    //return *this > str1 || *this == str1;
    return !(*this < str);
}

template <class T>
LInt LString<T>::CountString(const T* lpsz) const
{
    // 优先使用Policy中实现的方法进行实现
    LInt nLen = LString<T>::Policy::CountString(lpsz);
    // 返回-1表示方法未实现
    if (nLen != -1) {
        return nLen;
    }
    
    nLen = 0;
    while (*(lpsz + nLen)) {
        ++nLen;
    }

    return nLen;
}

template <class T>
LString<T>::operator const T*() const
{
    return (const T*)m_buffer;
}

template <class T>
typename LString<T>::Policy::LChars LString<T>::LCharString() const
{
    return (const typename LString<T>::Policy::LChars)GetBuffer();
}

template <class T>
LString<T> LString<T>::Mid(LInt nPos, LInt nLength) const
{
    if (nLength < 0) {
        nLength = GetLength() - nPos;
    }

    return LString<T>(m_buffer + nPos, nLength, LTrue);
}

template <class T>
LString<T> LString<T>::Left(LInt nLength) const
{
    return Mid(0, nLength);
}

template <class T>
LString<T> LString<T>::Right(LInt nLength) const
{
    return Mid(GetLength() - nLength, nLength);
}

template <class T>
LString<T>& LString<T>::ToLower()
{
    LInt nLen = GetLength();
    for (LInt i = 0; i < nLen; ++i) {
        if (m_buffer[i] >= T('A') && m_buffer[i] <= T('Z')) {
            m_buffer[i] += T('a') - T('A');
        }
    }

    return *this;
}

template <class T>
LString<T>& LString<T>::ToUpper()
{
    LInt nLen = GetLength();
    for (LInt i = 0; i < nLen; ++i) {
        if (m_buffer[i] >= T('a') && m_buffer[i] <= T('z')) {
            m_buffer[i] += T('A') - T('a');
        }
    }

    return *this;
}

template <class T>
LInt LString<T>::Find(const LString<T>& str, LInt nPos) const
{
    LInt nLen = str.GetLength();
    if (GetLength() >= nLen && nLen) {
        T* str_beg = m_buffer + nPos;
        T* str_end = m_buffer + GetLength() - nLen;
        for (; str_beg <= str_end; ++str_beg) {
            LString<T> strCmp(str_beg, LFalse, nLen);
            if (strCmp == str) {
                return LInt(str_beg - m_buffer);
            }
        }
    }

    return -1;
}

template <class T>
LInt LString<T>::FindNoCase(const LString<T>& str, LInt nPos) const
{
    LInt nLen = str.GetLength();
    if (GetLength() >= nLen && nLen) {
        T* str_beg = m_buffer + nPos;
        // 当begin = end的时候，正好等于过滤字符串的长度
        T* str_end = m_buffer + GetLength() - nLen;
        for (; str_beg <= str_end; ++str_beg) {
            LString<T> strCmp(str_beg, LFalse, nLen);
            if (strCmp.CompareNoCase(str)) {
                return LInt(str_beg - m_buffer);
            }
        }
    }

    return -1;
}

template <class T>
LInt LString<T>::ReverseFind(const LString<T>& str) const
{
    LInt nLen = str.GetLength();
    if (m_capacity > nLen && nLen) {

        T* str_beg = m_buffer + m_capacity - nLen;
        T* str_end = m_buffer;
        for (; str_beg >= str_end; --str_beg) {
            LString<T> strCmp(str_beg, LFalse, nLen);
            if (strCmp == str) {
                return LInt(str_beg - m_buffer);
            }
        }
    }

    return -1;
}

template <class T>
LInt LString<T>::ReverseFindNoCase(const LString<T>& str) const
{
    LInt nLen = str.GetLength();
    if (m_capacity > nLen && nLen) {

        T* str_beg = m_buffer + m_capacity - nLen;
        T* str_end = m_buffer;
        for (; str_beg >= str_end; --str_beg) {
            LString<T> strCmp(str_beg, LFalse, nLen);
            if (strCmp.CompareNoCase(str)) {
                return LInt(str_beg - m_buffer);
            }
        }
    }

    return -1;
}

template <class T>
LInt LString<T>::QuickFind(const LString<T>& pattern, LInt nPos) const
{
    // source串匹配的首字母对应的位置
    LInt nLen = pattern.GetLength();
    LInt srcLen = GetLength() - nPos;
    if (srcLen >= 0 && nLen) {
        T* srcBuffer = GetBuffer() + nPos;
        T* patternBuffer = pattern.GetBuffer();
        // next数组表示pattern匹配失败后需要移动的距离
        // 暂定next数组长度不超过32
        LInt next[32];
        {
            LInt i = 1;
            next[1] = 0;
            LInt j = 0;
            while (i < nLen) {
                if (j == 0 || patternBuffer[i-1] == patternBuffer[j-1]) {
                    i++;
                    j++;
                    next[i] = j;
                } else {
                    j = next[j];
                }
            }
        }
        
        // 匹配失败后，不需要调整srcBuffer的索引i
        LInt i = 1;
        LInt j = 1;
        while (i <= srcLen && j <= nLen) {
            if (j == 0 || srcBuffer[i-1] == patternBuffer[j-1]) {
                i++;
                j++;
            } else {
                j = next[j];
            }
        }
        
        // 如果条件为真，说明匹配成功
        if (j > nLen) {
            LInt result = nPos + i - nLen - 1;
            String str = Mid(nPos, result);
            return result;
        }
    }
    
    return -1;
}

template <class T>
LBool LString<T>::EndWith(const LString<T>& sEnd) const
{
    LInt mLen = GetLength();
    LInt sLen = sEnd.GetLength();

    if (mLen < sLen) {
        return LFalse;
    }

    LBool isEqual = LTrue;
    for (LInt i = 0; i < sLen; ++i) {
        if (m_buffer[mLen - i] != sEnd[sLen - i]) {
            isEqual = LFalse;
            break;
        }
    }

    return isEqual;
}

template <class T>
LBool LString<T>::EndWithNoCase(const LString<T>& sEnd)
{
    LString<T> str1 = *this;
    LString<T> str2 = sEnd;
    str1.ToLower();
    str2.ToLower();

    return str1.EndWith(str2);
}

template <class T>
LBool LString<T>::StartWith(const LString<T>& sStart) const
{
    LInt mLen = GetLength();
    LInt sLen = sStart.GetLength();

    if (mLen < sLen) {
        return LFalse;
    }

    LBool isEqual = LTrue;
    for (LInt i = 0; i < sLen; ++i) {
        if (m_buffer[i] != sStart[i]) {
            isEqual = LFalse;
            break;
        }
    }

    return isEqual;
}

template <class T>
LBool LString<T>::StartWithNoCase(const LString<T>& sStart) const
{
    LString<T> str1 = *this;
    LString<T> str2 = sStart;
    str1.ToLower();
    str2.ToLower();

    return str1.StartWith(str2);
}

template <class T>
T LString<T>::CharAt(LInt nIndex) const
{
    if (nIndex < m_capacity) {
        return *(m_buffer + nIndex);
    }

    return (T)0;
}

template <class T>
LVoid LString<T>::DeleteCharAt(LInt nIndex)
{
    LInt len = GetLength();
    if (nIndex < 0 || nIndex >= len) {
        return;
    }

    for (LInt i = nIndex; i < len - 1; ++i) {
        *(m_buffer + i) = *(m_buffer + i + 1);
    }

    *(m_buffer + len - 1) = (T)0;
    m_size = len - 1;
}

template <class T>
void LString<T>::Replace(const LString<T>& strSrc, const LString<T>& strReplace, int nRep)
{
    LString<T> strRet;
    LString<T> strRemain = *this;
    LInt nIndex;
    LInt size = 0;
    LInt delta = strReplace.GetLength() - strSrc.GetLength();
    for (LInt i = 0; i != nRep && strRemain.GetLength() && (nIndex = strRemain.Find(strSrc)) >= 0; ++i) {
        strRet += strRemain.Left(nIndex);
        strRet += strReplace;
        strRemain = strRemain.Mid(nIndex + strSrc.GetLength());
        size += delta;
    }

    strRet += strRemain;
    m_size += size;
    *this = strRet;
}

template <class T>
void LString<T>::ReplaceNoCase(const LString<T>& strSrc, const LString<T>& strReplace, int nRep)
{
    LString<T> strRet;
    LString<T> strRemain = *this;
    LInt nIndex = 0;
    LInt size = 0;
    LInt delta = strReplace.GetLength() - strSrc.GetLength();
    for (LInt i = 0; i != nRep && strRemain.GetLength() && (nIndex = strRemain.FindNoCase(strSrc)) >= 0; ++i) {
        strRet += strRemain.Left(nIndex);
        strRet += strReplace;
        strRemain = strRemain.Mid(nIndex + strSrc.GetLength());
        size += delta;
    }

    strRet += strRemain;
    m_size += size;
    *this = strRet;
}

template <class T>
LVoid LString<T>::ReleaseBuffer()
{
    m_capacity = 0;
    m_size = 0;
    m_buffer = kBoyiaNull;
    m_owner = LTrue;
}

template <class T>
LVoid LString<T>::ClearBuffer()
{
    m_size = 0;
    LMemset(m_buffer, 0, m_size * sizeof(T));
}

template <class T>
LString<T> GetNewString(const T* str1, const T* str2)
{
    LString<T> str = str1;
    str += str2;
    return str;
}

template <class T>
LString<T> operator+(const LString<T>& string1, const LString<T>& string2)
{
    return GetNewString(string1.GetBuffer(), string2.GetBuffer());
}

template <class T>
LString<T> operator+(const LString<T>& string1, const T* lpsz)
{
    return GetNewString(string1.GetBuffer(), lpsz);
}

template <class T>
LString<T> operator+(const T* lpsz, const LString<T>& string1)
{
    return GetNewString(lpsz, string1.GetBuffer());
}

template <class T>
LString<T> operator+(const LString<T>& string1, const T ch)
{
    T str[2] = { ch, 0 };
    return GetNewString(string1.GetBuffer(), str);
}

template <class T>
LString<T> operator+(const T ch, const LString<T>& string1)
{
    T str[2] = { ch, 0 };
    return GetNewString(str, string1.GetBuffer());
}
}

using util::CString;
using util::String;
using util::StringA;
using util::StringW;

#endif // LSTRING_H

// End of File
