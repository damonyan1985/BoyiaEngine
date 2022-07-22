#ifndef BoyiaSocket_h
#define BoyiaSocket_h

#include "WebSocket.h"
#include "UtilString.h"
#include "BaseThread.h"
#include "BoyiaBase.h"

namespace boyia {
class BoyiaSocketListener {
public:
    virtual LVoid onListen() = 0;
    virtual LVoid onMessage(const String& message) = 0;
    virtual LVoid onClose() = 0;
};

class BoyiaSocket : public BoyiaBase, public yanbo::BaseThread, public yanbo::WebSocketHandler {
public:
    BoyiaSocket(const String& url, BoyiaRuntime* runtime);
    ~BoyiaSocket();

    LVoid setSocketListener(BoyiaSocketListener* listener);

    virtual LVoid run();
    virtual LVoid handleMessage(const String& message);
    LVoid onListen();
    LVoid send(const String& message);

private:
    yanbo::WebSocket* m_socket;
    String m_wsUrl;
    BoyiaSocketListener* m_listener;
    KVector<String> m_msgs;
};
}

#endif
