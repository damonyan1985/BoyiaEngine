#ifndef BoyiaWindow_h
#define BoyiaWindow_h

#include "BaseWindow.h"
#include "BoyiaOnLoadWin.h"

namespace yanbo {

class BoyiaWindow : public BaseWindow {
public:
    BoyiaWindow();
    virtual ~BoyiaWindow();

protected:
    BoyiaMsg DWORD OnCreate(WPARAM, LPARAM);
    BoyiaMsg DWORD OnClose(WPARAM, LPARAM);
    BoyiaMsg DWORD OnDestroy(WPARAM, LPARAM);
    MESSAGE_MAP_TABLE()
};

using BoyiaUIEngine = BoyiaOnLoadWin;
class BoyiaApp {
public:
    BoyiaApp();
    ~BoyiaApp();

    void FreeWndPtr();
    virtual BOOL InitInstance(BoyiaUIEngine* engine, HINSTANCE inst, int nCmdShow);
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