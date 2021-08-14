#include "BoyiaAppWindow.h"
#include "BoyiaOnLoadWin.h"

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

BoyiaAppWindow::BoyiaAppWindow()
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
    BoyiaOnLoadWin::setContextWin(m_hWnd,
        rect.right - rect.left,
        rect.bottom - rect.top);
    BoyiaOnLoadWin::connectServer();
    /*
    HWND hWnd = m_hWnd;
    HINSTANCE hins = (HINSTANCE)::GetWindowLong(hWnd, GWL_HINSTANCE);
    m_hMenu = ::LoadMenu(hins, MAKEINTRESOURCE(IDR_APP_MENU));
    ::SetMenu(hWnd, m_hMenu);
    m_hIcon = ::LoadIcon(hins, MAKEINTRESOURCE(IDI_APP));
    ::SetClassLong(m_hWnd, GCL_HICON, (LONG)m_hIcon);
    */
    //to do any other logic

    return 0;
}

DWORD BoyiaAppWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    int mouseX = (int)LOWORD(lParam);
    int mouseY = (int)HIWORD(lParam);

    BoyiaOnLoadWin::handleTouchEvent(0, mouseX, mouseY);
    return 0;
}

DWORD BoyiaAppWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    int mouseX = (int)LOWORD(lParam);
    int mouseY = (int)HIWORD(lParam);

    BoyiaOnLoadWin::handleTouchEvent(1, mouseX, mouseY);
    return 0;
}

DWORD BoyiaAppWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
    BoyiaPaintDC dc(this);
    OnPrepareDC(&dc);
    OnDraw(&dc);

    BoyiaOnLoadWin::repaint();
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
    BoyiaOnLoadWin::cacheCode();
    return BoyiaWindow::OnClose(wParam, lParam);
}

BoyiaAppImpl::BoyiaAppImpl()
{
}

BoyiaAppImpl theApp;

BoyiaAppImpl::~BoyiaAppImpl()
{
}

BOOL BoyiaAppImpl::InitInstance(HINSTANCE hIns, int nCmdShow)
{
    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
    m_window = new BoyiaAppWindow;
    m_window->InitBaseWindow(hIns);
    m_window->CreateBaseWindow(L"Boyia", L"BoyiaApp Simulator", dwStyle, 0, 0, 360, 640, NULL);
    m_window->ShowWindow(nCmdShow);
    m_window->UpdateWindow();
    return TRUE;
}

}