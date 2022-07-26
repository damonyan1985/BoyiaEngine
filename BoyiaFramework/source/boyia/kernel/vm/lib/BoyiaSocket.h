#ifndef BoyiaSocket_h
#define BoyiaSocket_h

#include "WebSocket.h"
#include "UtilString.h"
#include "BoyiaBase.h"

namespace boyia {
class BoyiaSocket : 
    public BoyiaBase,
    public BoyiaRef,
    public yanbo::BaseThread,
    public yanbo::WebSocketHandler {
public:
    BoyiaSocket(const String& url, BoyiaValue* msgCB, BoyiaRuntime* runtime);
    virtual LVoid release();
    

    virtual LVoid run();
    virtual LVoid handleMessage(const String& message);
    LVoid send(const String& message);

private:
    ~BoyiaSocket();
    
    LVoid onListen();
    LVoid onMessage(const String& message);
    
    yanbo::WebSocket* m_socket;
    String m_wsUrl;
    KVector<String> m_msgs;
    BoyiaValue m_msgCB;
};
}

#endif
