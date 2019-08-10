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
    return m_stack.top();
}

Application* AppStack::pop()
{
    return m_stack.pop();
}
LVoid AppStack::push(Application* app)
{
    m_stack.push(app);
}
}