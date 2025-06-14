#ifndef BoyiaAppWindow_h
#define BoyiaAppWindow_h

#include "BoyiaDC.h"
#include "window/BoyiaWindow.h"

namespace yanbo {

class BoyiaAppWindow : public BoyiaWindow {
public:
    BoyiaAppWindow();
    virtual ~BoyiaAppWindow();
    
    virtual void OnPrepareDC(BoyiaDC* pDC);
    virtual void OnDraw(BoyiaDC* pDC);

    HICON GetWindowIcon() override;
    void CreateAppTray();

protected:
    HMENU m_hMenu;
    HICON m_hIcon;
    HCURSOR m_hCursor;
    
    BoyiaMsg DWORD OnCreate(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnPaint(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnLButtonDown(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnLButtonUp(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnSetCursor(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnKeyDown(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnRButtonDown(WPARAM wParam, LPARAM lParam);
    BoyiaMsg DWORD OnClose(WPARAM, LPARAM);
    BoyiaMsg DWORD OnTrayNotification(WPARAM, LPARAM);
    MESSAGE_MAP_TABLE()
};

class BoyiaAppImpl : public BoyiaApp {
public:
    BOOL InitInstance(BoyiaUIEngine* engine, int nCmdShow) override;
    VOID InitCrashHandler();
    BoyiaAppImpl();
    ~BoyiaAppImpl();

private:
    static LONG BoyiaUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo);
    static VOID BoyiaPureCallHandler();
    static int BoyiaNewHandler(size_t size);
    static void BoyiaInvalidParameterHandler(wchar_t const* expression,
        wchar_t const* function,
        wchar_t const* file,
        unsigned int line,
        uintptr_t pReserved);

    static void BoyiaSigabrtHandler(int signal);
    static void BoyiaSigintHandler(int signal);
    static void BoyiaSigtermHandler(int signal);
    static void BoyiaSigillHandler(int signal);
    static void BoyiaTerminateHandler();
    static void BoyiaUnexpectedHandler();
};

#define WM_TRAY_NOTIFICATION (WM_USER + 1)
#define WM_TRAY_NOTIFY_ITEM() { WM_TRAY_NOTIFICATION, 0, (BaseMsgFunc)&OnTrayNotification },

}
#endif