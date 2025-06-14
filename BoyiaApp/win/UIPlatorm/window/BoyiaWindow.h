#ifndef BoyiaWindow_h
#define BoyiaWindow_h

#include "BaseWindow.h"
#include "BoyiaDC.h"
#include "BoyiaOnLoadWin.h"

namespace yanbo {

class BoyiaWindow : public BaseWindow {
public:
    BoyiaWindow();
    virtual ~BoyiaWindow();

    virtual void OnPrepareDC(BoyiaDC* pDC);
    virtual void OnDraw(BoyiaDC* pDC);

protected:
    BoyiaMsg DWORD OnCreate(WPARAM, LPARAM);
    BoyiaMsg DWORD OnClose(WPARAM, LPARAM);
    BoyiaMsg DWORD OnDestroy(WPARAM, LPARAM);
    BoyiaMsg DWORD OnPaint(WPARAM, LPARAM);
    MESSAGE_MAP_TABLE()
};

using BoyiaUIEngine = BoyiaOnLoadWin;
class BoyiaApp {
public:
    BoyiaApp();
    ~BoyiaApp();

    void FreeWndPtr();
    virtual BOOL InitInstance(BoyiaUIEngine* engine, int nCmdShow);
    virtual BOOL Run();

    BoyiaUIEngine* GetEngine();
    static BoyiaApp* GetCurrApp();

protected:
    static BoyiaApp* m_pCurrApp;
    BoyiaWindow* m_window;
    BoyiaUIEngine* m_engine;
};

}

#endif