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
    BOOL m_bGameEnd;
    BoyiaDC m_memDrawDC;
    BoyiaDC m_memSurfaceDC;
    BOOL m_bFirstShow;
    HICON m_hIcon;
    BOOL m_bGameStart;
    HCURSOR m_hCursor;
    virtual void OnPrepareDC(BoyiaDC *pDC);
    virtual void OnDraw(BoyiaDC *pDC);
    virtual void ProcessTick();

protected:
    HMENU m_hMenu;
    BoyiaMsg DWORD OnCreate(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnPaint(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnLButtonDown(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnLButtonUp(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnSetCursor(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnKeyDown(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnGameStart(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnGamePause(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnGameEnd(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnGameAbout(WPARAM wParam, LPARAM lParam);
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