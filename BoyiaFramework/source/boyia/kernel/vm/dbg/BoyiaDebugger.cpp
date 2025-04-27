#include "BoyiaDebugger.h"
#include "BoyiaRuntime.h"
#include "WebSocket.h"

namespace boyia {
const LInt kCodePositionCapacity = 2 * MB;    
const String kDebugWsUrl = _CS("ws://127.0.0.1:8003");
class BoyiaDebugConnection {
public:
    BoyiaDebugConnection() : m_socket(kBoyiaNull) {}

    LVoid connect() {
        m_socket = yanbo::WebSocket::create(kDebugWsUrl);
    }

    ~BoyiaDebugConnection() {
        if (m_socket) {
            delete m_socket;
        }
    }

private:
    yanbo::WebSocket* m_socket;
};

BoyiaDebugger::BoyiaDebugger(BoyiaRuntime* runtime)
    : m_runtime(runtime)
    , m_connection(new BoyiaDebugConnection())
    , m_positions(0, kCodePositionCapacity) {}

BoyiaDebugger::~BoyiaDebugger() {}
    
LVoid BoyiaDebugger::setBreakPoint(KVector<Breakpoint>& breakpoints) {
    m_breakpoints.move(breakpoints);
}

LVoid BoyiaDebugger::stepOver() {}

LVoid BoyiaDebugger::resume() {}

LVoid BoyiaDebugger::setCodePosition(LInt codeIndex, LInt row, LInt column) {
    m_positions.addElement({row, column, codeIndex});
}

BoyiaCodePosition* BoyiaDebugger::getCodePosition(LInt codeIndex) {
    for (LInt i = 0; i < m_positions.size(); i++) {
        if (codeIndex == m_positions[i].mCodeIndex) {
            return &m_positions[i];
        }
    }
    return kBoyiaNull;
}

const KVector<BoyiaCodePosition>& BoyiaDebugger::getCodePositions() const {
    return m_positions;
}

LVoid BoyiaDebugger::loadCodePositions(KVector<BoyiaCodePosition>& positions) {
    m_positions.move(positions);
}

LVoid BoyiaDebugger::connectDebugServer() {
    m_connection->connect();
}
}