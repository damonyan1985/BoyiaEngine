#ifndef HashUtil_h
#define HashUtil_h

#include "StringUtils.h"
#include "PlatformBridge.h"

namespace util {
class HashString {
public:
    HashString(const HashString& str, LBool deep = LTrue)
    {
        Copy(str, deep);
    }

    HashString(const String& value, LBool deep = LTrue)
        : m_hash(StringUtils::hashCode(value))
    {
        Copy(value, deep);
    }

    LVoid Copy(const HashString& hashString, LBool deep = LTrue)
    {
        if (deep) {
            m_value = hashString.value();            
        } else {
            m_value.Copy(hashString.GetBuffer(), LFalse, hashString.length());
        }

        m_hash = hashString.hash();
    }


    LVoid Copy(const String& value, LBool deep = LTrue)
    {
        if (deep) {
            m_value = value;
        } else {
            m_value.Copy(value.GetBuffer(), LFalse, value.GetLength());
        }
    }

    bool operator==(const HashString& str1) const
    {
        if (m_hash != str1.m_hash) {
            return false;
        }

        return m_value.CompareCase(str1.m_value);
    }

    LUint hash() const
    {
        return m_hash;
    }
    
    LUint8* GetBuffer() const
    {
        return m_value.GetBuffer();
    }
    
    LInt GetStringSize() const
    {
        return yanbo::PlatformBridge::getTextSize(m_value);
    }

    const String& value() const
    {
        return m_value;
    }

    LInt length() const
    {
        return m_value.GetLength();
    }
    
private:
    String m_value;
    LUint m_hash;
};

class HashPtr {
public:
    HashPtr(LUintPtr ptr)
        : m_ptr(ptr)
    {
    }

    bool operator==(const HashPtr& ptr) const
    {
        return m_ptr == ptr.m_ptr;
    }

    LUint hash() const
    {
        return (LUint)m_ptr;
    }

    LUintPtr value() const {
        return m_ptr;
    }

private:
    LUintPtr m_ptr;
};

class HashInt {
public:
    HashInt(LInt value)
        : m_value(value)
    {
    }

    bool operator==(const HashInt& hashInt) const
    {
        return m_value == hashInt.m_value;
    }

    LUint hash() const
    {
        return (LUint)m_value;
    }

private:
    LInt m_value;
};
}

using util::HashString;
using util::HashPtr;
using util::HashInt;
#endif // !HashUtil_h
