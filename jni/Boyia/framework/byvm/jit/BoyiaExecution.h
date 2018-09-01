#ifndef BoyiaExecution_h
#define BoyiaExecution_h
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
class BoyiaExecution {
public:
    void callCode();
};
}

#endif
