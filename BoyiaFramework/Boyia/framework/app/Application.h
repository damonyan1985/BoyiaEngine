#ifndef Application_h
#define Application_h

#include "AppInfo.h"
#include "UIView.h"
#include "VirtualMachine.h"

namespace yanbo {
class Application {
public:
    Application(AppInfo* info);
    ~Application();

private:
    UIView* m_view;
    VirtualMachine* m_vm;
    AppInfo* m_info;
};
}

#endif