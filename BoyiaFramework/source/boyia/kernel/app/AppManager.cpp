#include "AppManager.h"
#include "PixelRatio.h"
#include "SalLog.h"

namespace yanbo {
AppManager::AppManager()
{
    // 只允许在主线程中执行m_network
    m_network = NetworkBase::create();
    m_loader = new AppLoader(this);
    m_uiThread = new UIThread(this);
}

AppManager::~AppManager()
{
    m_uiThread->quit();
}

NetworkBase* AppManager::network() const
{
    return m_network;
}

UIThread* AppManager::uiThread() const
{
    return m_uiThread;
}

LVoid AppManager::start()
{
    m_loader->startLoad();
}

Application* AppManager::currentApp()
{
    return m_stack.top();
}

AppManager* AppManager::instance()
{
    static AppManager sAppManager;
    return &sAppManager;
}

LVoid AppManager::setViewport(const LRect& rect)
{
    m_clientRect = rect;
}

const LRect& AppManager::getViewport() const
{
    return m_clientRect;
}

LVoid AppManager::launchApp(AppInfo* info)
{
    // 先清空栈
    m_stack.clear();
    m_stack.push(new Application(info));
    // 启动index.ui
    BOYIA_LOG("AppManager::launchApp path=%s", GET_STR(info->path));
    m_uiThread->initApp(info->path);
}

LVoid AppManager::pushApp(AppInfo* info)
{
    m_stack.push(new Application(info));
    // 启动index.ui
    m_uiThread->initApp(info->path);
}

LVoid AppManager::handleTouchEvent(LInt type, LInt x, LInt y)
{
    if (!currentApp() || !currentApp()->view() || !currentApp()->view()->canHit()) {
        return;
    }

    LTouchEvent* evt = new LTouchEvent;

    evt->m_type = 1 << type;
    evt->m_position.Set(PixelRatio::viewX(x), PixelRatio::viewY(y));
    BOYIA_LOG("AppManager::handleTouchEvent type=%d, x=%d, y=%d", type, x, y);
    m_uiThread->handleTouchEvent(evt);
}
}
