#include "BoyiaSocket.h"
#include "BaseThread.h"
#include "ThreadPool.h"

namespace yanbo {
class BoyiaRecvThread : public BaseThread {
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

class BoyiaSendTask : public TaskBase {
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
    ThreadPool::getInstance()->sendTask(new BoyiaSendTask(m_socket, text));
    //text.ReleaseBuffer();
}
}