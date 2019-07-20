#ifndef BoyiaProcess_h
#define BoyiaProcess_h

#include "PlatformLib.h"

namespace yanbo {
class BoyiaProcess {
public:
    BoyiaProcess();
    void init();
    void procExec();

private:
    LInt m_pid;
};
}

#endif
