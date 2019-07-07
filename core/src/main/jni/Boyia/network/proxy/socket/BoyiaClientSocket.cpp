#include "BoyiaClientSocket.h"
#include "AutoLock.h"
#include "SalLog.h"
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace yanbo {

const int KMaxReceiveLength = 1024;
BoyiaClientSocket::BoyiaClientSocket(BoyiaClientListener* listener)
    : m_socketHandle(-1)
    , m_listener(listener)
{
    initSocket();
}

BoyiaClientSocket::~BoyiaClientSocket()
{
    closeSocket();
}

LVoid BoyiaClientSocket::initSocket()
{
    if ((m_socketHandle = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        //close(m_socketHandle);
        KLOG("Create Socket Error");
        return;
    }
}

LInt BoyiaClientSocket::connectServer(const CString& hostName, int serverPort)
{
    if (m_socketHandle == -1) {
        return EConnectError;
    }

    struct hostent* host;
    struct sockaddr_in serv_addr;

    if ((host = gethostbyname(hostName.GetBuffer())) == NULL) {
        return EParseHostError;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);
    serv_addr.sin_addr = *((struct in_addr*)host->h_addr);
    bzero(&(serv_addr.sin_zero), sizeof(serv_addr.sin_zero));
    // inet_pton(AF_INET, "192.168.0.10", &serv_addr.sin_addr)
    /*调用connect函数主动发起对服务器端的连接*/
    if (connect(m_socketHandle, (struct sockaddr*)&serv_addr,
            sizeof(struct sockaddr))
        == -1) {
        KLOG("initClientSocket connect error");
        closeSocket();
        return EConnectError;
    }

    KLOG("initClientSocket connect Success");
    return EConnectSuccess;
}

LInt BoyiaClientSocket::sendData(const CString& data)
{
    KSTRFORMAT("requestData data=%s", data);
    KFORMATLOG("requestData dataLength=%d", data.GetLength());
    int ret = -1;

    if ((ret = send(m_socketHandle, data.GetBuffer(), data.GetLength(), 0)) == -1) {
        KLOG("requestData send Error");
        closeSocket();
        return ESendDataError;
    }

    return ESendDataSuccess;
}

LInt BoyiaClientSocket::recvData()
{
    int recvLength = -1;
    do {
        char* buffer = new char[KMaxReceiveLength];
        recvLength = recv(m_socketHandle, buffer, KMaxReceiveLength, 0);
        if (recvLength > 0) {
            KFORMATLOG("requestData recvLength=%d", recvLength);
            if (m_listener != NULL) {
                m_listener->onSocketDataReceive(buffer, recvLength);
            }
        } else if (recvLength <= 0) // 非正常结束
        {
            if (m_socketHandle != -1) {
                closeSocket();
                KFORMATLOG("requestData Error recvLength=%d", recvLength);
                if (m_listener) {
                    m_listener->onSocketDataError(recvLength);
                }
            } else {
                KFORMATLOG("requestData Completed recvLength=%d", recvLength);
                if (m_listener) {
                    m_listener->onSocketDataFinished();
                }
            }

            break;
        } else if (m_socketHandle == -1) {
            // 正常关闭
            break;
        }
    } while (LTrue);

    return recvLength;
}

void BoyiaClientSocket::closeSocket()
{
    // 客户端正常结束
    if (m_socketHandle != -1) {
        close(m_socketHandle);
        m_socketHandle = -1;
    }
}

void BoyiaClientSocket::blockClose()
{
    AutoLock lock(&m_mutex);
    closeSocket();
}
}
