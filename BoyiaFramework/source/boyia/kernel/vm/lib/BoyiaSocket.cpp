#include "BoyiaSocket.h"
#include "SalLog.h"

namespace yanbo {
BoyiaSocket::BoyiaSocket(const String& url)
    : m_wsUrl(url)
    , m_listener(kBoyiaNull)
{
    start();
}

LVoid BoyiaSocket::setSocketListener(BoyiaSocketListener* listener)
{
    m_listener = listener;
}

LVoid BoyiaSocket::onListen()
{
    m_socket->send(_CS("Test BoyiaSocket1"));
}

LVoid BoyiaSocket::send(const String& message)
{
    m_socket->send(message);
}

void BoyiaSocket::handleMessage(const String& message)
{
    BOYIA_LOG(">>> %s", GET_STR(message));
    m_socket->send(_CS("Test BoyiaSocket"));
    if (m_listener) {
        m_listener->onMessage(message);
    }
}

LVoid BoyiaSocket::run()
{
    m_socket = WebSocket::create(m_wsUrl); 
    if (!m_socket) {
        return;
    }

    m_socket->setHandler(this);
    onListen();
    if (m_listener) {
        m_listener->onListen();
    }

    while (m_socket->getReadyState() != yanbo::WebSocket::CLOSED) {
        m_socket->poll();
        m_socket->dispatch();
        m_socket->send(_CS("hahaha"));
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