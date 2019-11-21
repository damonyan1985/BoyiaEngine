#include "BoyiaOnLoadWin.h"
#include "AppManager.h"
#include "GraphicsContextWin.h"
#include <stdio.h>

#ifndef  _WINDLL
#define _WINDLL
#endif // ! _WINDLL


void BoyiaOnLoadWin::setContextWin(HWND hwnd)
{
	util::LGraphicsContext* gc = yanbo::AppManager::instance()->uiThread()->graphics();
	static_cast<util::GraphicsContextWin*>(gc)->setContextWin(hwnd);
	printf("hello world");
}