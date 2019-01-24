#ifndef BoyiaRef_h
#define BoyiaRef_h

#include "PlatformLib.h"

namespace util
{
class BoyiaRef
{
public:
	virtual ~BoyiaRef();

	void ref();
	void deref();
	LInt count();

protected:
	BoyiaRef();

protected:
	LInt m_refCount;
};
}

using util::BoyiaRef;

#endif
