#include "BoyiaWindow.h"
#include <string.h>

namespace yanbo {

BEGIN_MAP_TABLE(BoyiaWindow, BaseWindow)
// msg mapping item begin
WM_CREATE_ITEN()
WM_PAINT_ITEM()
WM_CLOSE_ITEM()
WM_DESTROY_ITEM()
// msg mapping item end
END_MAP_TABLE()

BoyiaWindow::BoyiaWindow()
{
}

BoyiaWindow::~BoyiaWindow()
{
}

DWORD BoyiaWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
    return BaseWindow::OnClose(wParam, lParam);
}

DWORD BoyiaWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    return BaseWindow::OnDestroy(wParam, lParam);
}

DWORD BoyiaWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

void BoyiaWindow::OnPrepareDC(BoyiaDC* pDC)
{
}

void BoyiaWindow::OnDraw(BoyiaDC* pDC)
{
}

DWORD BoyiaWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
    BoyiaPaintDC dc(this);
    OnPrepareDC(&dc);
    OnDraw(&dc);

    return FALSE;
}

//app
BoyiaApp* BoyiaApp::m_pCurrApp = nullptr;

BoyiaApp::BoyiaApp()
    : m_engine(nullptr)
{
    m_pCurrApp = (BoyiaApp*)this;
}

BoyiaApp::~BoyiaApp()
{
    FreeWndPtr();
    m_engine = nullptr;
    m_pCurrApp = nullptr;
}

BOOL BoyiaApp::InitInstance(BoyiaUIEngine* engine, int nCmdShow)
{
    m_engine = engine;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
    m_window = new BoyiaWindow();
    m_window->InitBaseWindow();
    m_window->CreateBaseWindow(L"Boyia", L"BoyiaWindow", dwStyle, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL);
    m_window->ShowWindow(nCmdShow);
    m_window->UpdateWindow();
    return TRUE;
}

BoyiaUIEngine* BoyiaApp::GetEngine()
{
    return m_engine;
}

BOOL BoyiaApp::Run()
{
    m_window->MessageLoop();
    return TRUE;
}

BoyiaApp* BoyiaApp::GetCurrApp()
{
    return m_pCurrApp;
}

void BoyiaApp::FreeWndPtr()
{
    if (m_window) {
        delete m_window;
        m_window = nullptr;
    }
}

}
