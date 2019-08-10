#include "Application.h"

namespace yanbo {
Application::Application(AppInfo* info)
    : m_info(info)
{
    m_view = new UIView();
    m_vm = new VirtualMachine();
}

Application::~Application()
{
}
}