#include "BoyiaOnLoadWin.h"
#include "AppManager.h"
#include "GraphicsContextWin.h"
#include "PlatformBridge.h"
#include "PixelRatio.h"
#include "WebSocket.h"
#include "ThreadPool.h"
#include <stdio.h>

#ifndef  _WINDLL
#define _WINDLL
#endif

void handle_message(const std::string& message)
{
    printf(">>> %s\n", message.c_str());
    //if (message == "world") { ws->close(); }
}

class SocketTask : public yanbo::TaskBase {
public:
    virtual void execute()
    {
        ws = yanbo::WebSocket::from_url("ws://127.0.0.1:6666/test");
        while (ws->getReadyState() != yanbo::WebSocket::CLOSED) {
            ws->poll();
            ws->send("goodbye");
            ws->send("hello");
            ws->dispatch(handle_message);
        }
        //delete ws;
    }

    yanbo::WebSocket::pointer ws;
};


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
    yanbo::ThreadPool::getInstance()->sendTask(new SocketTask());
}

void BoyiaOnLoadWin::networkInit()
{
    yanbo::WebSocket::networkInit();
}

void BoyiaOnLoadWin::networkDestroy()
{
    yanbo::WebSocket::networkDestroy();
}