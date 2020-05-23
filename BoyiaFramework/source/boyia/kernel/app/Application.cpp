#include "Application.h"

namespace yanbo {
Application::Application(AppInfo* info)
    : m_info(info)
{
    m_runtime = new boyia::BoyiaRuntime();
    m_view = new UIView();
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