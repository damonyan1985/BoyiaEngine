#ifndef BoyiaRuntime_h
#define BoyiaRuntime_h

#include "UtilString.h"

namespace boyia {
class BoyiaRuntime {
public:
    BoyiaRuntime();
    ~BoyiaRuntime();

    LVoid compile(const String& script);
    LVoid useVM();
    LVoid* vm() const;

private:
    LVoid* m_vm;
    LVoid* m_gc;
};
}

#endif