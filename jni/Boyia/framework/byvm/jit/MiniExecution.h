#ifndef MiniExecution_h
#define MiniExecution_h
#include "PlatformLib.h"
namespace mjs {
class NativeCode {
public:
	NativeCode();
	~NativeCode();
	LByte* codeStart();
    LVoid copyCode(LByte* buffer, LInt len);

private:
	LByte* m_codeSegment;
    LInt m_codeSize;
};
class MiniExecution {
public:
    void callCode();
};
}

#endif
