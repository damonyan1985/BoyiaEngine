#include "BoyiaRef.h"
#include "SalLog.h"
//#include "BoyiaMemory.h"

// DOMView树构建在sDOMMemPool这块内存池上，大小为1MB
//#define DOM_MEMORY_SIZE      (LInt)1024*1024
//static LVoid* sDOMMemPool = NULL;

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
    //	if (sDOMMemPool == NULL)
    //    {
    //    	sDOMMemPool = InitMemoryPool(DOM_MEMORY_SIZE);
    //    }
    //
    //	void* data = NewData(sz, sDOMMemPool);
    //	//PrintPoolSize(sDOMMemPool);
    //	return data;
    return malloc(sz);
}

void BoyiaRef::operator delete(void* p)
{
    //DeleteData(p, sDOMMemPool);
    free(p);
}
}
