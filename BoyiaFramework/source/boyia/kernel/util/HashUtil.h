#ifndef HashUtil_h
#define HashUtil_h

#include "StringUtils.h"

namespace util {
class HashString {
public:
    HashString(const HashString& str, LBool deep = LTrue)
        : HashString(str.m_value, deep)
    {
    }

    HashString(const String& value, LBool deep = LTrue)
        : m_hash(StringUtils::hashCode(value))
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
        return m_hash;
    }

private:   
    String m_value;
    LUint m_hash;
};
}

using util::HashString;
#endif // !HashUtil_h

