#include "BaseWindow.h"

BaseWindow* BaseWindow::m_currWinPtr = NULL;
HINSTANCE BaseWindow::m_hInst = NULL;
const TCommandMessageItem BaseWindow::messageEntries[] = {
    { (INT_PTR) nullptr, 0, 0 },
    WM_CLOSE_ITEM()
    WM_DESTROY_ITEM()
    { 0, 0, 0 }
};

BaseWindow::BaseWindow()
{
    m_hWnd = nullptr;
    m_prevWinPtr = nullptr;
}

void BaseWindow::RegisterBaseWindow(WNDCLASS& wndClass)
{
    RegisterClass(&wndClass);
}

void BaseWindow::InitBaseWindow(HINSTANCE _thInstance)
{
    m_hInst = _thInstance;

    WNDCLASS wndClass;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wndClass.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.hInstance = _thInstance;
    wndClass.lpfnWndProc = BaseWindow::TWndProc;
    wndClass.lpszClassName = L"Boyia";
    wndClass.lpszMenuName = NULL;
    wndClass.style = CS_VREDRAW | CS_HREDRAW;

    RegisterBaseWindow(wndClass);
}

BOOL BaseWindow::CreateBaseWindow(LPCWSTR className, LPCWSTR name, DWORD style, int x, int y, int w, int h, HWND hWndParent)
{
    m_currWinPtr = (BaseWindow*)this;
    m_hWnd = ::CreateWindow(className, name, style, x, y, w, h, nullptr, nullptr, m_hInst, nullptr);
    if (NULL == m_hWnd)
        return FALSE;

    return TRUE;
}

BaseWindow* BaseWindow::CreateTChild(LPCWSTR classname, LPCWSTR name, DWORD style, int x, int y, int w, int h, HWND hWndParent, DWORD id, HINSTANCE _thinstance)
{
    HWND hwnd;
    BaseWindow* childTW;
    hwnd = ::CreateWindow(classname, name, style | WS_VISIBLE | WS_CHILD, x, y, w, h, hWndParent, (HMENU)id, m_hInst, 0);

    if (NULL == hwnd)
        return NULL;
    childTW = new BaseWindow;
    childTW->m_hWnd = hwnd;
    AddChildTW(childTW);
    return childTW;
}

void BaseWindow::AddChildTW(BaseWindow* childtw)
{
    childTWList.push_back(childtw);
}

void BaseWindow::RemoveAllChildTW()
{
    list<BaseWindow*>::iterator ipos;
    list<BaseWindow*>::iterator ipos_begin = childTWList.begin();
    list<BaseWindow*>::iterator ipos_end = childTWList.end();
    for (ipos = ipos_begin; ipos != ipos_end;) {
        list<BaseWindow*>::iterator ipos_tmp;
        ipos_tmp = ipos;
        delete *ipos_tmp;
        ipos++;
        childTWList.erase(ipos_tmp);
    }
}

BaseWindow::~BaseWindow()
{
    RemoveAllChildTW();
    DestroyWindow(m_hWnd);
}

LRESULT CALLBACK BaseWindow::TWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    /*(BaseWindow*)::GeBaseWindowLong(hWnd, GWL_USERDATA);*/
    if (WM_CREATE == message || WM_INITDIALOG == message) {
        m_currWinPtr->m_hWnd = hWnd;
    }
    BaseWindow* _twnd = m_currWinPtr;

    if (_twnd) {
        const TCommandMessageItem* msg_item = _twnd->GetMessageEntries();
        while ((NULL != msg_item)) {
            for (int i = 1; msg_item[i]._tfunc; i++) {
                if (msg_item[i].message == message) {
                    if (WM_COMMAND == message) {
                        if (msg_item[i].id == LOWORD(wParam)) {
                            (_twnd->*(msg_item[i]._tfunc))(wParam, lParam);
                            return 0;
                        }
                    } else {
                        (_twnd->*(msg_item[i]._tfunc))(wParam, lParam);
                        return 0;
                    }
                }
            }
            msg_item = (const TCommandMessageItem*)msg_item[0].message;
        }
    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL BaseWindow::UpdateTW()
{
    return (::UpdateWindow(m_hWnd));
}

BOOL BaseWindow::ShowTW(int show_cmd)
{
    return (::ShowWindow(m_hWnd, show_cmd));
}

void BaseWindow::MessageLoop()
{
    MSG msg;
    while (TRUE) {

        if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
            if (::GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                continue;
            }
            break;
        } else {
            ProcessTick();
        }
    }
}

HWND BaseWindow::GetWndHandle()
{
    return m_hWnd;
}

DWORD BaseWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
    ::DestroyWindow(m_hWnd);
    return TRUE;
}

DWORD BaseWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    ::PostQuitMessage(0);
    return TRUE;
}

int BaseWindow::MessageBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    return ::MessageBox(m_hWnd, lpText, lpCaption, uType);
}

BOOL BaseWindow::GetClientRect(LPRECT lpRect)
{
    return ::GetClientRect(m_hWnd, lpRect);
}

BOOL BaseWindow::GetWindowRect(LPRECT lpRect)
{
    return ::GetWindowRect(m_hWnd, lpRect);
}

BOOL BaseWindow::ScreenToClient(LPRECT lpRect)
{
    BOOL reVal = TRUE;
    POINT* ptLT = (POINT*)lpRect;
    POINT* ptRB = (POINT*)(lpRect + sizeof(POINT));
    reVal &= ::ScreenToClient(m_hWnd, ptLT);
    reVal &= ::ScreenToClient(m_hWnd, ptRB);
    return reVal;
}

BOOL BaseWindow::ClientToScreen(LPRECT lpRect)
{
    BOOL reVal = TRUE;
    POINT* ptLT = (POINT*)lpRect;
    POINT* ptRB = (POINT*)(lpRect + sizeof(POINT));
    reVal &= ::ClientToScreen(m_hWnd, ptLT);
    reVal &= ::ClientToScreen(m_hWnd, ptRB);
    return reVal;
}

BOOL BaseWindow::Invalidate(CONST RECT* lpRect, BOOL bErase)
{
    return ::InvalidateRect(m_hWnd, lpRect, bErase);
}

void BaseWindow::ProcessTick()
{
}