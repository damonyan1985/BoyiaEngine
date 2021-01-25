#ifndef VsyncWaiterWin_h
#define VsyncWaiterWin_h

#include "VsyncWaiter.h"
#include <Windows.h>

namespace yanbo {
class VsyncWaiterWin : public VsyncWaiter {
public:
    VsyncWaiterWin();
    LVoid setWindow(HWND hwnd);
    virtual LVoid awaitVSync();

private:
    LBool getVSyncParametersIfAvailable();

    HWND m_hwnd;
};
}
#endif