#ifndef VsyncWaiterWin_h
#define VsyncWaiterWin_h

#include "VsyncWaiter.h"
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace yanbo {
template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
class VsyncWaiterWin : public VsyncWaiter {
public:
    VsyncWaiterWin();
    LVoid setWindow(HWND hwnd);
    virtual LVoid awaitVSync();

private:
    LBool getVSyncParametersIfAvailable();
    const ComPtr<ID3D11Device> m_d3d11Device;
    ComPtr<IDXGIOutput> m_primaryOutput;
    HMONITOR m_primaryMonitor;
    HWND m_hwnd;
};
}
#endif