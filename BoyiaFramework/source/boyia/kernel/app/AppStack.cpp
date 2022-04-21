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
    return m_stack.size() > 0 ? m_stack.top() : kBoyiaNull;
}

Application* AppStack::pop()
{
    return m_stack.size() > 0 ? m_stack.pop() : kBoyiaNull;
}
LVoid AppStack::push(Application* app)
{
    m_stack.push(app);
}

LVoid AppStack::clear()
{
    for (LInt i = 0; i < m_stack.size(); i++) {
        delete m_stack.elementAt(i);
    }
    m_stack.clear();
}
}
