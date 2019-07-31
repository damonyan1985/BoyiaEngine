#include "BoyiaRef.h"
#include "SalLog.h"
//#include "BoyiaMemory.h"

// DOMView树构建在sDOMMemPool这块内存池上，大小为1MB
//#define DOM_MEMORY_SIZE      (LInt)1024*1024
//static LVoid* sDOMMemPool = NULL;

namespace util {
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
    if (--m_refCount <= 0) {
        delete this;
    }
}

LInt BoyiaRef::count()
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
    //	//__android_log_print(ANDROID_LOG_INFO, "MiniJS", "DOMMemPool  addr=%x size=%d", (LInt)sDOMMemPool->m_address, sDOMMemPool->m_used);
    //	return data;
    return malloc(sz);
}

void BoyiaRef::operator delete(void* p)
{
    //DeleteData(p, sDOMMemPool);
    free(p);
}
}