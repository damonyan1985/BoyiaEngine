#include "Application.h"

namespace yanbo {
Application::Application(AppInfo* info)
    : m_info(info)
{
    m_view = new UIView();
    m_vm = new AppVM();
}

Application::~Application()
{
}

UIView* Application::view() const
{
    return m_view;
}
}