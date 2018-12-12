#ifndef KRef_h
#define KRef_h

#include "PlatformLib.h"

namespace util
{
class KRef
{
public:
	virtual ~KRef();

	void ref();
	void deref();
	LInt count();

protected:
	KRef();

protected:
	LInt m_refCount;
};
}

using util::KRef;

#endif
