#include "Application.h"

namespace yanbo {
Application::Application(AppInfo* info)
    : m_info(info)
{
    m_vm = new AppVM();
    m_view = new UIView();
}

Application::~Application()
{
}

UIView* Application::view() const
{
    return m_view;
}

AppVM* Application::vm() const
{
    return m_vm;
}
}