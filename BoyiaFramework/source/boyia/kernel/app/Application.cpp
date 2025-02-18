#include "Application.h"

namespace yanbo {
Application::Application(AppInfo* info)
    : m_info(info)
    , m_view(new UIView(this))
    , m_runtime(new boyia::BoyiaRuntime(this))
{
}

LVoid Application::init(const String& entryPage)
{
    // init native function and special identify
    m_runtime->init();
    m_view->loadPage(entryPage);
}

Application::~Application()
{
}

UIView* Application::view() const
{
    return m_view;
}

boyia::BoyiaRuntime* Application::runtime() const
{
    return m_runtime;
}

const AppInfo* Application::appInfo() const
{
    return m_info;
}
}
