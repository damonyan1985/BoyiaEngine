#ifndef BoyiaRef_h
#define BoyiaRef_h

#include "RefCount.h"
#include <stdlib.h>

namespace util {
class BoyiaRef {
public:
    virtual ~BoyiaRef();

    LVoid ref();
    LVoid deref();
    // 只做引用计数减一，但不释放内存
    LVoid onlyDeref();
    RefCount* count() const;

    void* operator new(size_t sz);
    void operator delete(void* p);

protected:
    BoyiaRef();

protected:
    RefCount* m_refCount;
};
}

using util::BoyiaRef;

#endif
