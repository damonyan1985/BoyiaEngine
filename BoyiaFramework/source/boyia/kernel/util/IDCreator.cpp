#include "IDCreator.h"

namespace util {
//typedef struct BoyiaId {
//    BoyiaStr mStr;
//    LUint mID;
//    BoyiaId* mNext;
//} BoyiaId;
//
//typedef struct IdLink {
//    BoyiaId* mBegin;
//    BoyiaId* mEnd;
//} IdLink;

IDCreator::IDCreator()
//    : m_idCount(0)
//    , m_idLink(kBoyiaNull)
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
//    if (!m_idLink) {
//        m_idLink = new IdLink;
//        m_idLink->mBegin = kBoyiaNull;
//        m_idLink->mEnd = kBoyiaNull;
//    }
//
//    BoyiaId* id = m_idLink->mBegin;
//    while (id) {
//        if (MStrcmp(str, &id->mStr)) {
//            return id->mID;
//        }
//
//        id = id->mNext;
//    }

//    id = new BoyiaId;
//    id->mID = m_idLink->mEnd ? m_idLink->mEnd->mID + 1 : m_idCount + 1;
//    id->mStr.mPtr = NEW_BUFFER(LInt8, str->mLen);
//    id->mStr.mLen = str->mLen;
//    LMemcpy(id->mStr.mPtr, str->mPtr, str->mLen);
//    id->mNext = kBoyiaNull;
//
//    if (!m_idLink->mBegin) {
//        m_idLink->mBegin = id;
//    } else {
//        m_idLink->mEnd->mNext = id;
//    }
//
//    m_idLink->mEnd = id;
//
//    ++m_idCount;
//    return id->mID;
    
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
//    if (!m_idLink) {
//        str->mLen = 0;
//        str->mPtr = kBoyiaNull;
//        return;
//    }
//
//    BoyiaId* bid = m_idLink->mBegin;
//    while (bid) {
//        if (bid->mID == id) {
//            str->mLen = bid->mStr.mLen;
//            str->mPtr = bid->mStr.mPtr;
//            return;
//        }
//
//        bid = bid->mNext;
//    }
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
//    String buffer((LUint8)0, 1024);
//    BoyiaId* bid = m_idLink->mBegin;
//    while (bid) {
//        buffer += String(_CS(bid->mStr.mPtr), LFalse, bid->mStr.mLen);
//        buffer += _CS(":");
//        LUint8 str[256];
//        LInt2Str(bid->mID, str, 10);
//        buffer += str;
//        if (bid != m_idLink->mEnd) {
//            buffer += _CS("\n");
//        }
//
//        bid = bid->mNext;
//    }
//
//    OwnerPtr<String> ownerString = new String(buffer.GetBuffer(), LFalse, buffer.GetLength());
//    buffer.ReleaseBuffer();
//    return ownerString;
    
    String buffer((LUint8)0, 1024);
    for (LInt index = 0; index < m_strIds.size(); index++) {
        LInt id = index + 1;
        
        BoyiaStr* idStr = m_strIds.elementAt(index);
        buffer += String(_CS(idStr->mPtr), LFalse, idStr->mLen);
        buffer += _CS(":");
        LUint8 str[256];
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
//    if (!m_idLink) {
//        m_idLink = new IdLink;
//        m_idLink->mBegin = kBoyiaNull;
//        m_idLink->mEnd = kBoyiaNull;
//    }
//
//    BoyiaId* bid = new BoyiaId;
//    bid->mID = id;
//    bid->mStr.mPtr = NEW_BUFFER(LInt8, str.GetLength());
//    bid->mStr.mLen = str.GetLength();
//    LMemcpy(bid->mStr.mPtr, str.GetBuffer(), str.GetLength());
//    bid->mNext = kBoyiaNull;
//
//    if (!m_idLink->mBegin) {
//        m_idLink->mBegin = bid;
//    } else {
//        m_idLink->mEnd->mNext = bid;
//    }
//
//    m_idLink->mEnd = bid;
//    m_idCount = id;
    BoyiaStr* newStr = new BoyiaStr;
    newStr->mPtr = NEW_BUFFER(LInt8, str.GetLength());
    newStr->mLen = str.GetLength();
    LMemcpy(newStr->mPtr, str.GetBuffer(), str.GetLength());
    m_strIds.addElement(newStr);
}
}
