/*
============================================================================
 Name        : UtilString_h
 Author      : yanbo
 Version     : 1.0
 Copyright   : All Copyright Reserved
 Description : support Original 本字符串封装包含
               浅拷贝字符串，如果m_isDeep为LFalse
               则表示该字符串为浅拷贝，浅拷贝字符串只能
               用作临时字符串进行比较查询使用，用来提高
               程序效率，不可作为打印和显示的正式字符串使用，
               浅拷贝字符串不会析构new所申请的内存
============================================================================
*/

#ifndef UtilString_h
#define UtilString_h

#include "PlatformLib.h"
#include "KRef.h"
#include "SalLog.h"

namespace util
{
	template <class T>
    class LStringPolicy;

	template <> 
	class LStringPolicy<LCharA>
	{
	public:
		typedef LUint8* LChars;
	};
	
	template <> 
	class LStringPolicy<LUint8>
	{
	public:
		typedef LUint8* LChars;
	};
	
	template <> 
	class LStringPolicy<LUint16>
	{
	public:
		typedef LUint16* LChars;
	};

	template <class T>
	class LString : public KRef
	{
	public:
		typedef LStringPolicy<T> Policy;

	public:
		LString();
		virtual ~LString();
		LString(const LString<T>& stringSrc, Bool isDeep = LTrue);
		LString(const T* lpsz, Bool isDeep = LTrue, LInt size = -1);
		LString(T ch, LInt nRepeat = 1);
		LString(const T *lpsz, LInt nLength);
		LVoid AllocBuffer(LInt nLen);
		LVoid Copy(const T* lpsz, Bool isDeep = LTrue, LInt size = -1);
	
	protected:
		LVoid StrAssignment(const T* lpsz);
		LVoid StrAssignment(const T* lpsz, LInt len);
		LVoid StrPlus(const T* lpsz);
		LInt  CountString(const T* lpsz) const;
		LVoid ResetBuffer();

	public:
		
		const LString<T>& operator = (const LString<T>& stringSrc);
		const LString<T>& operator = (const T* lpsz);
		const LString<T>& operator = (const T ch);

		const LString<T>& operator += (const LString<T>& stringSrc);
		const LString<T>& operator += (const T* lpsz);
		const LString<T>& operator += (const T ch);

		LBool operator == (const LString<T>& str) const;
		LBool operator == (const T* lpsz) const;
		LBool operator != (const LString<T>& str) const;
		LBool operator >= (const LString<T>& str) const;
		LBool operator <= (const LString<T>& str) const;
		LBool operator >  (const LString<T>& str) const;
		LBool operator <  (const LString<T>& str) const;
		
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
		
		LBool EndWith(const LString<T>& sEnd);
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

	protected:
		// heap alloc size, m_pchDataLen is not the size of the string
		LInt      m_pchDataLen;
		LInt      m_size;
		T*        m_pchData;
		Bool      m_isDeep;
	};

	typedef LString<LUint8> StringA;
	typedef LString<LUint16>  StringW;
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

    #define GET_STR(str) ((const char*)(str).GetBuffer())

	template<class T>
	LString<T>::LString()
	    : m_pchData(NULL)
	    , m_pchDataLen(0)
	    , m_size(0)
	    , m_isDeep(LTrue)
	{
	}

	template<class T>
	LString<T>::~LString()
	{
		ResetBuffer();
	}

	template<class T>
	LString<T>::LString(const T *lpsz, Bool isDeep, LInt size)
	    : m_pchData(NULL)
	{
		LString<T>::Copy(lpsz, isDeep, size);
	}

	template<class T>
	LString<T>::LString(const LString<T>& stringSrc, Bool isDeep)
	{
		if (isDeep == LTrue)
		{
			int nLen = stringSrc.GetLength();
			AllocBuffer(nLen);
			LMemcpy(m_pchData, stringSrc.GetBuffer(), nLen*sizeof(T));
			m_size = nLen;
		}
		else
		{
			m_pchData = stringSrc.GetBuffer();
			m_pchDataLen = stringSrc.GetLength();
			m_size = stringSrc.GetLength();
		}

		m_isDeep = isDeep;
	}

	template<class T>
	LString<T>::LString(const T *lpsz, LInt nLength)
	{
		int nLen = nLength;
		AllocBuffer(nLen);
		LMemcpy(m_pchData, lpsz, (nLen)*sizeof(T));
		m_size = nLen;
		m_isDeep = LTrue;
	}

	template<class T>
	LString<T>::LString(T ch, LInt nRepeat)
	{
		AllocBuffer(nRepeat);
		LMemset(m_pchData, ch, nRepeat*sizeof(T));
		m_size = nRepeat;
		m_isDeep = LTrue;
	}

	template<class T>
	LVoid LString<T>::Copy(const T* lpsz, Bool isDeep, LInt size)
	{
		if (lpsz == NULL)
		{
			ResetBuffer();
		}
		else
		{
			if (size == -1)
			{
				LInt nLen = CountString(lpsz);
				AllocBuffer(nLen);
				LMemcpy(m_pchData, lpsz, nLen * sizeof(T));
				m_size = nLen;
			}
			else
			{
				//KFORMATLOG("String not deep STR=%s and size=%d", (const char*)lpsz, size);
				m_size = size;
				m_pchDataLen = m_size + 1;
				m_pchData = (T*)lpsz;
			}
		}

		m_isDeep = isDeep;
	}

	template<class T>
	LVoid LString<T>::ResetBuffer()
	{
		if (m_pchData != NULL && m_isDeep)
		{
			delete []m_pchData;
		}

		m_pchData = NULL;
		m_pchDataLen = 0;
		m_size = 0;
	}

	template<class T>
	const LString<T>& LString<T>::operator = (const LString<T>& stringSrc)
	{
		if (stringSrc.GetBuffer() == m_pchData)
		{
			return *this;
		}

		T* src = stringSrc.GetBuffer();
		if (src == NULL)
		{
			ResetBuffer();
		}
		else
		{
		    StrAssignment(src, stringSrc.GetLength());
		}

		return *this;
	}

	template<class T>
	const LString<T>& LString<T>::operator = (const T *lpsz)
	{
		if (lpsz == NULL)
		{
			ResetBuffer();
		}
		else
		{
		    StrAssignment(lpsz);
		}
		
		return *this;
	}

	template<class T>
	const LString<T>& LString<T>::operator = (const T ch)
	{
		T str[2] = {ch, 0};
		StrAssignment(str);
		return *this;
	}

	template<class T>
	const LString<T>& LString<T>::operator += (const LString<T>& stringSrc)
	{
		StrPlus(stringSrc.GetBuffer());
		return *this;
	}

	template<class T>
	const LString<T>& LString<T>::operator += (const T *lpsz)
	{
		StrPlus(lpsz);
		return *this;
	}

	template<class T>
	const LString<T>& LString<T>::operator += (const T ch)
	{
		T str[2] = {ch, 0};
		StrPlus(str);
		return *this;
	}

	template<class T>
	T& LString<T>::operator[](LInt nIndex)
	{
		return m_pchData[nIndex];
	}

	template<class T>
	const T& LString<T>::operator[](LInt nIndex) const
	{
        return m_pchData[nIndex];
	}

	template<class T>
	T *LString<T>::GetBuffer() const
	{
		return m_pchData;
	}

	template<class T>
	LInt LString<T>::GetLength() const
	{
	    return m_size;
	}

	template<class T>
	LVoid LString<T>::AllocBuffer(LInt nLen)
	{
		m_pchDataLen = nLen+1;
		m_pchData = new T[nLen+1];
		LMemset(m_pchData, 0, m_pchDataLen*sizeof(T));
	}

	template<class T>
	LVoid LString<T>::StrAssignment(const T* lpsz, LInt nLen)
	{
		if (m_pchDataLen <= nLen)
		{
			ResetBuffer();
			AllocBuffer(nLen);
		}
		else
		{
			LMemset(m_pchData, 0, m_pchDataLen*sizeof(T));
		}

		LMemcpy(m_pchData, lpsz, nLen*sizeof(T));
		m_size = nLen;
	}

	template<class T>
	LVoid LString<T>::StrAssignment(const T *lpsz)
	{
		LInt nLen = CountString(lpsz);
		StrAssignment(lpsz, nLen);
	}

	template<class T>
	LVoid LString<T>::StrPlus(const T *lpsz)
	{
		int plusSize = CountString(lpsz);
		LInt nLen = GetLength() + plusSize;
		if (m_pchDataLen <= nLen)
		{
			if (NULL == m_pchData)
			{
				AllocBuffer(nLen);
				LMemcpy(m_pchData, lpsz, plusSize*sizeof(T));
			}
			else
			{
				T *pOldData = m_pchData;
				LInt nOldDataLen = CountString(pOldData);
				AllocBuffer(nLen);
				LMemcpy(m_pchData, pOldData, nOldDataLen*sizeof(T));
				LMemcpy(m_pchData+nOldDataLen, lpsz, plusSize*sizeof(T));
				delete []pOldData;
			}

		}
		else
		{
			LMemcpy(m_pchData+GetLength(), lpsz, plusSize*sizeof(T));
		}

		m_size = nLen;
	}
	
	template<class T>
	LBool LString<T>::operator == (const T* lpsz) const
	{
        T* src = GetBuffer();
        while (*src && *lpsz)
        {
            if (*src++ != *lpsz++)
            {
                return LFalse;
            }
        }

        return *src || *lpsz ? LFalse : LTrue;
	}

	template<class T>
	LBool LString<T>::operator == (const LString<T>& str1) const
	{
		return CompareCase(str1);
	}

	template<class T>
	LBool LString<T>::CompareCase(const LString<T>& str) const
	{
		LInt nLen = GetLength();
		LInt nComparedLen = str.GetLength();
		if (nLen != nComparedLen)
		{
			return LFalse;
		}

		LInt ipos = 0;
		for (; ipos < nLen; ++ipos)
		{
			if (m_pchData[ipos] != str[ipos])
			{
				return LFalse;
			}
		}
		
		return LTrue;
	}

	template<class T>
	LBool LString<T>::CompareNoCase(const LString<T>& str) const
	{
		LInt nLen = GetLength();
		LInt nComparedLen = str.GetLength();
		if (nLen != nComparedLen)
		{
			return LFalse;
		}

		LString<T> str1 = *this;
		LString<T> str2 = str;
		str1.ToLower();
		str2.ToLower();
		return str1.CompareCase(str2);
	}

	template<class T>
	LBool LString<T>::operator != (const LString<T>& str1) const
	{
		if (*this == str1)
		{
			return LFalse;
		}
		
		return LTrue;
	}

	template<class T>
	LBool LString<T>::operator < (const LString<T>& str1) const
	{
		LInt nLen = GetLength();
		LInt nComparedLen = str1.GetLength();
		LInt nLowerLen = 0;
		LBool bLargeORequal = LFalse;
		if (nLen>=nComparedLen)
		{
			nLowerLen = nLen;
			bLargeORequal = LTrue;
		}
		else
		{
			nLowerLen = nComparedLen;
		}

		LInt ipos=0;
		for (; ipos < nLowerLen; ++ipos)
		{
			if (m_pchData[ipos] < str1[ipos])
			{
				return LTrue;
			}
			if (m_pchData[ipos] > str1[ipos])
			{
				return LFalse;
			}
		}

		if (bLargeORequal)
		{
			return LFalse;
		}

		return LTrue;
	}

	template<class T>
	LBool LString<T>::operator <= (const LString<T>& str1) const
	{
		if ((*this < str1) || (*this == str1))
		{
			return LTrue;
		}
		return LFalse;
	}

	template<class T>
	LBool LString<T>::operator > (const LString<T>& str1) const
	{
		if (*this <= str1)
		{
			return LFalse;
		}
		
		return LTrue;
	}

	template<class T>
	LBool LString<T>::operator >= (const LString<T>& str1) const
	{
		if ((*this > str1) || (*this == str1))
		{
			return LTrue;
		}
		return LFalse;
	}

	template<class T>
	LInt LString<T>::CountString(const T* lpsz) const
	{
		LInt nLen = 0;
		//T* p = (T*)lpsz;
		while ( *(lpsz + nLen) != 0 )
		{
			++nLen;
		}

		return nLen;
	}

	template<class T>
	LString<T>::operator const T*() const
	{
		return (const T*)m_pchData;
	}

	template <class T>
	typename LString<T>::Policy::LChars LString<T>::LCharString() const
	{
		return (const typename LString<T>::Policy::LChars)GetBuffer();
	}

	template<class T>
	LString<T> LString<T>::Mid(LInt nPos, LInt nLength) const
	{
		if (nLength < 0)
		{
			nLength = GetLength() - nPos;
		}	

		return LString<T>(m_pchData + nPos, nLength);
	}

	template<class T>
	LString<T> LString<T>::Left(LInt nLength) const
	{
		return Mid(0, nLength);
	}

	template<class T>
	LString<T> LString<T>::Right(LInt nLength) const
	{
		return Mid(GetLength()-nLength, nLength);
	}

	template<class T>
	LString<T>& LString<T>::ToLower()
	{
		LInt nLen = GetLength();
		for (LInt i = 0; i < nLen; ++i)
		{
			if (m_pchData[i] >= T('A') && m_pchData[i] <= T('Z'))
			{
				m_pchData[i] += T('a') - T('A');
			}
		}
		
		return *this;
	}

	template<class T>
	LString<T>& LString<T>::ToUpper()
	{
		LInt nLen = GetLength();
		for (LInt i = 0; i < GetLength(); ++i)
		{
			if (m_pchData[i] >= T('a') && m_pchData[i] <= T('z'))
			{
				m_pchData[i] += T('A') - T('a');
			}
		}
		
		return *this;
	}

	template<class T>
	LInt LString<T>::Find(const LString<T>& str, LInt nPos) const
	{
		LInt nLen = str.GetLength();
		if (GetLength() >= nLen && nLen)
		{
			T *str_beg = m_pchData + nPos;
			T *str_end = m_pchData + GetLength() - nLen;
			for (; str_beg <= str_end; ++str_beg)
			{
				LString<T> strCmp(str_beg, LFalse, nLen);
				if (strCmp == str)
				{
					return LInt(str_beg - m_pchData);
				}
			}
		}

		return -1;
	}

	template<class T>
	LInt LString<T>::FindNoCase(const LString<T>& str, LInt nPos) const
	{
		LInt nLen = str.GetLength();
		if (GetLength() >= nLen && nLen)
		{
			T *str_beg = m_pchData + nPos;
			T *str_end = m_pchData + GetLength() - nLen;
			for (; str_beg <= str_end; ++str_beg)
			{
				LString<T> strCmp(str_beg, LFalse, nLen);
				if (strCmp.CompareNoCase(str))
				{
					return LInt(str_beg - m_pchData);
				}
			}
		}

		return -1;
	}

	template<class T>
	LInt LString<T>::ReverseFind(const LString<T>& str) const
	{
		LInt nLen = str.GetLength();
		if (m_pchDataLen > nLen && nLen)
		{

			T *str_beg = m_pchData + m_pchDataLen - nLen;
			T *str_end = m_pchData;
			for (; str_beg >= str_end; --str_beg)
			{
				LString<T> strCmp(str_beg, LFalse, nLen);
				if (strCmp == str)
				{
					return LInt(str_beg - m_pchData);
				}
			}
		}

		return -1;
	}

	template<class T>
	LInt LString<T>::ReverseFindNoCase(const LString<T>& str) const
	{
		LInt nLen = str.GetLength();
		if (m_pchDataLen > nLen && nLen)
		{

			T *str_beg = m_pchData + m_pchDataLen - nLen;
			T *str_end = m_pchData;
			for (; str_beg >= str_end; --str_beg)
			{
				LString<T> strCmp(str_beg, LFalse, nLen);
				if (strCmp.CompareNoCase(str))
				{
					return LInt(str_beg - m_pchData);
				}
			}
		}

		return -1;
	}
	
	template<class T>
	LBool LString<T>::EndWith(const LString<T>& sEnd)
	{
		LInt mLen = GetLength();
		LInt sLen = sEnd.GetLength();
		
		if (mLen < sLen)
		{
		    return LFalse;
		}
		
		LBool isEqual = LTrue;
		for (LInt i = 0; i < sLen; ++i)
		{
		    if (m_pchData[mLen-i] != sEnd[sLen-i])
		    {
		        isEqual = LFalse;
		        break;
		    }
		}
		
		return isEqual;
	}
	
	template<class T>
	LBool LString<T>::EndWithNoCase(const LString<T>& sEnd)
	{
		LString<T> str1 = *this;
		LString<T> str2 = sEnd;
		str1.ToLower();
		str2.ToLower();
		
		return str1.EndWith(str2);
	}
	
	template<class T>
	LBool LString<T>::StartWith(const LString<T>& sStart) const
	{
		LInt mLen = GetLength();
		LInt sLen = sStart.GetLength();
		
		if (mLen < sLen)
		{
		    return LFalse;
		}
		
		LBool isEqual = LTrue;
		for (LInt i=0; i < sLen; ++i)
		{
		    if (m_pchData[i] != sStart[i])
		    {
		        isEqual = LFalse;
		        break;
		    }
		}
		
		return isEqual;
	}
	
	template<class T>
	LBool LString<T>::StartWithNoCase(const LString<T>& sStart) const
	{
		LString<T> str1 = *this;
		LString<T> str2 = sStart;
		str1.ToLower();
		str2.ToLower();
		
		return str1.StartWith(str2);
	}
	
	template<class T>
	T LString<T>::CharAt(LInt nIndex) const
	{
		if (nIndex < m_pchDataLen)
		{
			return *(m_pchData+nIndex);
		}

		return (T)0;
	}
	
	template<class T>
	LVoid LString<T>::DeleteCharAt(LInt nIndex)
	{
		LInt len = GetLength();
		if (nIndex < 0 || nIndex >= len)
		{
		    return;
		}
		
		for (LInt i = nIndex; i < len-1; ++i)
		{
		    *(m_pchData + i) = *(m_pchData + i + 1);
		}
		
		*(m_pchData+len-1) = (T)0;
		m_size = len - 1;
	}
	
	template<class T>
	void LString<T>::Replace(const LString<T>& strSrc, const LString<T>& strReplace, int nRep)
	{
		LString<T> strRet;
		LString<T> strRemain = *this;
		LInt nIndex;
		LInt size = 0;
		LInt delta = strReplace.GetLength() - strSrc.GetLength();
		for (LInt i = 0; i != nRep && strRemain.GetLength() && (nIndex = strRemain.Find(strSrc)) >= 0; ++i)
		{
		    strRet += strRemain.Left(nIndex);
		    strRet += strReplace;
			strRemain = strRemain.Mid(nIndex+strSrc.GetLength());
			size += delta;
		}
		
		strRet += strRemain;
		m_size += size;
		*this = strRet;
		
	}
	
	template<class T>
	void LString<T>::ReplaceNoCase(const LString<T>& strSrc, const LString<T>& strReplace, int nRep)
	{
		LString<T> strRet;
		LString<T> strRemain = *this;
		LInt nIndex = 0;
		LInt size = 0;
		LInt delta = strReplace.GetLength() - strSrc.GetLength();
		for (LInt i = 0; i != nRep && strRemain.GetLength() && (nIndex = strRemain.FindNoCase(strSrc)) >= 0; ++i)
		{
		    strRet += strRemain.Left(nIndex);
		    strRet += strReplace;
			strRemain = strRemain.Mid(nIndex+strSrc.GetLength());
			size += delta;
		}
		
		strRet += strRemain;
		m_size += size;
		*this = strRet;
	}

	template<class T>
	LVoid LString<T>::ReleaseBuffer()
	{
		m_pchDataLen = 0;
		m_size = 0;
		m_pchData = NULL;
		m_isDeep = LTrue;
	}

	template<class T>
	LString<T> GetNewString(const T *str1, const T *str2)
	{
		LString<T> str = str1;
		str += str2;
		return str;
	}

	template<class T>
	LString<T> operator + (const LString<T>& string1, const LString<T>& string2)
	{
		return GetNewString(string1.GetBuffer(), string2.GetBuffer());
	}

	template<class T>
	LString<T> operator + (const LString<T>& string1, const T* lpsz)
	{
		return GetNewString(string1.GetBuffer(), lpsz);
	}

	template<class T>
	LString<T> operator + (const T* lpsz, const LString<T>& string1)
	{
		return GetNewString(lpsz, string1.GetBuffer());
	}

	template<class T>
	LString<T> operator + (const LString<T>& string1, const T ch)
	{
		T str[2] = {ch, 0};
		return GetNewString(string1.GetBuffer(), str);
	}

	template<class T>
	LString<T> operator + (const T ch, const LString<T>& string1)
	{
		T str[2] = {ch, 0};
		return GetNewString(str, string1.GetBuffer());
	}
	
}

using util::String;
using util::StringA;
using util::StringW;
using util::CString;

#endif // LSTRING_H

// End of File
