#ifndef BoyiaSocket_h
#define BoyiaSocket_h

#include "WebSocket.h"
#include "UtilString.h"
#include "BaseThread.h"

namespace yanbo {
class BoyiaSocketListener {
public:
    virtual LVoid onListen() = 0;
    virtual LVoid onMessage(const String& message) = 0;
    virtual LVoid onClose() = 0;
};

class BoyiaSocket : public BaseThread, public WebSocketHandler {
public:
    BoyiaSocket(const String& url);
    ~BoyiaSocket();

    LVoid setSocketListener(BoyiaSocketListener* listener);

    virtual LVoid run();
    virtual LVoid handleMessage(const String& message);
    LVoid onListen();
    LVoid send(const String& message);

private:
    WebSocket* m_socket;
    String m_wsUrl;
    BoyiaSocketListener* m_listener;
};
}

#endif
