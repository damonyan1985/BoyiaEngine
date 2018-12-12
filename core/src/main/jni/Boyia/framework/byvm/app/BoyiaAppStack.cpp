#include "BoyiaAppStack.h"

#define DEFAULT_APP_SIZE 3
namespace boyia
{
BoyiaAppStack::BoyiaAppStack()
    : m_appStack(DEFAULT_APP_SIZE)
{
}

BoyiaAppStack::~BoyiaAppStack()
{
}

LVoid BoyiaAppStack::pushApp(BoyiaApplication* app)
{
    m_appStack.push(app);
    app->resume();
}

LVoid BoyiaAppStack::popApp()
{
    m_appStack.pop();
    if (m_appStack.size())
    {
        BoyiaApplication* app = m_appStack.elementAt(m_appStack.size() - 1);
        app->resume();
    }
}
}
