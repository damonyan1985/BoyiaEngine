#ifndef BoyiaSocket_h
#define BoyiaSocket_h

#include "WebSocket.h"
#include "UtilString.h"
#include "BaseThread.h"

namespace yanbo {
class BoyiaSocket : public BaseThread, public WebSocketHandler {
public:
    BoyiaSocket(const String& url);
    ~BoyiaSocket();

    virtual LVoid run();
    virtual LVoid handleMessage(const std::string& message);
    LVoid onListen();

private:
    WebSocket* m_socket;
    String m_wsUrl;
};
}

#endif
