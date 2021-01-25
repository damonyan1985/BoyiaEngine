#ifndef VsyncWaiter_h
#define VsyncWaiter_h

#include "BoyiaRef.h"

namespace yanbo {
class VsyncWaiter : public BoyiaRef {
public:
    static VsyncWaiter* createVsyncWaiter();
    virtual LVoid awaitVSync() = 0;
};
}
#endif
