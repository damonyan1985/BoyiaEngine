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
    NOTIFYICONDATA m_nid;
    
    BoyiaMsg DWORD OnCreate(WPARAM, LPARAM);
    BoyiaMsg DWORD OnPaint(WPARAM, LPARAM);
    BoyiaMsg DWORD OnLButtonDown(WPARAM, LPARAM);
    BoyiaMsg DWORD OnLButtonUp(WPARAM, LPARAM);
    BoyiaMsg DWORD OnSetCursor(WPARAM, LPARAM);
    BoyiaMsg DWORD OnKeyDown(WPARAM, LPARAM);
    BoyiaMsg DWORD OnRButtonDown(WPARAM, LPARAM);
    BoyiaMsg DWORD OnClose(WPARAM, LPARAM);
    BoyiaMsg DWORD OnTrayNotification(WPARAM, LPARAM);
    BoyiaMsg DWORD OnMenuItemExit(WPARAM, LPARAM);
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