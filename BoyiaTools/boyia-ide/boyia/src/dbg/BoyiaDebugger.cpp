#include "BoyiaDebugger.h"
#include "BoyiaRuntime.h"
#include "WebSocket.h"

namespace boyia {
class BoyiaDebugConnection {

};

BoyiaDebugger::BoyiaDebugger(BoyiaRuntime* runtime)
    : m_runtime(runtime)
    , m_connection(new BoyiaDebugConnection()) {}
    
LVoid BoyiaDebugger::setBreakPoint(KVector<Breakpoint>& breakpoints) {

}

LVoid BoyiaDebugger::stepOver() {}

LVoid BoyiaDebugger::resume() {}
}