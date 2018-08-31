/*
 * Utf8StreamReader.h
 *
 *  Created on: 2011-7-21
 *      Author: yanbo
 */

#ifndef Utf8StreamReader_h
#define Utf8StreamReader_h

#include "InputStream.h"

namespace util
{
#define MAX_PARSER_STACK_DEPTH 2

class Utf8StreamReader
{
protected:
	InputStream m_is;
	int         m_top;
	int*        m_stack;
	
public:
	Utf8StreamReader(InputStream& is);
	Utf8StreamReader(String& stream);
	virtual ~Utf8StreamReader();
	
public:
	int read();
	
private:
	int readByte();
	void push(int c);
	int pop();
};

}

#endif /* UTF8STREAMREADER_H_ */
