#include "BoyiaAppWindow.h"
#include "BoyiaOnLoadWin.h"
#include "res/res.h"

#include <dbghelp.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>
#include <tchar.h>

namespace yanbo {

const int kTrayIconId = 1;

enum BoyiaTrayMenuIds {
    kBoyiaTrayMenuExit = 1
};

BEGIN_MAP_TABLE(BoyiaAppWindow, BoyiaWindow)
// msg mapping item begin
WM_CREATE_ITEN()
//WM_PAINT_ITEM()
WM_LBUTTONDOWN_ITEM()
WM_LBUTTONUP_ITEM()
WM_SETCURSOR_ITEM()
WM_KEYDOWN_ITEM()
WM_RBUTTONDOWN_ITEM()
WM_CLOSE_ITEM()
WM_TRAY_NOTIFY_ITEM()
WM_COMMAND_ITEM(kBoyiaTrayMenuExit, OnMenuItemExit)
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
    Shell_NotifyIcon(NIM_DELETE, &m_nid);
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

    CreateAppTray();
    return FALSE;
}

DWORD BoyiaAppWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    int mouseX = (int)LOWORD(lParam);
    int mouseY = (int)HIWORD(lParam);

    if (GetEngine())
    {
        GetEngine()->handleTouchEvent(0, mouseX, mouseY);
    }
    return FALSE;
}

DWORD BoyiaAppWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    int mouseX = (int)LOWORD(lParam);
    int mouseY = (int)HIWORD(lParam);

    if (GetEngine())
    {
        GetEngine()->handleTouchEvent(1, mouseX, mouseY);
    }
    
    return FALSE;
}

void BoyiaAppWindow::OnDraw(BoyiaDC* pDC)
{
    if (GetEngine())
    {
        GetEngine()->repaint();
    }
}

DWORD BoyiaAppWindow::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    SetCursor(m_hCursor);
    return FALSE;
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

    return FALSE;
}

DWORD BoyiaAppWindow::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

DWORD BoyiaAppWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
    if (GetEngine())
    {
        GetEngine()->cacheCode();
    }
    // return BoyiaWindow::OnClose(wParam, lParam);
    ShowWindow(SW_HIDE);
    UpdateWindow();
    return FALSE;
}

DWORD BoyiaAppWindow::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
    if (lParam == WM_LBUTTONUP) {
        ShowWindow(SW_NORMAL);
    } else if (lParam == WM_RBUTTONUP) {
        HMENU hMenu = ::CreatePopupMenu();
        ::AppendMenu(hMenu, MF_STRING, kBoyiaTrayMenuExit, _T("Exit"));
        
        POINT pt;
        ::GetCursorPos(&pt);

        ::TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
        ::DestroyMenu(hMenu);
    }
    return FALSE;
}

DWORD BoyiaAppWindow::OnMenuItemExit(WPARAM wParam, LPARAM lParam)
{
    if (wParam == kBoyiaTrayMenuExit) {
        PostQuitMessage(0);
    }
    return FALSE;
}

HICON BoyiaAppWindow::GetWindowIcon()
{
    if (!m_hIcon) {
        m_hIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_BOYIA_APP_MAIN_ICON));
    }

    return m_hIcon;
}

void BoyiaAppWindow::CreateAppTray()
{
    
    ZeroMemory(&m_nid, sizeof(NOTIFYICONDATA));
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hWnd;
    m_nid.uID = kTrayIconId;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.hIcon = GetWindowIcon();
    m_nid.uCallbackMessage = WM_TRAY_NOTIFICATION;
    _tcscpy_s(m_nid.szTip, _T("BoyiaApp"));

    Shell_NotifyIcon(NIM_ADD, &m_nid);
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