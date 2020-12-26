#include "BoyiaWindow.h"
#include <string.h>

namespace yanbo {

BEGIN_MAP_TABLE(BoyiaWindow, BaseWindow)
// msg mapping item begin
WM_CREATE_ITEN()
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


//app
BoyiaApp* BoyiaApp::m_pCurrApp = NULL;

BoyiaApp::BoyiaApp()
{
	m_pCurrApp = (BoyiaApp*)this;
}

BoyiaApp::~BoyiaApp()
{
	FreeWndPtr();
}

BOOL BoyiaApp::InitInstance(HINSTANCE hIns, int nCmdShow)
{
	DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
    m_window = new BoyiaWindow;
    m_window->InitBaseWindow(hIns);
    m_window->CreateBaseWindow(L"Boyia", L"BoyiaWindow", dwStyle, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL);
    m_window->ShowWindow(nCmdShow);
    m_window->UpdateWindow();
	return TRUE;
}

BOOL BoyiaApp::run()
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
	if (m_window)
	{
		delete m_window;
        m_window = NULL;
	}
}

}

extern yanbo::BoyiaApp* tfxGetApp()
{
	return yanbo::BoyiaApp::GetCurrApp();
}

