/*
 * StringUtils.cpp
 *
 *  Created on: 2011-6-25
 *  Modify on: 2018-9-2 9:30
 *      Author: yanbo
 */

#include "StringUtils.h"
#include "SalLog.h"
#include "BoyiaValue.h"

namespace util
{

// delete the begin and end blank string from the source
String StringUtils::skipBlank(const String& src)
{
    LInt len = src.GetLength();
    LCharA ch;
    const LUint8* pos = src.GetBuffer();
    LInt length = len;

    for (LInt i = 0; i < len; i++) 
    {
        ch = (LCharA) src.CharAt(i);
        if (isspace((LInt)ch))
        {
            ++pos;
            --length;
        }
        else
        {
            break;
        }
    }

    if (length == 0)
    {
        return (const LUint8*)NULL;
    }

    for (LInt j = len - 1; j >= 0; j--)
    {
        ch = (LCharA) src.CharAt(j);
        if (isspace((LInt)ch))
        {
            --length;
        }
        else
        {
            break;
        }
    }

    return String(pos, length);
}

LInt StringUtils::stringToInt(const String& src, LInt radix)
{
	return LStr2Int(src.GetBuffer(), radix);
}

KVector<String>* StringUtils::split(const String& src, const String& splitSrc)
{
	String srcTmp = src;
	KVector<String>* splits = new KVector<String>();
	
	LInt strLen = srcTmp.GetLength();
	LInt splitLen = splitSrc.GetLength();
	
	LInt i = srcTmp.Find(splitSrc);
	while (i >= 0)
	{
	    String str = srcTmp.Mid(0, i);
	    splits->addElement(str);

	    if (i+splitLen < strLen)
	    {
	        srcTmp = srcTmp.Mid(i+splitLen);
	        strLen = srcTmp.GetLength();
	    }
	    else
	    {
	        srcTmp = (const LUint8*)NULL;
	        break;
	    }
	    
	    if (srcTmp.GetLength() > 0)
	    {
	        i = srcTmp.Find(splitSrc);
	    }
	    
	}
	
    if (srcTmp.GetLength() > 0)
    {
        splits->addElement(srcTmp);
    }
    
    return splits;
}

LBool StringUtils::isspace(LInt ch)
{
    return LIsSpace(ch);
}

LBool StringUtils::isdigit(LInt ch)
{
//	if(ch >= '0' && ch <= '9')
//	{
//	    return LTrue;
//	}
//
//	return LFalse;
    return LIsDigit(ch);
}

LBool StringUtils::isalpha(LInt ch)
{
//	if (ch >= 'a' && ch <= 'z')
//	{
//	    return LTrue;
//	}
//
//	if (ch >= 'A' && ch <= 'Z')
//	{
//	    return LTrue;
//	}
//
//	return LFalse;
    return LIsAlpha(ch);
}

LBool StringUtils::isalnum(LInt ch)
{
	if (ch >= '0' && ch <= '9')
	{
	    return LTrue;
	}
	
	if (ch >= 'a' && ch <= 'z')
	{
	    return LTrue;
	}
	
	if (ch >= 'A' && ch <= 'Z')
	{
	    return LTrue;
	}
	
	return LFalse;
}

LInt StringUtils::tolower(LInt ch)
{
	if (ch >= 'A' && ch <= 'Z')
	{
	    ch += 'a' - 'A';
	}

	return ch;
}

LInt StringUtils::toupper(LInt ch)
{
	if (ch >= 'a' && ch <= 'z')
	{
		ch += 'A' - 'a';
	}

	return ch;
}


// utf8 to unicode begin
static int cU8xU(LUint16* pOut,char *pText)
{ 
    int ret = 0;
    char* uchar = (char *)pOut;
    unsigned cIn = (unsigned char)pText[0];
    if (cIn < 0x80) // ASCII  0x00 ~ 0x7f
    {
        pOut[0] = pText[0]; 
    }
    else if (cIn < 0xdf)
    {
        uchar[0] = (pText[0]<<6)|(pText[1]&0x3f); 
        uchar[1] = (pText[0]>>2)&0x0f; 
        ret = 1; 
    }
    else if (cIn <= 0xef) // "，"
    {
        uchar[0] = (pText[1]<<6)|(pText[2]&0x3f); 
        uchar[1] = (pText[0]<<4)|((pText[1]>>2)&0x0f); 
        ret = 2; 
    }
    else if (cIn < 0xf7)
    {
        uchar[0] = (pText[2]<<6)|(pText[3]&0x3f); 
        uchar[1] = (pText[1]<<4)|((pText[2]>>2)&0x0f); 
        uchar[2] = ((pText[0]<<2)&0x1c)|((pText[1]>>4)&0x03); 
        ret = 3; 
    }

    return ret;
} 

// utf8 to unicode end
static int sU8xU(LUint16* pOut,char *pText,int len)
{ 
    int i,j;
    for (i=0, j=0; i<len; i++, j++)
    {
        i += cU8xU(&pOut[j],&pText[i]);
    }

    return j;
}

// unicode to utf8 begin
static int cUxU8(char* pOut, LUint16* pText)
{ 
    int ret = 0;
    unsigned char* pchar = (unsigned char *)pText;
    if (pText[0]<=0x7f) // ASCII  0x00 ~ 0x7f
    {
        pOut[0] = (char)pchar[0]; 
    }
    else if (pText[0]<=0x7ff) // 0x080 ~ 0x7ff
    {
        pOut[0] = 0xc0|(pchar[1]<<2)|(pchar[0]>>6); 
        pOut[1] = 0x80|(pchar[0]&0x3f); 
        ret = 1; 
    }
    else                    // 0x0800 ~ 0xFFFF
    {
        pOut[0] = 0xe0|(pchar[1]>>4);
        pOut[1] = 0x80|((pchar[1]&0x0f)<<2)|(pchar[0]>>6);
        pOut[2] = 0x80|(pchar[0]&0x3f);
        ret = 2;
    }

    return ret;
}

static int sUxU8(char* pOut, LUint16* pText,int len)
{
    int i,j;
    for (i=0, j=0; i < len; i++, j++)
    {
        j += cUxU8(&pOut[j], &pText[i]);
    }
    return j;
} 

// unicode to utf8 end
void StringUtils::strTostrW(const String& src, StringW& dest)
{
	int len = src.GetLength();
	KFORMATLOG("strTostrW d=%s", (const char*)src.GetBuffer());
	LUint16 *lpwsz = new LUint16[3*len+1];
	LMemset(lpwsz, 0, (3*len+1)*sizeof(LUint16));
	sU8xU(lpwsz, (char*)src.GetBuffer(), len);
	//lpwsz[len] = L'\0';
	dest = (LUint16*)lpwsz;
    //dest.copyString(lpwsz, LTrue, len);
	delete []lpwsz;
}

void StringUtils::strWtoStr(const StringW& src, String& dest)
{
	KLOG("strWtoStr0");
	int len = src.GetLength();
	LUint8* d = new LUint8[3*len+1];
	LMemset(d, 0, 3*len+1);
	KLOG("strWtoStr1");
	sUxU8((char*)d, (LUint16*)src.GetBuffer(), /*2*len*/len);
	dest.Copy(d, LTrue, 3*len+1);
	KFORMATLOG("strWtoStr d=%s", (const char*)d);
}

void StringUtils::replaceSpecialChars(util::String& text)
{
    text = StringUtils::skipBlank(text);
	text.ReplaceNoCase(_CS("&nbsp;"), _CS(" "));
	text.ReplaceNoCase(_CS("&#39;"), _CS(""));
	text.ReplaceNoCase(_CS("&#x20;"), _CS(" "));
	text.ReplaceNoCase(_CS("&#039;"), _CS("'"));
	text.ReplaceNoCase(_CS("&#160;"), _CS(""));
}

LUint StringUtils::hashCode(const String& keyName)
{
    return GenHashCode((const LInt8*)keyName.GetBuffer(), keyName.GetLength());
}
}
