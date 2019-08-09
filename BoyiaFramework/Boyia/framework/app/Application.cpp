#include "Application.h"

namespace yanbo {
Application::Application()
{
    m_view = new UIView();
    m_vm = new VirtualMachine();
}

Application::~Application()
{
}
}