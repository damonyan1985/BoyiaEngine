#include "BoyiaAppWindow.h"
#include "BoyiaOnLoadWin.h"
#include "res/res.h"

#include <dbghelp.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>

namespace yanbo {

BEGIN_MAP_TABLE(BoyiaAppWindow, BoyiaWindow)
// msg mapping item begin
WM_CREATE_ITEN()
WM_PAINT_ITEM()
WM_LBUTTONDOWN_ITEM()
WM_LBUTTONUP_ITEM()
WM_SETCURSOR_ITEM()
WM_KEYDOWN_ITEM()
WM_RBUTTONDOWN_ITEM()
WM_CLOSE_ITEM()
// msg mapping item end
END_MAP_TABLE()


BoyiaUIEngine* GetEngine()
{
    if (!BoyiaApp::GetCurrApp()) {
        return nullptr;
    }
    return BoyiaApp::GetCurrApp()->GetEngine();
}

BoyiaAppWindow::BoyiaAppWindow()
    : m_hIcon(NULL)
    , m_hMenu(NULL)
{
    m_hCursor = ::LoadCursor(NULL, IDC_ARROW);
}

BoyiaAppWindow::~BoyiaAppWindow()
{
}

DWORD BoyiaAppWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
    RECT rect;
    GetClientRect(&rect);
    if (GetEngine())
    {
        GetEngine()->setContextWin(m_hWnd,
            rect.right - rect.left,
            rect.bottom - rect.top);
        GetEngine()->connectServer();
    }
    /*
    HWND hWnd = m_hWnd;
    HINSTANCE hins = (HINSTANCE)::GetWindowLong(hWnd, GWL_HINSTANCE);
    m_hMenu = ::LoadMenu(hins, MAKEINTRESOURCE(IDR_APP_MENU));
    ::SetMenu(hWnd, m_hMenu);
    m_hIcon = ::LoadIcon(hins, MAKEINTRESOURCE(IDI_APP));
    ::SetClassLong(m_hWnd, GCL_HICON, (LONG)m_hIcon);
    */
    // 设置窗口左上角图标和任务栏图标
    HICON hIcon = GetWindowIcon();
    ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    //to do any other logic

    return 0;
}

DWORD BoyiaAppWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    int mouseX = (int)LOWORD(lParam);
    int mouseY = (int)HIWORD(lParam);

    if (GetEngine())
    {
        GetEngine()->handleTouchEvent(0, mouseX, mouseY);
    }
    return 0;
}

DWORD BoyiaAppWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    int mouseX = (int)LOWORD(lParam);
    int mouseY = (int)HIWORD(lParam);

    if (GetEngine())
    {
        GetEngine()->handleTouchEvent(1, mouseX, mouseY);
    }
    
    return 0;
}

DWORD BoyiaAppWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
    BoyiaPaintDC dc(this);
    OnPrepareDC(&dc);
    OnDraw(&dc);

    if (GetEngine())
    {
        GetEngine()->repaint();
    }
    
    return 0;
}

void BoyiaAppWindow::OnDraw(BoyiaDC* pDC)
{
}

DWORD BoyiaAppWindow::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    SetCursor(m_hCursor);
    return 0;
}

void BoyiaAppWindow::OnPrepareDC(BoyiaDC* pDC)
{
}

DWORD BoyiaAppWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
    case VK_UP:
        break;
    case VK_DOWN:
        break;
    case VK_LEFT:
        break;
    case VK_RIGHT:
        break;
    case VK_SPACE:
        break;
    }

    return 0;
}

DWORD BoyiaAppWindow::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

DWORD BoyiaAppWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
    if (GetEngine())
    {
        GetEngine()->cacheCode();
    }
    return BoyiaWindow::OnClose(wParam, lParam);
}

HICON BoyiaAppWindow::GetWindowIcon()
{
    if (!m_hIcon) {
        m_hIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_BOYIA_APP_MAIN_ICON));
    }

    return m_hIcon;
}

BoyiaAppImpl::BoyiaAppImpl()
{
    InitCrashHandler();
}

BoyiaAppImpl::~BoyiaAppImpl()
{
}

BOOL BoyiaAppImpl::InitInstance(BoyiaUIEngine* engine, int nCmdShow)
{
    m_engine = engine;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
    m_window = new BoyiaAppWindow();
    m_window->InitBaseWindow();
    m_window->CreateBaseWindow(L"Boyia", L"BoyiaApp Simulator", dwStyle, 0, 0, 360, 640, NULL);
    m_window->ShowWindow(nCmdShow);
    m_window->UpdateWindow();

    return TRUE;
}

// Exception handle for windows native program
VOID BoyiaAppImpl::InitCrashHandler()
{
    // Crash handler Win32 API
    ::SetUnhandledExceptionFilter(BoyiaUnhandledExceptionFilter);
    // (CRT) Exception
    _set_purecall_handler(BoyiaPureCallHandler);
    _set_new_handler(BoyiaNewHandler);
    _set_invalid_parameter_handler(BoyiaInvalidParameterHandler);
    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
    
    signal(SIGABRT, BoyiaSigabrtHandler);
    signal(SIGINT, BoyiaSigintHandler);
    signal(SIGTERM, BoyiaSigtermHandler);
    signal(SIGILL, BoyiaSigillHandler);
    // C++ Exception
    set_terminate(BoyiaTerminateHandler);
    set_unexpected(BoyiaUnexpectedHandler);
}

LONG BoyiaAppImpl::BoyiaUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
    return EXCEPTION_EXECUTE_HANDLER;
}

VOID BoyiaAppImpl::BoyiaPureCallHandler()
{

}

int BoyiaAppImpl::BoyiaNewHandler(size_t size)
{
    return 0;
}

void BoyiaAppImpl::BoyiaInvalidParameterHandler(wchar_t const* expression,
    wchar_t const* function,
    wchar_t const* file,
    unsigned int line,
    uintptr_t pReserved)
{

}

void BoyiaAppImpl::BoyiaSigabrtHandler(int signal)
{

}

void BoyiaAppImpl::BoyiaSigintHandler(int signal)
{

}

void BoyiaAppImpl::BoyiaSigtermHandler(int signal)
{

}


void BoyiaAppImpl::BoyiaSigillHandler(int signal)
{

}

void BoyiaAppImpl::BoyiaTerminateHandler()
{

}

void BoyiaAppImpl::BoyiaUnexpectedHandler()
{

}

}