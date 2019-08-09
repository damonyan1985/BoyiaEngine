#ifndef AppManager_h
#define AppManager_h

#include "AppStack.h"
#include "Application.h"

namespace yanbo {
class AppManager {
private:
public:
    AppManager();
    ~AppManager();

    Application* currentApp();

private:
    AppStack m_stack;
};
}

#endif