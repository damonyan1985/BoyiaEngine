#ifndef WebSocket_h
#define WebSocket_h

#include "UtilString.h"
#include <string>
#include <vector>

namespace yanbo {
class WebSocketHandler {
public:
    virtual void handleMessage(const String& message) = 0;
};

class WebSocket {
public:
    typedef WebSocket* pointer;
    typedef enum ReadyState {
        kClosing,
        kClosed,
        kConnection,
        kOpen
    } ReadyState;

    // Factories:
    static void networkInit();
    static void networkDestroy();
    static pointer createDummy();
    static pointer create(const String& url, const std::string& origin = std::string());
    static pointer createNoMask(const String& url, const std::string& origin = std::string());

    // Interfaces:
    virtual ~WebSocket() {}
    virtual void poll(int timeout = 0) = 0; // timeout in milliseconds
    virtual void send(const String& message) = 0;
    virtual void sendBinary(const std::string& message) = 0;
    virtual void sendBinary(const std::vector<uint8_t>& message) = 0;
    virtual void sendPing() = 0;
    virtual void close() = 0;
    virtual ReadyState getReadyState() const = 0;

    virtual void setHandler(WebSocketHandler* handler) = 0;
    virtual void dispatch() = 0;
};
}

#endif
