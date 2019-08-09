#include "AppStack.h"

namespace yanbo {
AppStack::AppStack()
    : m_stack(10)
{
}

AppStack::~AppStack()
{
}

Application* AppStack::top() const
{
    return m_stack.top();
}
}