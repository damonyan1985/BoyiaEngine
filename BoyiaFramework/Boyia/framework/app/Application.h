#ifndef Application_h
#define Application_h

#include "UIView.h"
#include "VirtualMachine.h"

namespace yanbo {
class Application {
public:
    Application();
    ~Application();

private:
    UIView* m_view;
    VirtualMachine* m_vm;
};
}

#endif