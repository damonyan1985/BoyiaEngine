#ifndef Debugger_h
#define Debugger_h

#include "PlatformLib.h"
#include "KVector.h"
#include "OwnerPtr.h"
#include "HashMap.h"

namespace boyia {
class BoyiaRuntime;
class BoyiaDebugConnection;
class Breakpoint {
public:
    LInt codeLine;
    LInt codeColumn;
    LInt scriptId;
};
class BoyiaDebugger {
public:
    BoyiaDebugger(BoyiaRuntime* runtime);
    LVoid setBreakPoint(KVector<Breakpoint>& breakpoints);
    LVoid stepOver();
    LVoid resume();

private:
    BoyiaRuntime* m_runtime;
    OwnerPtr<BoyiaDebugConnection> m_connection;
};
}
#endif
