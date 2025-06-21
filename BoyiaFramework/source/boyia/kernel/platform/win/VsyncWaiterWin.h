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
    VsyncWaiterWin(ComPtr<IDXGIDevice> dxgiDevice);
    LVoid setWindow(HWND hwnd);
    virtual LVoid awaitVSync();

private:
    LBool getVSyncParametersIfAvailable();
    const ComPtr<ID3D11Device> m_dxgiDevice;
    ComPtr<IDXGIOutput> m_primaryOutput;
    HMONITOR m_primaryMonitor;
    ComPtr<IDXGIAdapter> m_dxgiAdapter;
    HWND m_hwnd;
    const LUID m_originalAdapterLuid;
};
}
#endif