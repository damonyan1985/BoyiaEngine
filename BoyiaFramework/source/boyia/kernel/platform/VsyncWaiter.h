#ifndef VsyncWaiter_h
#define VsyncWaiter_h

#include <functional>
#include "BoyiaRef.h"
#include "Lock.h"

namespace yanbo {
class VsyncWaiter : public BoyiaRef {
public:
    using Callback = std::function<void()>;
    static VsyncWaiter* createVsyncWaiter();
    LVoid awaitVsyncForCallback(const Callback& callback);
    
protected:
    virtual LVoid awaitVSync() = 0;
    LVoid fireCallback();
    
private:
    Lock m_lock;
    Callback m_callback;
};
}
#endif
