#ifndef StringBuilder_h
#define StringBuilder_h

#include "KList.h"
#include "OwnerPtr.h"
#include "UtilString.h"

namespace yanbo {

struct BufferItem;
class StringBuilder {
public:
    StringBuilder();
    virtual ~StringBuilder();

public:
    void append(const LByte ch);
    void append(const String& str);
    void append(const LByte* buffer, LInt pos, LInt len, LBool isConst);
    void append(const LByte* buffer);
    OwnerPtr<String> toString() const;
    void clear();

    LInt size() const;

private:
    KList<OwnerPtr<BufferItem>> m_buffer;
    int m_length;
};
}
#endif
