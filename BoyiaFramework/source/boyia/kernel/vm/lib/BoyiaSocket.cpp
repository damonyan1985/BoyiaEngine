#include "BoyiaSocket.h"
#include "UIThread.h"
#include "SalLog.h"

namespace boyia {
BoyiaSocket::BoyiaSocket(const String& url, BoyiaRuntime* runtime)
    : BoyiaBase(runtime)
    , m_wsUrl(url)
    , m_listener(kBoyiaNull)
    , m_socket(kBoyiaNull)
{
    start();
}

LVoid BoyiaSocket::setSocketListener(BoyiaSocketListener* listener)
{
    m_listener = listener;
}

LVoid BoyiaSocket::onListen()
{
    for (LInt i = 0; i < m_msgs.size(); i++) {
        m_socket->send(m_msgs[i]);
    }
}

LVoid BoyiaSocket::send(const String& message)
{
    if (m_socket) {
        m_socket->send(message);
    } else {
        m_msgs.addElement(message);
    }
}

LVoid BoyiaSocket::handleMessage(const String& message)
{
    BOYIA_LOG(">>> %s", GET_STR(message));
    if (m_listener) {
        m_listener->onMessage(message);
    }
}

LVoid BoyiaSocket::run()
{
    yanbo::WebSocket* socket = yanbo::WebSocket::create(m_wsUrl);
    if (!socket) {
        return;
    }

    socket->setHandler(this);
    //onListen();
    // 切到UI线程处理消息
    yanbo::UIThread::instance()->postClosureTask([self = this]() -> void {
        self->onListen();
    });
    
    m_socket = socket;
    
    if (m_listener) {
        m_listener->onListen();
    }

    while (m_socket->getReadyState() != yanbo::WebSocket::kClosed) {
        // 接受消息
        m_socket->poll();
        // 派发消息
        m_socket->dispatch();
    }

    if (m_listener) {
        m_listener->onClose();
    }
}

BoyiaSocket::~BoyiaSocket()
{
    stop();
}
}
