#include "IDCreator.h"

namespace util
{
IDCreator::IDCreator()
    : m_idCount(0)
    , m_idLink(NULL)
{
}

LUint IDCreator::genIdentify(const String& str)
{
	return genIdentByStr((const LInt8*)str.GetBuffer(), str.GetLength());
}

LUint IDCreator::genIdentByStr(const LInt8* str, LInt len)
{
	BoyiaStr strId;
	strId.mPtr = (LInt8*)str;
	strId.mLen = len;
	return genIdentifier(&strId);
}

LUint IDCreator::genIdentifier(BoyiaStr* str)
{
	if (!m_idLink)
	{
		m_idLink = new IdLink;
		m_idLink->mBegin = NULL;
		m_idLink->mEnd = NULL;
	}

	BoyiaId* id = m_idLink->mBegin;
	while (id)
	{
		if (MStrcmp(str, &id->mStr))
		{
			return id->mID;
		}

		id = id->mNext;
	}

	id = new BoyiaId;
	id->mID = ++m_idCount;
	id->mStr.mPtr = new LInt8[str->mLen];
	id->mStr.mLen = str->mLen;
	LMemcpy(id->mStr.mPtr, str->mPtr, str->mLen);
	id->mNext = NULL;

	if (!m_idLink->mBegin)
	{
		m_idLink->mBegin = id;
	}
	else
	{
		m_idLink->mEnd->mNext = id;
	}

	m_idLink->mEnd = id;

	return id->mID;
}
}
