#include "BoyiaOnLoadWin.h"
#include "AppManager.h"
#include "GraphicsContextWin.h"
#include "PlatformBridge.h"
#include "PixelRatio.h"
#include "BoyiaSocket.h"
#include "ThreadPool.h"
#include <stdio.h>

#ifndef  _WINDLL
#define _WINDLL
#endif


void BoyiaOnLoadWin::setContextWin(HWND hwnd, int width, int height)
{
    yanbo::PixelRatio::setWindowSize(width, height);
	util::LGraphicsContext* gc = yanbo::AppManager::instance()->uiThread()->graphics();
	static_cast<util::GraphicsContextWin*>(gc)->setContextWin(hwnd);
	BOYIA_LOG("hello world apppath=%s\n", yanbo::PlatformBridge::getAppPath());

    yanbo::AppManager::instance()->setViewport(LRect(0, 0, 720, 1280));
    yanbo::AppManager::instance()->start();
}

void BoyiaOnLoadWin::repaint()
{
    util::LGraphicsContext* gc = yanbo::AppManager::instance()->uiThread()->graphics();
    static_cast<util::GraphicsContextWin*>(gc)->repaint();
    //yanbo::AppManager::instance()->currentApp()->view()->getLoader()->repaint();
}

void BoyiaOnLoadWin::connectServer()
{
    //yanbo::ThreadPool::getInstance()->sendTask(new SocketTask());
    yanbo::BoyiaSocket* socket = new yanbo::BoyiaSocket(_CS("ws://192.168.0.7:6666"));
}

void BoyiaOnLoadWin::networkInit()
{
    yanbo::WebSocket::networkInit();
}

void BoyiaOnLoadWin::networkDestroy()
{
    yanbo::WebSocket::networkDestroy();
}