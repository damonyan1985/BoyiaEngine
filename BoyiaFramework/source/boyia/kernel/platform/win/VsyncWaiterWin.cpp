#include "VsyncWaiterWin.h"

#include <dwmapi.h>

namespace yanbo {
VsyncWaiterWin::VsyncWaiterWin()
    : m_hwnd(nullptr)
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
        if (timing_info.rateRefresh.uiDenominator > 0 &&
            timing_info.rateRefresh.uiNumerator > 0) {
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
    const HMONITOR monitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
}

VsyncWaiter* VsyncWaiter::createVsyncWaiter()
{
    return new VsyncWaiterWin();
}
}
