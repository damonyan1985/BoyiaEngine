#ifndef BoyiaAppWindow_h
#define BoyiaAppWindow_h

#include "BoyiaDC.h"
#include "window/BoyiaWindow.h"

namespace yanbo {

class BoyiaAppWindow : public BoyiaWindow
{
public:
    BoyiaAppWindow();
    virtual ~BoyiaAppWindow();

public:
    HICON m_hIcon;
    HCURSOR m_hCursor;
    virtual void OnPrepareDC(BoyiaDC *pDC);
    virtual void OnDraw(BoyiaDC *pDC);

protected:
    HMENU m_hMenu;
    BoyiaMsg DWORD OnCreate(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnPaint(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnLButtonDown(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnLButtonUp(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnSetCursor(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnKeyDown(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnRButtonDown(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnClose(WPARAM, LPARAM);
    MESSAGE_MAP_TABLE()
};

class BoyiaAppImpl : public BoyiaApp
{
public:
    virtual BOOL InitInstance(HINSTANCE hIns, int nCmdShow);
    BoyiaAppImpl();
    ~BoyiaAppImpl();
};

}
#endif