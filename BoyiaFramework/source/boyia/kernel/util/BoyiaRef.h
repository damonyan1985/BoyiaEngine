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
    //LInt count();

    void* operator new(size_t sz);
    void operator delete(void* p);

protected:
    BoyiaRef();

protected:
    //LInt m_refCount;
    RefCount* m_refCount;
};
}

using util::BoyiaRef;

#endif
