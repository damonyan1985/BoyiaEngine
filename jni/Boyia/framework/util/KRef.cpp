#include "KRef.h"

namespace util
{
KRef::KRef()
    : m_refCount(0)
{
}

KRef::~KRef()
{
}

void KRef::ref()
{
	++m_refCount;
}

void KRef::deref()
{
    if (--m_refCount <= 0)
    {
    	delete this;
    }
}

LInt KRef::count()
{
    return m_refCount;
}

}
