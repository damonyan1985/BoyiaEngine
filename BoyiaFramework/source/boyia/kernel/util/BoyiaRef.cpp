#include "BoyiaRef.h"
#include "SalLog.h"

namespace util {
BoyiaRef::BoyiaRef()
    : m_refCount(new RefCount())
{
}

BoyiaRef::~BoyiaRef()
{
    m_refCount->release();
}

void BoyiaRef::ref()
{
    m_refCount->ref();
}

LVoid BoyiaRef::onlyDeref()
{
    m_refCount->deref();
}

void BoyiaRef::deref()
{
    m_refCount->deref();
    if (m_refCount->shareCount() <= 0) {
        delete this;
    }
}


RefCount* BoyiaRef::count() const
{
    return m_refCount;
}

void* BoyiaRef::operator new(size_t sz)
{
    KLOG("BoyiaRef::operator new");
    return malloc(sz);
}

void BoyiaRef::operator delete(void* p)
{
    free(p);
}
}
