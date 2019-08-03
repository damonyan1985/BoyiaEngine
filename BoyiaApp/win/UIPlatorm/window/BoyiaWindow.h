#ifndef BoyiaWindow_h
#define BoyiaWindow_h

#include "BaseWindow.h"

class BoyiaWindow : public BaseWindow
{
public:
	BoyiaWindow();
	virtual ~BoyiaWindow();

protected:
	BoyiaMsg DWORD OnCreate(WPARAM, LPARAM);
	BoyiaMsg DWORD OnClose(WPARAM, LPARAM);
	BoyiaMsg DWORD OnDestroy(WPARAM, LPARAM);
	MESSAGE_MAP_TABLE()
};

class BoyiaApp
{
public:
	BoyiaApp();
	~BoyiaApp();
protected:
	static BoyiaApp *m_pCurrApp;

public:
	BoyiaWindow * _ptwin;
	void FreeWndPtr();
	virtual BOOL InitInstance(HINSTANCE inst, int nCmdShow);
	virtual BOOL run();
	static BoyiaApp* GetCurrApp();
};

#endif