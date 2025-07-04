/*
============================================================================
 Name        : IDCreator.h
 Author      : yanbo
 Version     : 1.0
 Copyright   : All Copyright Reserved
 Description : support Original
============================================================================
*/
#ifndef IDCreator_h
#define IDCreator_h

#include "BoyiaValue.h"
#include "UtilString.h"
#include "OwnerPtr.h"
#include "KVector.h"

namespace util {

struct IdLink;
class IDCreator {
public:
    IDCreator();
    LUint genIdentify(const String& str);
    LUint genIdentByStr(const LInt8* str, LInt len);
    LUint genIdentifier(BoyiaStr* str);
    LVoid getIdentName(LUint id, BoyiaStr* str);
    LVoid appendIdentify(const String& str, LUint id);
    OwnerPtr<String> idsToString();

private:
    KVector<BoyiaStr*> m_strIds;
};
}

#endif
