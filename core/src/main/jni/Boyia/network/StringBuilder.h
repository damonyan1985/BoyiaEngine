#ifndef StringBuilder_h
#define StringBuilder_h

#include "KList.h"
#include "UtilString.h"
#include "BoyiaPtr.h"

namespace yanbo
{
struct BufferItem;
class StringBuilder
{
public:
	StringBuilder();
	virtual ~StringBuilder();

public:
	void append(const LByte ch);
	void append(const String& str);
	void append(const LByte* buffer, int pos, int len, LBool isConst);
	void append(const LByte* buffer);
	BoyiaPtr<String> toString() const;
	void clear();

	LInt size() const;

private:
    KList<BoyiaPtr<BufferItem> > m_buffer;
    int m_length;
};
}
#endif
