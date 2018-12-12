/*
 * InputStream.h
 *
 *  Created on: 2011-6-24
 *      Author: yanbo
 */

#ifndef InputStream_h
#define InputStream_h

#include "UtilString.h"

namespace util
{

class InputStream
{
public:
	InputStream();
	InputStream(const InputStream& is);
	InputStream(const String& stream);
	virtual ~InputStream();
	
public:
	LInt  read();
	LBool isEnd();
	
protected:
	String   m_stream;
	LInt     m_readCount;
	LInt     m_len;
};

}

#endif /* INPUTSTREAM_H_ */
