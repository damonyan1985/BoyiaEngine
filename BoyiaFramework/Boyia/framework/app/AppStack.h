#ifndef AppStack_h
#define AppStack_h

#include "Application.h"
#include "Stack.h"

namespace yanbo {
class AppStack {
public:
    AppStack();
    ~AppStack();

    Application* top();

private:
    Stack<Application*> m_stack;
};
}

#endif