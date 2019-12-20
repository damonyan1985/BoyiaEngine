#include "BoyiaSocket.h"

namespace yanbo {
BoyiaSocket::BoyiaSocket(const String& url)
    : m_wsUrl(url)
{
    //m_socket = WebSocket::create(GET_STR(url));
    start();
}

LVoid BoyiaSocket::onListen()
{
}

void BoyiaSocket::handleMessage(const std::string& message)
{
    printf(">>> %s\n", message.c_str());
    m_socket->send("Test BoyiaSocket");
}

LVoid BoyiaSocket::run()
{
    m_socket = WebSocket::create(GET_STR(m_wsUrl)); 
    if (!m_socket) {
        return;
    }

    m_socket->setHandler(this);
    onListen();
    while (m_socket->getReadyState() != yanbo::WebSocket::CLOSED) {
        m_socket->poll();
        m_socket->send("Test BoyiaSocket");
        m_socket->dispatch();
    }
}

BoyiaSocket::~BoyiaSocket()
{
    stop();
}
}