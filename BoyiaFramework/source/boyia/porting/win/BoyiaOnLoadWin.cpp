#include "BoyiaOnLoadWin.h"
#include "AppManager.h"
#include "GraphicsContextWin.h"
#include "PlatformBridge.h"
#include "PixelRatio.h"
#include "BoyiaSocket.h"
#include "ThreadPool.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "BoyiaCore.h"
#include "FileUtil.h"
#include <stdio.h>
//#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

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

    HashMap<HashString, String> map;
    map.put(HashString(_CS("key1")), _CS("value1"));
    map.put(HashString(_CS("key2")), _CS("value2"));
    map.put(HashString(_CS("key3")), _CS("value3"));
    map.put(HashString(_CS("key4")), _CS("value4"));
    
    map.put(HashString(_CS("key5")), _CS("value5"));
    map.put(HashString(_CS("key6")), _CS("value6"));
    map.put(HashString(_CS("key7")), _CS("value7"));
    map.put(HashString(_CS("key8")), _CS("value8"));

    BOYIA_LOG("get map value=%s", GET_STR(map.get(HashString(_CS("key7")))));
}

void BoyiaOnLoadWin::networkInit()
{
    WSADATA wsaData;
    ::WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void BoyiaOnLoadWin::networkDestroy()
{
    ::WSACleanup();
}

void BoyiaOnLoadWin::handleTouchEvent(int type, int x, int y)
{
    yanbo::AppManager::instance()->handleTouchEvent(type, x, y);
}

void BoyiaOnLoadWin::cacheCode()
{
   
    if (FileUtil::isExist(yanbo::PlatformBridge::getInstructionEntryPath())) {
        return;
    }

    CacheVMCode(yanbo::AppManager::instance()->currentApp()->runtime()->vm());
}