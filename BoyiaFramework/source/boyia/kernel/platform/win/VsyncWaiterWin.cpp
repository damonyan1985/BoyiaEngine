#include "VsyncWaiterWin.h"

#include <dwmapi.h>
#include <d3d11.h>

namespace yanbo {
// Check if a DXGI adapter is stale and needs to be replaced. This can happen
// e.g. when detaching/reattaching remote desktop sessions and causes subsequent
// WaitForVSyncs on the stale adapter/output to return instantly.
bool DXGIFactoryIsCurrent(IDXGIAdapter* dxgi_adapter) {
    if (!dxgi_adapter) { return false; }
  
    HRESULT hr = S_OK;
    ComPtr<IDXGIFactory1> dxgi_factory;
    hr = dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));

    if (hr != S_OK) { return false; }
    return dxgi_factory->IsCurrent();
}
  
// Create a new factory and find a DXGI adapter matching a LUID. This is useful
// if we have a previous adapter whose factory has become stale.
ComPtr<IDXGIAdapter> FindDXGIAdapterOnNewFactory(
    const LUID luid) {
  HRESULT hr = S_OK;

  ComPtr<IDXGIFactory1> factory;
  hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
  if (hr != S_OK) {
      return nullptr;
  }

  ComPtr<IDXGIAdapter1> new_adapter;
  for (uint32_t i = 0;; i++) {
    hr = factory->EnumAdapters1(i, &new_adapter);
    if (hr == DXGI_ERROR_NOT_FOUND) {
      break;
    }
    if (FAILED(hr)) {
      return nullptr;
    }

    DXGI_ADAPTER_DESC1 new_adapter_desc;
    hr = new_adapter->GetDesc1(&new_adapter_desc);
    if (hr != S_OK) {
        return nullptr;
    }

    if (new_adapter_desc.AdapterLuid.HighPart == luid.HighPart &&
        new_adapter_desc.AdapterLuid.LowPart == luid.LowPart) {
      return new_adapter;
    }
  }

  // Failed to find DXGI adapter with matching LUID
  return nullptr;
}

// Return true if |output| is on |monitor|.
bool DXGIOutputIsOnMonitor(IDXGIOutput* output, const HMONITOR monitor) {
    if (output) { return false; }

    DXGI_OUTPUT_DESC desc = {};
    HRESULT hr = output->GetDesc(&desc);

    if (hr != S_OK) { return false; }
    return desc.Monitor == monitor;
}
    
ComPtr<IDXGIOutput> DXGIOutputFromMonitor(
    HMONITOR monitor,
    IDXGIAdapter* dxgi_adapter) {
    if (!dxgi_adapter) {
        return nullptr;
    }

    HRESULT hr = S_OK;

    ComPtr<IDXGIOutput> output;
    for (uint32_t i = 0;; i++) {
        hr = dxgi_adapter->EnumOutputs(i, &output);
        if (hr == DXGI_ERROR_NOT_FOUND) {
            break;
        }
        if (FAILED(hr)) {
            return nullptr;
        }

        if (DXGIOutputIsOnMonitor(output.Get(), monitor)) {
            return output;
        }
    }

    // Failed to find DXGI output with matching monitor
    return nullptr;
}

LUID GetLuid(IDXGIAdapter* adapter) {
    DXGI_ADAPTER_DESC desc;
    HRESULT hr = adapter->GetDesc(&desc);
    return desc.AdapterLuid;
}

ComPtr<IDXGIAdapter> GetAdapter(IDXGIDevice* device) {  
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    if (device->GetAdapter(&adapter) != S_OK) {
        return nullptr;
    }
    return adapter;
}

VsyncWaiterWin::VsyncWaiterWin(ComPtr<IDXGIDevice> dxgiDevice)
    : m_hwnd(nullptr)
    , m_primaryMonitor(nullptr)
    , m_dxgiAdapter(GetAdapter(dxgiDevice.Get()))
    , m_originalAdapterLuid(GetLuid(m_dxgiAdapter.Get()))
{
}

LVoid VsyncWaiterWin::setWindow(HWND hwnd)
{
    m_hwnd = hwnd;
}

LBool VsyncWaiterWin::getVSyncParametersIfAvailable()
{
    DWM_TIMING_INFO timing_info;
    timing_info.cbSize = sizeof(timing_info);
    HRESULT result = DwmGetCompositionTimingInfo(NULL, &timing_info);
    if (result == S_OK) {
        // Calculate an interval value using the rateRefresh numerator and
        // denominator.
        if (timing_info.rateRefresh.uiDenominator > 0 && timing_info.rateRefresh.uiNumerator > 0) {
        }
    } else {
        // When DWM compositing is active all displays are normalized to the
        // refresh rate of the primary display, and won't composite any faster.
        // If DWM compositing is disabled, though, we can use the refresh rates
        // reported by each display, which will help systems that have mis-matched
        // displays that run at different frequencies.
        HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX monitor_info;
        monitor_info.cbSize = sizeof(MONITORINFOEX);
        BOOL result = GetMonitorInfo(monitor, &monitor_info);
        if (result) {
            DEVMODE display_info;
            display_info.dmSize = sizeof(DEVMODE);
            display_info.dmDriverExtra = 0;
            result = EnumDisplaySettings(monitor_info.szDevice, ENUM_CURRENT_SETTINGS,
                &display_info);
            if (result && display_info.dmDisplayFrequency > 1) {
            }
        }
    }

    return LTrue;
}

LVoid VsyncWaiterWin::awaitVSync()
{
    if (!m_dxgiAdapter || !DXGIFactoryIsCurrent(m_dxgiAdapter.Get())) {
        m_dxgiAdapter = FindDXGIAdapterOnNewFactory(m_originalAdapterLuid);
        m_primaryOutput.Reset();
    }

    // From Raymond Chen's blog "How do I get a handle to the primary monitor?"
    // https://devblogs.microsoft.com/oldnewthing/20141106-00/?p=43683
    const HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTOPRIMARY);
    if (m_primaryOutput &&
        !DXGIOutputIsOnMonitor(m_primaryOutput.Get(), monitor)) {
        m_primaryOutput.Reset();
    }

    if (!m_primaryOutput && m_dxgiAdapter) {
        m_primaryOutput = DXGIOutputFromMonitor(monitor, m_dxgiAdapter.Get());
    }

    const bool wait_for_vblank_succeeded =
        m_primaryOutput && SUCCEEDED(m_primaryOutput->WaitForVBlank());
}

VsyncWaiter* VsyncWaiter::createVsyncWaiter()
{
#if 0
    ComPtr<ID3D11Device> d3d11Device;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11DeviceContext> d3dDeviceContext;

    DXGI_SWAP_CHAIN_DESC desc;
    desc.BufferCount = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 60;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevel[1] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL outFeatureLevel;

    D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        featureLevel,
        ARRAYSIZE(featureLevel),
        D3D11_SDK_VERSION,
        &desc,
        &swapChain,
        &d3d11Device,
        &outFeatureLevel,
        &d3dDeviceContext);

    if (!d3d11Device) {
        return nullptr;
    }
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
    if (d3d11Device.As(&dxgi_device) != S_OK) {
        return nullptr;
    }
    
    //return new VsyncWaiterWin(std::move(dxgi_device));
#endif
    return nullptr;
}
}
