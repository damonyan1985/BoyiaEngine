#ifndef BoyiaSocket_h
#define BoyiaSocket_h

#include "WebSocket.h"
#include "UtilString.h"
#include "BaseThread.h"
#include "BoyiaBase.h"

namespace boyia {
class BoyiaSocket : public BoyiaBase, public yanbo::BaseThread, public yanbo::WebSocketHandler {
public:
    BoyiaSocket(const String& url, BoyiaValue* msgCB, BoyiaRuntime* runtime);
    ~BoyiaSocket();

    virtual LVoid run();
    virtual LVoid handleMessage(const String& message);
    LVoid onListen();
    LVoid send(const String& message);

private:
    LVoid onMessage(const String& message);
    
    yanbo::WebSocket* m_socket;
    String m_wsUrl;
    KVector<String> m_msgs;
    BoyiaValue m_msgCB;
};
}

#endif
