/*
 * Utf8StreamReader.cpp
 *
 *  Created on: 2011-7-21
 *      Author: yanbo
 */

#include "Utf8StreamReader.h"

namespace util
{

Utf8StreamReader::Utf8StreamReader(InputStream& is)
    : m_is(is)
    , m_top(-1)
    , m_stack(NULL)
{
	m_stack = new int[MAX_PARSER_STACK_DEPTH];
}

Utf8StreamReader::Utf8StreamReader(String& stream)
    : m_is(stream)
    , m_top(-1)
{
	m_stack = new int[MAX_PARSER_STACK_DEPTH];
}

Utf8StreamReader::~Utf8StreamReader()
{
	if(NULL != m_stack)
	{
	    delete[] m_stack;
	    m_stack = NULL;
	}
}

int Utf8StreamReader::read()
{
    int c = readByte();
    if (c == -1)
    {
        return (c);
    }
    switch (c >> 4)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        /* 0xxxxxxx */
        return (c);
    case 12:
    case 13: /* 110x xxxx 10xx xxxx */
    {
        int c2 = readByte();
        if ((c2 & 0xC0) != 0x80)
        {
            push(c2);
            return (c); // Recovery
        }
        int res = ((c & 0x1F) << 6) | (c2 & 0x3F);
        return (res);
    }
    case 14: /* 1110 xxxx 10xx xxxx 10xx xxxx */
    {
        int c2 = readByte();
        if (((c2 & 0xC0) != 0x80))
        {
             push(c2);
             return (c); // Recovery
        }
        int c3 = readByte();
        if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80))
        {
            push(c3);
            push(c2);
            return (c); // Recovery
        }
        int res = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6)
                | ((c3 & 0x3F) << 0);
        return (res);
    }
    default: /* 10xx xxxx, 1111 xxxx */
        return (c); // Recovery
    }
}

int Utf8StreamReader::readByte()
{
    if (m_top < 0)
    {
        return m_is.read(); 
    } 
    else
    {
        return pop();
    }
}

void Utf8StreamReader::push(int c)
{
	m_stack[m_top] = c;
}

int Utf8StreamReader::pop()
{
	return m_stack[m_top--];
}

}
