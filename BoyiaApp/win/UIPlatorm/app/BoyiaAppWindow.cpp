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
// msg mapping item end
END_MAP_TABLE()

BoyiaAppWindow::BoyiaAppWindow()
{
    m_hCursor = ::LoadCursor(NULL, IDC_ARROW);
    m_bGameStart = FALSE;
    m_bGameEnd = FALSE;
    m_bFirstShow = FALSE;
}

BoyiaAppWindow::~BoyiaAppWindow()
{
}

DWORD BoyiaAppWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
    BoyiaOnLoadWin::setContextWin(m_hWnd, 360, 640);
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
    return 0;
}

DWORD BoyiaAppWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
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

void BoyiaAppWindow::OnDraw(BoyiaDC *pDC)
{
    RECT rect;
    GetClientRect(&rect);
    pDC->BitBlt(0, 0, rect.right - rect.left, rect.bottom - rect.top, &m_memSurfaceDC, 0, 0, SRCCOPY);
}

DWORD BoyiaAppWindow::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    SetCursor(m_hCursor);
    return 0;
}

void BoyiaAppWindow::OnPrepareDC(BoyiaDC *pDC)
{
}

DWORD BoyiaAppWindow::OnGameStart(WPARAM wParam, LPARAM lParam)
{
    m_bGameStart = TRUE;
    m_bFirstShow = TRUE;

    //Invalidate();
    return 0;
}

DWORD BoyiaAppWindow::OnGameEnd(WPARAM wParam, LPARAM lParam)
{
    m_bGameEnd = TRUE;
    return 0;
}

DWORD BoyiaAppWindow::OnGamePause(WPARAM wParam, LPARAM lParam)
{

	return 0;
}

DWORD BoyiaAppWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
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

DWORD BoyiaAppWindow::OnGameAbout(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

DWORD BoyiaAppWindow::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void BoyiaAppWindow::ProcessTick()
{
	BoyiaClientDC dc(this);
	RECT rect;
	GetClientRect(&rect);

	if (m_bGameStart)
	{
		if (!m_memSurfaceDC.GetDCHandle())
		{
			m_memSurfaceDC.CreateCompatibleDC(&dc);
			m_memDrawDC.CreateCompatibleDC(&dc);
		}

		HBITMAP hSurface;
		hSurface = ::CreateCompatibleBitmap(dc.GetDCHandle(), rect.right - rect.left, rect.bottom - rect.top);
		m_memSurfaceDC.SelectObject((HGDIOBJ)hSurface);
		m_memSurfaceDC.BitBlt(0, 0, rect.right - rect.left, rect.bottom - rect.top, &dc, 0, 0, WHITENESS);


		//for enemy
		dc.BitBlt(0, 0, rect.right - rect.left, rect.bottom - rect.top, &m_memSurfaceDC, 0, 0, SRCCOPY);
		::DeleteObject(hSurface);

	}

	if (m_bGameEnd)
	{
		m_memSurfaceDC.DeleteDC();
		m_memSurfaceDC.DeleteDC();
	}
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
    _ptwin = new BoyiaAppWindow;
    _ptwin->InitBaseWindow(hIns);
    _ptwin->CreateBaseWindow(L"Boyia", L"BoyiaWindow", dwStyle, 0, 0, 360, 640, NULL);
    _ptwin->ShowTW(nCmdShow);
    _ptwin->UpdateTW();
    return TRUE;
}

}