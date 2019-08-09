#ifndef VirtualMachine_h
#define VirtualMachine_h

#include "UtilString.h"

namespace yanbo {
class VirtualMachine {
public:
    VirtualMachine();
    ~VirtualMachine();

    LVoid compile(const String& script);

private:
    LVoid* m_vm;
};
}

#endif