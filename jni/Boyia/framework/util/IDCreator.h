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

#include "UtilString.h"
#include "BoyiaValue.h"

namespace util
{
typedef struct BoyiaId {
	BoyiaStr  mStr;
	LUint     mID;
	BoyiaId*  mNext;
} BoyiaId;

typedef struct {
	BoyiaId* mBegin;
	BoyiaId* mEnd;
} IdLink;

class IDCreator
{
public:
	IDCreator();
	LUint genIdentify(const String& str);
	LUint genIdentByStr(const LInt8* str, LInt len);
	LUint genIdentifier(BoyiaStr* str);

private:
	LUint   m_idCount;
	IdLink* m_idLink;
};
}

#endif
