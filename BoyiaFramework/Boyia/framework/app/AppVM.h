#ifndef VirtualMachine_h
#define VirtualMachine_h

#include "UtilString.h"

namespace yanbo {
class AppVM {
public:
    AppVM();
    ~AppVM();

    LVoid compile(const String& script);

private:
    LVoid* m_vm;
};
}

#endif