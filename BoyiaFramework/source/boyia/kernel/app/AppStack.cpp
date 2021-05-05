#include "AppStack.h"

namespace yanbo {
AppStack::AppStack()
    : m_stack(10)
{
}

AppStack::~AppStack()
{
}

Application* AppStack::top()
{
    return m_stack.size() > 0 ? m_stack.top() : NULL;
}

Application* AppStack::pop()
{
    return m_stack.size() > 0 ? m_stack.pop() : kBoyiaNull;
}
LVoid AppStack::push(Application* app)
{
    m_stack.push(app);
}
}