#include "IDCreator.h"

namespace util {

IDCreator::IDCreator()
    : m_strIds(0, 512)
{
}

LUint IDCreator::genIdentify(const String& str)
{
    return genIdentByStr((const LInt8*)str.GetBuffer(), str.GetLength());
}

LUint IDCreator::genIdentByStr(const LInt8* str, LInt len)
{
    BoyiaStr strId;
    strId.mPtr = (LInt8*)str;
    strId.mLen = len;
    return genIdentifier(&strId);
}

// id从1开始自增
LUint IDCreator::genIdentifier(BoyiaStr* str)
{    
    for (LInt index = 0; index < m_strIds.size(); index++) {
        if (MStrcmp(str, m_strIds[index])) {
            return index + 1;
        }
    }
    
    BoyiaStr* newStr = new BoyiaStr;
    newStr->mPtr = NEW_BUFFER(LInt8, str->mLen);
    newStr->mLen = str->mLen;
    LMemcpy(newStr->mPtr, str->mPtr, str->mLen);
    
    m_strIds.addElement(newStr);
    return m_strIds.size();
}

LVoid IDCreator::getIdentName(LUint id, BoyiaStr* str)
{
    if (!m_strIds.size()) {
        str->mLen = 0;
        str->mPtr = kBoyiaNull;
        return;
    }
    
    LInt index = id - 1;
    BoyiaStr* idStr = m_strIds.elementAt(index);
    str->mLen = idStr->mLen;
    str->mPtr = idStr->mPtr;
}

OwnerPtr<String> IDCreator::idsToString()
{    
    String buffer((LUint8)0, KB);
    for (LInt index = 0; index < m_strIds.size(); index++) {
        LInt id = index + 1;
        
        BoyiaStr* idStr = m_strIds.elementAt(index);
        buffer += String(_CS(idStr->mPtr), LFalse, idStr->mLen);
        buffer += _CS(":");
        LUint8 str[MAX_PATH_SIZE];
        LInt2Str(id, str, 10);
        buffer += str;
        if (index != m_strIds.size() - 1) {
            buffer += _CS("\n");
        }
    }
    
    OwnerPtr<String> ownerString = new String(buffer.GetBuffer(), LFalse, buffer.GetLength());
    buffer.ReleaseBuffer();
    return ownerString;
}

LVoid IDCreator::appendIdentify(const String& str, LUint id)
{
    BoyiaStr* newStr = new BoyiaStr;
    newStr->mPtr = NEW_BUFFER(LInt8, str.GetLength());
    newStr->mLen = str.GetLength();
    LMemcpy(newStr->mPtr, str.GetBuffer(), str.GetLength());
    m_strIds.addElement(newStr);
}
}
