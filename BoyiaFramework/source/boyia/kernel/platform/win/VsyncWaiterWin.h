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
    LInt64 tryGetVsyncIntervalFromDisplayConfig();
    LInt64 tryGetVSyncParamsFromDwmCompInfo();
    LInt64 tryGetVSyncIntervalFromDisplaySettings();

    LInt64 getVSyncIntervalIfAvailable();
    const ComPtr<ID3D11Device> m_dxgiDevice;
    ComPtr<IDXGIOutput> m_primaryOutput;
    HMONITOR m_primaryMonitor;
    ComPtr<IDXGIAdapter> m_dxgiAdapter;
    HWND m_hwnd;
    const LUID m_originalAdapterLuid;
};
}
#endif