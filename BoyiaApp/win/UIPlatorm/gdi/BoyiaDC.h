#ifndef BoyiaDC_h
#define BoyiaDC_h

#include "window/BoyiaWindow.h"

namespace yanbo {

class BoyiaDC
{
public:
	BoyiaDC();
	virtual ~BoyiaDC();

public:
	BOOL DeleteDC();
	BOOL CreateCompatibleDC(BoyiaDC* pDC);
	COLORREF SetPixel(int x, int y, COLORREF crColor);
	BOOL Ellipse(int x1, int y1, int x2, int y2);
	BOOL Rectangle(int x1, int y1, int x2, int y2);
	int  SetROP2(int nDrawMode);
	BOOL TextOut(int, int, LPCWSTR, int);
	BOOL MoveTo(POINT &point);
	BOOL MoveTo(int x, int y);
	BOOL LineTo(POINT &point);
	BOOL LineTo(int x, int y);
	BOOL BitBlt(int nXDest, int nYDest, int nWidth, int nHeight, BoyiaDC *hdcSrc, int nXSrc, int nYSrc, DWORD dwRop);
	BOOL StretchBlt(int nXDest, int nYDest, int nWidth, int nHeight, BoyiaDC *hdcSrc, int nXSrc, int nYSrc, int nWidthSrc, int nHeightSrc, DWORD dwRop);
	HGDIOBJ SelectObject(HGDIOBJ hgdiobj);
	HDC  GetDCHandle();

protected:
	BaseWindow * m_ptWnd;
	HDC m_hDC;
};

class BoyiaClientDC : public BoyiaDC
{
public:
	BoyiaClientDC(BoyiaWindow *ptwnd);
	virtual ~BoyiaClientDC();
};

class BoyiaPaintDC : public BoyiaDC
{
public:
	BoyiaPaintDC(BoyiaWindow *ptwnd);
	virtual ~BoyiaPaintDC();
protected:
	PAINTSTRUCT m_ps;
};

class BoyiaWindowDC : public BoyiaDC
{
public:
	BoyiaWindowDC(BoyiaWindow *ptwnd);
	~BoyiaWindowDC();
};

}

#endif