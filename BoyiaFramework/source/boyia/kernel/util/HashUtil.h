#ifndef HashUtil_h
#define HashUtil_h

#include "StringUtils.h"

namespace util {
class HashString {
public:
    HashString(const HashString& str, LBool deep = LTrue)
    {
        Copy(str.m_value, deep);
    }

    HashString(const String& value, LBool deep = LTrue)
    {
        Copy(value, deep);
    }

    LVoid Copy(const String& value, LBool deep = LTrue)
    {
        if (deep) {
            m_value = value;
        } else {
            m_value.Copy(value.GetBuffer(), LFalse, value.GetLength());
        }
    }

    bool operator == (const HashString& str1) const
    {
        return m_value.CompareCase(str1.m_value);
    }

    LUint hash() const
    {
        return StringUtils::hashCode(m_value);
    }

private:   
    String m_value;
};
}

using util::HashString;
#endif // !HashUtil_h

