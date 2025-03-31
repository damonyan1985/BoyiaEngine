#include "BoyiaDebugger.h"
#include "BoyiaRuntime.h"
#include "WebSocket.h"

namespace boyia {
const LInt kCodePositionCapacity = 2 * MB;    
const String kDebugWsUrl = _CS("ws://127.0.0.1:8003");
class BoyiaDebugConnection {
public:
    BoyiaDebugConnection() {
        m_socket = yanbo::WebSocket::create(kDebugWsUrl);
    }

    ~BoyiaDebugConnection() {
        delete m_socket;
    }

private:
    yanbo::WebSocket* m_socket;
};

BoyiaDebugger::BoyiaDebugger(BoyiaRuntime* runtime)
    : m_runtime(runtime)
    , m_connection(new BoyiaDebugConnection())
    , m_positions(0, kCodePositionCapacity) {}
    
LVoid BoyiaDebugger::setBreakPoint(KVector<Breakpoint>& breakpoints) {
    m_breakpoints.move(breakpoints);
}

LVoid BoyiaDebugger::stepOver() {}

LVoid BoyiaDebugger::resume() {}

LVoid BoyiaDebugger::setCodePosition(LInt codeIndex, LInt row, LInt column) {
    if (m_positions.size() != codeIndex) {
        return;
    }
    m_positions.addElement({row, column});
}

BoyiaCodePosition* BoyiaDebugger::getCodePosition(LInt codeIndex) {
    return &m_positions[codeIndex];
}
}