#ifndef MiniProcess_h
#define MiniProcess_h

#include "PlatformLib.h"

namespace yanbo {
class MiniProcess {
public:
	MiniProcess();
	void init();
	void procExec();

private:
    LInt m_pid;
};

}

#endif
