#include "Application.h"

namespace yanbo {
Application::Application(AppInfo* info)
    : m_info(info)
{
    m_view = new UIView();
    m_runtime = new boyia::BoyiaRuntime(this);
}

LVoid Application::init(const String& entryPage)
{
    m_runtime->useVM();
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
}