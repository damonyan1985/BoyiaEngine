#include "BoyiaOnLoadWin.h"
#include "AppManager.h"
#include "PlatformBridge.h"
#include "PixelRatio.h"
#include "BoyiaSocket.h"
#include "ThreadPool.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "BoyiaCore.h"
#include "FileUtil.h"
#include "RenderThread.h"
#include "RenderEngineWin.h"
#include <stdio.h>
#include <mfapi.h>
//#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

#ifndef  _WINDLL
#define _WINDLL
#endif

#define OLD_RENDER_FEATURE 0

class WindowRepaintEvent : public yanbo::UIEvent {
public:
    // Run In UIThread
    virtual LVoid run() LOverride
    {
        yanbo::RenderThread::instance()->renderReset();
    }
};

BoyiaOnLoadWin::BoyiaOnLoadWin() {
    initEngine();
}

BoyiaOnLoadWin::~BoyiaOnLoadWin() {
    destroyEngine();
}

void BoyiaOnLoadWin::setContextWin(HWND hwnd, int width, int height)
{
    yanbo::PixelRatio::setWindowSize(width, height);
    yanbo::PixelRatio::setLogicWindowSize(720, 1280);

    yanbo::IRenderEngine* engine = yanbo::RenderThread::instance()->getRenderer();
    static_cast<yanbo::RenderEngineWin*>(engine)->setContextWin(hwnd);
    
    BOYIA_LOG("hello world apppath=%s\n", yanbo::PlatformBridge::getAppPath());

    LInt logicHeight = (1.0f * 720 / width) * height;
    yanbo::AppManager::instance()->setViewport(LRect(0, 0, 720, logicHeight));
    yanbo::AppManager::instance()->start();
}

void BoyiaOnLoadWin::repaint()
{
    yanbo::RenderThread::instance()->renderReset();
}

void BoyiaOnLoadWin::connectServer()
{
    //yanbo::ThreadPool::getInstance()->sendTask(new SocketTask());
    //boyia::BoyiaSocket* socket = new boyia::BoyiaSocket(_CS("ws://192.168.0.7:6666"), kBoyiaNull);

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

void BoyiaOnLoadWin::initEngine()
{
    WSADATA wsaData;
    ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    // 媒体库初始化
    ::MFStartup(MF_VERSION, MFSTARTUP_FULL);
}

void BoyiaOnLoadWin::destroyEngine()
{
    ::WSACleanup();
    // 媒体库资源释放
    ::MFShutdown();
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