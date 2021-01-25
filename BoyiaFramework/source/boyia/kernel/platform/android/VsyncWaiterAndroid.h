#ifndef VsyncWaiterAndroid_h
#define VsyncWaiterAndroid_h

#include "VsyncWaiter.h"

namespace yanbo {
class VsyncWaiterAndroid : public VsyncWaiter {
public:
    virtual LVoid awaitVSync();
};
}
#endif
