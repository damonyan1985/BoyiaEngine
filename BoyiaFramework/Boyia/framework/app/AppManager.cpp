#include "AppManager.h"
#include "SalLog.h"
#include "ShaderUtil.h"

namespace yanbo {
AppManager::AppManager()
{
    m_network = NetworkBase::create();
    m_loader = new AppLoader(this);
    m_appThread = new AppThread();
    m_uiThread = new UIThread(this);
}

AppManager::~AppManager()
{
}

NetworkBase* AppManager::network() const
{
    return m_network;
}

AppThread* AppManager::appThread() const
{
    return m_appThread;
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
    m_stack.push(new Application(info));
    m_appThread->load(info->path);
}

LVoid AppManager::handleTouchEvent(LInt type, LInt x, LInt y)
{
    if (!currentApp() || !currentApp()->view() || !currentApp()->view()->canHit()) {
        return;
    }

    LTouchEvent* evt = new LTouchEvent;

    evt->m_type = 1 << type;
    evt->m_position.Set(ShaderUtil::viewX(x), ShaderUtil::viewY(y));
    BOYIA_LOG("AppManager::handleTouchEvent type=%d, x=%d, y=%d", type, x, y);
    m_uiThread->handleTouchEvent(evt);
}
}