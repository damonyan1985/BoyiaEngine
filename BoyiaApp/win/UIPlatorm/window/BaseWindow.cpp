#include "BaseWindow.h"

namespace yanbo {
BaseWindow* BaseWindow::m_currWinPtr = NULL;
HINSTANCE BaseWindow::m_hInst = NULL;

const CommandMessageItem BaseWindow::messageEntries[] = {
    { (INT_PTR) nullptr, 0, 0 },
    WM_CLOSE_ITEM()
    WM_DESTROY_ITEM() { 0, 0, 0 }
};

BaseWindow::BaseWindow()
    : m_hWnd(0)
    , m_prevWinPtr(nullptr)
{
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
    wndClass.lpfnWndProc = BaseWindow::BaseWndProc;
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

BaseWindow* BaseWindow::CreateChild(LPCWSTR classname, LPCWSTR name, DWORD style, int x, int y, int w, int h, HWND hWndParent, DWORD id, HINSTANCE _thinstance)
{
    HWND hwnd;
    BaseWindow* child;
    hwnd = ::CreateWindow(classname, name, style | WS_VISIBLE | WS_CHILD, x, y, w, h, hWndParent, (HMENU)id, m_hInst, 0);

    if (!hwnd)
        return NULL;
    child = new BaseWindow;
    child->m_hWnd = hwnd;
    AddChild(child);
    return child;
}

void BaseWindow::AddChild(BaseWindow* childtw)
{
    m_childList.push_back(childtw);
}

void BaseWindow::RemoveAllChild()
{
    list<BaseWindow*>::iterator ipos;
    list<BaseWindow*>::iterator ipos_begin = m_childList.begin();
    list<BaseWindow*>::iterator ipos_end = m_childList.end();
    for (ipos = ipos_begin; ipos != ipos_end;) {
        list<BaseWindow*>::iterator ipos_tmp;
        ipos_tmp = ipos;
        delete *ipos_tmp;
        ipos++;
        m_childList.erase(ipos_tmp);
    }
}

BaseWindow::~BaseWindow()
{
    RemoveAllChild();
    DestroyWindow(m_hWnd);
}

LRESULT CALLBACK BaseWindow::BaseWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (WM_CREATE == message || WM_INITDIALOG == message) {
        m_currWinPtr->m_hWnd = hWnd;
    }
    BaseWindow* window = m_currWinPtr;

    if (window) {
        const CommandMessageItem* msgItem = window->GetMessageEntries();
        while (msgItem) {
            // index 0 is super class message map
            for (int i = 1; msgItem[i]._tfunc; i++) {
                if (msgItem[i].message != message) {
                    continue;
                }

                if (WM_COMMAND != message) {
                    (window->*(msgItem[i]._tfunc))(wParam, lParam);
                    return 0;
                }

                if (msgItem[i].id == LOWORD(wParam)) {
                    (window->*(msgItem[i]._tfunc))(wParam, lParam);
                    return 0;
                }
            }

            msgItem = (const CommandMessageItem*)msgItem[0].message;
        }
    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL BaseWindow::UpdateWindow()
{
    return ::UpdateWindow(m_hWnd);
}

BOOL BaseWindow::ShowWindow(int showCmd)
{
    return ::ShowWindow(m_hWnd, showCmd);
}

void BaseWindow::MessageLoop()
{
    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
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

}