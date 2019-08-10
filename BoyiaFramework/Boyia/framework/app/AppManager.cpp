#include "AppManager.h"

namespace yanbo {
AppManager::AppManager()
{
    m_loader = new AppLoader(this);
    m_appThread = new BoyiaThread();
    m_uiThread = new UIThread();
}

AppManager::~AppManager()
{
}

BoyiaThread* AppManager::appThread() const
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
}