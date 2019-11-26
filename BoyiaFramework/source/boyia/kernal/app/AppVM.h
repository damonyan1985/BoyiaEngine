#ifndef AppVM_h
#define AppVM_h

#include "UtilString.h"

namespace yanbo {
class AppVM {
public:
    AppVM();
    ~AppVM();

    LVoid compile(const String& script);
    LVoid useVM();

private:
    LVoid* m_vm;
    LVoid* m_gc;
};
}

#endif