#include "BoyiaWindow.h"
#include <string.h>

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
	_ptwin = new BoyiaWindow;
	_ptwin->InitBaseWindow(hIns);
	_ptwin->CreateBaseWindow(L"Boyia", L"BoyiaWindow", dwStyle, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL);
	_ptwin->ShowTW(nCmdShow);
	_ptwin->UpdateTW();
	return TRUE;
}

BOOL BoyiaApp::run()
{
	_ptwin->MessageLoop();
	return TRUE;
}

BoyiaApp* BoyiaApp::GetCurrApp()
{
	return m_pCurrApp;
}

void BoyiaApp::FreeWndPtr()
{
	if (_ptwin)
	{
		delete _ptwin;
		_ptwin = NULL;
	}
}

extern BoyiaApp *tfxGetApp()
{
	return BoyiaApp::GetCurrApp();
}

