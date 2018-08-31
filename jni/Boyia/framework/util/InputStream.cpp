/*
 * InputStream.cpp
 *
 *  Created on: 2011-6-24
 *      Author: yanbo
 */

#include "InputStream.h"

namespace util
{

InputStream::InputStream()
    : m_readCount(0)
    , m_len(0)
    , m_stream(_CS("")) 
{	
}

InputStream::InputStream(const InputStream& is)
{
	m_stream    = is.m_stream;
	m_readCount = is.m_readCount;
	m_len       = m_stream.GetLength();
}

InputStream::InputStream(const String& stream)
    : m_stream(stream, LFalse)
    , m_readCount(0)
    , m_len(m_stream.GetLength())
{
}

InputStream::~InputStream()
{
	m_readCount = 0;
}

LInt InputStream::read()
{
	LUint8* p = m_stream.GetBuffer();
	LInt i = (LInt)(*(p+m_readCount));
	m_readCount++;
	
	return i;
}

LBool InputStream::isEnd()
{
	LBool ret = LFalse;
	if (m_readCount >= m_len + 1)
    {
	    ret = LTrue;
    }
	
	return ret;
}

}
