#include "BoyiaSocket.h"
#include "MiniThread.h"
#include "MiniThreadPool.h"

namespace yanbo {
class BoyiaRecvThread : public MiniThread {
public:
    BoyiaRecvThread(BoyiaClientSocket* socket)
        : m_socket(socket)
    {
        start();
    }

    virtual LVoid run()
    {
        while (true) {
            m_socket->recvData();
        }
    }

private:
    BoyiaClientSocket* m_socket;
};

class BoyiaSendTask : public MiniTaskBase {
public:
    BoyiaSendTask(BoyiaClientSocket* socket, const CString& text)
        : m_socket(socket)
        , m_message(text, LTrue, text.GetLength())
    {
    }
    virtual ~BoyiaSendTask() {}
    virtual void execute()
    {
        m_socket->sendData(m_message);
    }

    BoyiaClientSocket* m_socket;
    CString m_message;
};

BoyiaSocket::BoyiaSocket()
{
    m_socket = new BoyiaClientSocket(NULL);
}

LVoid BoyiaSocket::connect(const CString& hostName, int serverPort)
{
    m_socket->connectServer(hostName, serverPort);
}

LVoid BoyiaSocket::sendData(const CString& text)
{
    MiniThreadPool::getInstance()->sendMiniTask(new BoyiaSendTask(m_socket, text));
    //text.ReleaseBuffer();
}
}