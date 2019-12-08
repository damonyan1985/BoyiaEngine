#include "BoyiaOnLoadWin.h"
#include "AppManager.h"
#include "GraphicsContextWin.h"
#include "PlatformBridge.h"
#include "PixelRatio.h"
#include <stdio.h>

#ifndef  _WINDLL
#define _WINDLL
#endif // ! _WINDLL


void BoyiaOnLoadWin::setContextWin(HWND hwnd, int width, int height)
{
    yanbo::PixelRatio::setWindowSize(width, height);
	util::LGraphicsContext* gc = yanbo::AppManager::instance()->uiThread()->graphics();
	static_cast<util::GraphicsContextWin*>(gc)->setContextWin(hwnd);
	BOYIA_LOG("hello world apppath=%s\n", yanbo::PlatformBridge::getAppPath());

    yanbo::AppManager::instance()->setViewport(LRect(0, 0, 720, 1280));
    yanbo::AppManager::instance()->start();
}