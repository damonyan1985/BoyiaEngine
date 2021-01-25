#ifndef BaseWindow_h
#define BaseWindow_h

#include <list>
#include <windows.h>

using namespace std;

namespace yanbo {
class BaseWindow;

#define BoyiaWinMain wWinMain
#define BoyiaWinApi WINAPI

typedef DWORD (BaseWindow::*BaseMsgFunc)(WPARAM, LPARAM);

typedef struct
{
    INT_PTR message;
    DWORD id;
    BaseMsgFunc _tfunc;
} CommandMessageItem;

#define MESSAGE_MAP_TABLE()                                                          \
public:                                                                              \
    virtual const CommandMessageItem* GetMessageEntries() { return messageEntries; } \
    static const CommandMessageItem messageEntries[];

#define BEGIN_MAP_TABLE(thisClass, baseClass)                \
    const CommandMessageItem thisClass::messageEntries[] = { \
        { (INT_PTR)(baseClass::messageEntries), 0, 0 },

#define END_MAP_TABLE() \
    {                   \
        0, 0, 0         \
    }                   \
    }                   \
    ;

#define BoyiaMsg

#define WM_COMMAND_ITEM(id, func) { WM_COMMAND, id, (BaseMsgFunc)&func },
#define WM_CREATE_ITEN() { WM_CREATE, 0, (BaseMsgFunc)&OnCreate },
#define WM_CLOSE_ITEM() { WM_CLOSE, 0, (BaseMsgFunc)&OnClose },
#define WM_DESTROY_ITEM() { WM_DESTROY, 0, (BaseMsgFunc)&OnDestroy },
#define WM_PAINT_ITEM() { WM_PAINT, 0, (BaseMsgFunc)&OnPaint },
#define WM_LBUTTONDOWN_ITEM() { WM_LBUTTONDOWN, 0, (BaseMsgFunc)&OnLButtonDown },
#define WM_LBUTTONUP_ITEM() { WM_LBUTTONUP, 0, (BaseMsgFunc)&OnLButtonUp },
#define WM_CHAR_ITEM() { WM_CHAR, 0, (BaseMsgFunc)&OnChar },
#define WM_KEYDOWN_ITEM() { WM_KEYDOWN, 0, (BaseMsgFunc)&OnKeyDown },
#define WM_KEYUP_ITEM() { WM_KEYUP, 0, (BaseMsgFunc)&OnKeyUp },
#define WM_TIMER_ITEM() { WM_TIMER, 0, (BaseMsgFunc)&OnTimer },
#define WM_MOUSEMOVE_ITEM() { WM_MOUSEMOVE, 0, (BaseMsgFunc)&OnMouseMove },
#define WM_MOVE_ITEM() { WM_MOVE, 0, (BaseMsgFunc)&OnMove },
#define WM_SETFOCUS_ITEM() { WM_SETFOCUS, 0, (BaseMsgFunc)&OnSetFocus },
#define WM_SETCURSOR_ITEM() { WM_SETCURSOR, 0, (BaseMsgFunc)&OnSetCursor },
#define WM_ERASEBKGND_ITEM() { WM_ERASEBKGND, 0, (BaseMsgFunc)&OnEraseBkgnd },
#define WM_INITDIALOG_ITEM() { WM_INITDIALOG, 0, (BaseMsgFunc)&OnInitDialog },
#define WM_SIZE_ITEM() { WM_SIZE, 0, (BaseMsgFunc)&OnSize },
#define WM_RBUTTONDOWN_ITEM() { WM_RBUTTONDOWN, 0, (BaseMsgFunc)&OnRButtonDown },

class BaseWindow {
protected:
    HWND m_hWnd;
    static HINSTANCE m_hInst;
    list<BaseWindow*> m_childList;

protected:
    static LRESULT CALLBACK BaseWndProc(HWND HWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    int MessageBox(LPCTSTR lpText, LPCTSTR lpCaption = TEXT("Boyia"), UINT uType = MB_OK);
    void MessageLoop();
    BOOL ShowWindow(int showCmd);
    BOOL UpdateWindow();
    BaseWindow();
    virtual ~BaseWindow();
    void RemoveAllChild();
    void AddChild(BaseWindow* childtw);
    BaseWindow* CreateChild(LPCWSTR classname, LPCWSTR name, DWORD style, int x, int y, int w, int h, HWND hWndParent, DWORD id, HINSTANCE _thinstance);
    void InitBaseWindow(HINSTANCE _thInstance);
    void RegisterBaseWindow(WNDCLASS& wndClass);
    BOOL CreateBaseWindow(LPCWSTR className, LPCWSTR name, DWORD style, int x, int y, int w, int h, HWND hWndParent);
    HWND GetWndHandle();
    BOOL GetClientRect(LPRECT lpRect);
    BOOL GetWindowRect(LPRECT lpRect);
    BOOL ScreenToClient(LPRECT lpRect);
    BOOL ClientToScreen(LPRECT lpRect);
    BOOL Invalidate(CONST RECT* lpRect = NULL, BOOL bErase = TRUE);

protected:
    static BaseWindow* m_currWinPtr;
    BaseWindow* m_prevWinPtr;
    // 消息响应函数BEGIRN
    BoyiaMsg DWORD OnClose(WPARAM, LPARAM);
    BoyiaMsg DWORD OnDestroy(WPARAM, LPARAM);
    // 消息响应函数END
    MESSAGE_MAP_TABLE()
};

}

#endif