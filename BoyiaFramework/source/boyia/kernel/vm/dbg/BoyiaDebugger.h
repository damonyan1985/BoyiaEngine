#ifndef BoyiaDebugger_h
#define BoyiaDebugger_h

#include "PlatformLib.h"
#include "KVector.h"
#include "OwnerPtr.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "BoyiaValue.h"

namespace boyia {
class BoyiaRuntime;
class BoyiaDebugConnection;

class Breakpoint {
public:
    LInt scriptId;
    BoyiaCodePosition position;
};
class BoyiaDebugger {
public:
    BoyiaDebugger(BoyiaRuntime* runtime);
    LVoid setBreakPoint(KVector<Breakpoint>& breakpoints);
    LVoid stepOver();
    LVoid resume();
    LVoid setCodePosition(LInt codeIndex, LInt row, LInt column);
    BoyiaCodePosition* getCodePosition(LInt codeIndex);

private:
    BoyiaRuntime* m_runtime;
    OwnerPtr<BoyiaDebugConnection> m_connection;
    KVector<BoyiaCodePosition> m_positions;
    KVector<Breakpoint> m_breakpoints;
};
}
#endif
