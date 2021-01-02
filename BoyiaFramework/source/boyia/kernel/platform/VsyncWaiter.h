#ifndef VsyncWaiter_h
#define VsyncWaiter_h

#include "BoyiaRef.h"

namespace yanbo {
class VsyncWaiter : public BoyiaRef {
public:
    virtual LVoid awaitVSync();
};
}
#endif
