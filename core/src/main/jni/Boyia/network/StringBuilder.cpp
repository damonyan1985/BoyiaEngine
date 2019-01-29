#include "StringBuilder.h"
#include "SalLog.h"

namespace yanbo
{
// const字串和CCString不能执行delete
// 故需要加以判断，防止出错
class BufferItem : public BoyiaRef
{
public:
	BufferItem(LBool isConst = LFalse)
	    : buffer(NULL)
	    , length(0)
		, pos(0)
	    , bConst(isConst)
	{
	}

	~BufferItem()
	{
		if (buffer != NULL && !bConst)
		{
			KFORMATLOG("HTTPParser::parseResponse delete item address=%d and length=%d", (LIntPtr) buffer, length);
            delete[] buffer;
            buffer = NULL;
		}
	}

public:
	LByte* buffer;
    int length;
	int pos;
	int bConst;
};

StringBuilder::StringBuilder()
    : m_length(0)
{
}

StringBuilder::~StringBuilder()
{
}

void StringBuilder::append(const LByte ch)
{
	LByte* chPtr = new LByte[1];
	*chPtr = ch;
	append(chPtr, 0, 1, LFalse);
}

void StringBuilder::append(const String& str)
{
    append(str.GetBuffer(), 0, str.GetLength(), LTrue);
}

void StringBuilder::append(const LByte* buffer)
{
	append(buffer, 0, util::LStrlen((const LByte*)buffer), LTrue);
}

void StringBuilder::append(const LByte* buffer, int pos, int len, LBool isConst)
{
	if (len <= 0)
	{
		return;
	}

	BoyiaPtr<BufferItem> item = new BufferItem(isConst);
	item->buffer = (LByte*)(buffer);
	item->length = len;
	item->pos = pos;
	m_length += len;
	m_buffer.push(item);
}

BoyiaPtr<String> StringBuilder::toString() const
{
	KFORMATLOG("StringBuilder::append str m_length=%d", m_length);
	if (!m_length)
	{
		return NULL;
	}

	//KFORMATLOG("StringBuilder::append str m_length1=%d", m_length);
	LByte* buffer = new LByte[m_length+1];
    util::LMemset(buffer, 0, m_length+1);
    KList<BoyiaPtr<BufferItem> >::Iterator iter = m_buffer.begin();
    KList<BoyiaPtr<BufferItem> >::Iterator iterEnd = m_buffer.end();

    int pos = 0;
    for (; iter != iterEnd; ++iter)
    {
        util::LMemcpy(buffer+pos, (*iter)->buffer + (*iter)->pos, (*iter)->length);
        pos += (*iter)->length;
    }

	// 这里使用字符串浅拷贝，因此不用释放buffer
	// 最后的释放过程由sptr代理完成
	return new String(buffer, LTrue, m_length);
}

void StringBuilder::clear()
{
	m_length = 0;
	m_buffer.clear();
}

LInt StringBuilder::size() const
{
	return m_buffer.count();
}

}
