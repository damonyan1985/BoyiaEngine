#ifndef BaseWindow_h
#define BaseWindow_h

#include <list>
#include <windows.h>

using namespace std;

class BaseWindow;

#define BoyiaWinMain wWinMain
#define BoyiaWinApi WINAPI

typedef DWORD(BaseWindow::*BaseWndProc)(WPARAM, LPARAM);

typedef struct TCommandMessageItem
{
	DWORD         message;
	DWORD              id;
	BaseWndProc    _tfunc;
}TCommandMessageItem;

#define MESSAGE_MAP_TABLE()                                                            \
public:                                                                                \
    virtual const TCommandMessageItem* GetMessageEntries() { return messageEntries; }  \
	static const TCommandMessageItem messageEntries[];

#define BEGIN_MAP_TABLE(thisClass, baseClass) \
	const TCommandMessageItem thisClass::messageEntries[]= \
	{ \
		{(INT_PTR)(baseClass::messageEntries), 0, 0},

#define END_MAP_TABLE() {0,0,0}};                                                            


#define BoyiaMsg

#define WM_COMMAND_ITEM(id, func) {WM_COMMAND, id, (BaseWndProc)&func},
#define WM_CREATE_ITEN() {WM_CREATE, 0, (BaseWndProc)&OnCreate},
#define WM_CLOSE_ITEM() {WM_CLOSE, 0, (BaseWndProc)&OnClose},
#define WM_DESTROY_ITEM() {WM_DESTROY, 0, (BaseWndProc)&OnDestroy},
#define WM_PAINT_ITEM() {WM_PAINT, 0, (BaseWndProc)&OnPaint},
#define WM_LBUTTONDOWN_ITEM() {WM_LBUTTONDOWN, 0, (BaseWndProc)&OnLButtonDown},
#define WM_LBUTTONUP_ITEM() {WM_LBUTTONUP, 0, (BaseWndProc)&OnLButtonUp},
#define WM_CHAR_ITEM() {WM_CHAR, 0, (BaseWndProc)&OnChar},
#define WM_KEYDOWN_ITEM() {WM_KEYDOWN, 0, (BaseWndProc)&OnKeyDown},
#define WM_KEYUP_ITEM() {WM_KEYUP, 0, (BaseWndProc)&OnKeyUp},
#define WM_TIMER_ITEM() {WM_TIMER, 0, (BaseWndProc)&OnTimer},
#define WM_MOUSEMOVE_ITEM() {WM_MOUSEMOVE, 0, (BaseWndProc)&OnMouseMove},
#define WM_MOVE_ITEM() {WM_MOVE, 0, (BaseWndProc)&OnMove},
#define WM_SETFOCUS_ITEM() {WM_SETFOCUS, 0, (BaseWndProc)&OnSetFocus},
#define WM_SETCURSOR_ITEM() {WM_SETCURSOR, 0, (BaseWndProc)&OnSetCursor},
#define WM_ERASEBKGND_ITEM() {WM_ERASEBKGND, 0, (BaseWndProc)&OnEraseBkgnd},
#define WM_INITDIALOG_ITEM() {WM_INITDIALOG, 0, (BaseWndProc)&OnInitDialog},
#define WM_SIZE_ITEM() {WM_SIZE, 0, (BaseWndProc)&OnSize},
#define WM_RBUTTONDOWN_ITEM() {WM_RBUTTONDOWN, 0, (BaseWndProc)&OnRButtonDown},

class BaseWindow
{
protected:
	HWND                 m_hWnd;
	static HINSTANCE    m_hInst;
	list<BaseWindow *> childTWList;

protected:
	static LRESULT CALLBACK TWndProc(HWND HWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	int MessageBox(LPCTSTR lpText, LPCTSTR lpCaption = TEXT("Boyia"), UINT uType = MB_OK);
	void MessageLoop();
	BOOL ShowTW(int show_cmd);
	BOOL UpdateTW();
	BaseWindow();
	virtual ~BaseWindow();
	virtual void ProcessTick();
	void RemoveAllChildTW();
	void AddChildTW(BaseWindow *childtw);
	BaseWindow *CreateTChild(LPCWSTR classname, LPCWSTR name, DWORD style, int x, int y, int w, int h, HWND hWndParent, DWORD id, HINSTANCE _thinstance);
	void InitBaseWindow(HINSTANCE _thInstance);
	void RegisterBaseWindow(WNDCLASS &wndClass);
	BOOL CreateBaseWindow(LPCWSTR className, LPCWSTR name, DWORD style, int x, int y, int w, int h, HWND hWndParent);
	HWND GetWndHandle();
	BOOL GetClientRect(LPRECT lpRect);
	BOOL GetWindowRect(LPRECT lpRect);
	BOOL ScreenToClient(LPRECT lpRect);
	BOOL ClientToScreen(LPRECT lpRect);
	BOOL Invalidate(CONST RECT* lpRect = NULL, BOOL bErase = TRUE);

protected:
	static BaseWindow *m_currWinPtr;
	BaseWindow *m_prevWinPtr;
	//消息响应函数BEGIRN
	BoyiaMsg DWORD OnClose(WPARAM, LPARAM);
	BoyiaMsg DWORD OnDestroy(WPARAM, LPARAM);
	//消息响应函数END
	MESSAGE_MAP_TABLE()
};

#endif