#include "BoyiaRef.h"

namespace util
{
BoyiaRef::BoyiaRef()
    : m_refCount(0)
{
}

BoyiaRef::~BoyiaRef()
{
}

void BoyiaRef::ref()
{
	++m_refCount;
}

void BoyiaRef::deref()
{
    if (--m_refCount <= 0)
    {
    	delete this;
    }
}

LInt BoyiaRef::count()
{
    return m_refCount;
}

}
