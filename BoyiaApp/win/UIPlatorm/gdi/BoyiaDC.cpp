#include "BoyiaDC.h"

namespace yanbo {

BoyiaDC::BoyiaDC()
{
    m_hDC = NULL;
    m_ptWnd = NULL;
}

BoyiaDC::~BoyiaDC()
{
}

BOOL BoyiaDC::MoveTo(POINT& point)
{
    return ::MoveToEx(m_hDC, point.x, point.y, NULL);
}

BOOL BoyiaDC::MoveTo(int x, int y)
{
    return ::MoveToEx(m_hDC, x, y, NULL);
}

BOOL BoyiaDC::LineTo(POINT& point)
{
    return ::LineTo(m_hDC, point.x, point.y);
}

BOOL BoyiaDC::LineTo(int x, int y)
{
    return ::LineTo(m_hDC, x, y);
}

int BoyiaDC::SetROP2(int nDrawMode)
{
    return ::SetROP2(m_hDC, nDrawMode);
}

BOOL BoyiaDC::Rectangle(int x1, int y1, int x2, int y2)
{
    return ::Rectangle(m_hDC, x1, y1, x2, y2);
}

BOOL BoyiaDC::Ellipse(int x1, int y1, int x2, int y2)
{
    return ::Ellipse(m_hDC, x1, y1, x2, y2);
}

COLORREF BoyiaDC::SetPixel(int x, int y, COLORREF crColor)
{
    return ::SetPixel(m_hDC, x, y, crColor);
}

BOOL BoyiaDC::TextOut(int x, int y, LPCWSTR lpszString, int nCount)
{
    return ::TextOut(m_hDC, x, y, lpszString, nCount);
}

BOOL BoyiaDC::BitBlt(int nXDest, int nYDest, int nWidth, int nHeight, BoyiaDC* hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
    return ::BitBlt(m_hDC, nXDest, nYDest, nWidth, nHeight, hdcSrc->m_hDC, nXSrc, nYSrc, dwRop);
}

BOOL BoyiaDC::CreateCompatibleDC(BoyiaDC* pDC)
{
    m_ptWnd = pDC->m_ptWnd;
    m_hDC = ::CreateCompatibleDC(pDC->m_hDC);
    return m_hDC ? TRUE : FALSE;
}

BOOL BoyiaDC::StretchBlt(int nXDest, int nYDest, int nWidth, int nHeight, BoyiaDC* hdcSrc, int nXSrc, int nYSrc, int nWidthSrc, int nHeightSrc, DWORD dwRop)
{
    return ::StretchBlt(m_hDC, nXDest, nYDest, nWidth, nHeight, hdcSrc->m_hDC, nXSrc, nYSrc, nWidthSrc, nHeightSrc, dwRop);
}

HGDIOBJ BoyiaDC::SelectObject(HGDIOBJ hgdiobj)
{
    return ::SelectObject(m_hDC, hgdiobj);
}

BOOL BoyiaDC::DeleteDC()
{
    return ::DeleteDC(m_hDC);
}

HDC BoyiaDC::GetDCHandle()
{
    return m_hDC;
}

BoyiaClientDC::BoyiaClientDC(BoyiaWindow* ptwnd)
{
    m_ptWnd = ptwnd;
    m_hDC = ::GetDC(m_ptWnd->GetWndHandle());
}

BoyiaClientDC::~BoyiaClientDC()
{
    ::ReleaseDC(m_ptWnd->GetWndHandle(), m_hDC);
}

BoyiaPaintDC::BoyiaPaintDC(BoyiaWindow* ptwnd)
{
    m_ptWnd = ptwnd;
    m_hDC = ::BeginPaint(m_ptWnd->GetWndHandle(), &m_ps);
}

BoyiaPaintDC::~BoyiaPaintDC()
{
    ::EndPaint(m_ptWnd->GetWndHandle(), &m_ps);
}

BoyiaWindowDC::BoyiaWindowDC(BoyiaWindow* ptwnd)
{
    m_ptWnd = ptwnd;
    m_hDC = ::GetWindowDC(m_ptWnd->GetWndHandle());
}

BoyiaWindowDC::~BoyiaWindowDC()
{
    ::ReleaseDC(m_ptWnd->GetWndHandle(), m_hDC);
}

}