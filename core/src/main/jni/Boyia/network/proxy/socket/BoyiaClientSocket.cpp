#include "BoyiaClientSocket.h"
#include "SalLog.h"
#include "AutoLock.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

namespace yanbo
{
const int KMaxReceiveLength = 1024;
BoyiaClientSocket::BoyiaClientSocket(BoyiaClientListener* listener)
    : m_socketHandle(-1)
    , m_listener(listener)
{
}

BoyiaClientSocket::~BoyiaClientSocket()
{
	closeSocket();
}

int BoyiaClientSocket::initClientSocket(const CString& hostName, int serverPort)
{
	struct hostent *host;
	struct sockaddr_in serv_addr;

	if ((host = gethostbyname(hostName.GetBuffer())) == NULL)
	{
        return EParseHostError;
	}

	KFORMATLOG("initClientSocket gethostbyname hostname=%s", host->h_name);

	if ((m_socketHandle = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		close(m_socketHandle);
        return ECreateSocketError;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serverPort);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serv_addr.sin_zero), sizeof(serv_addr.sin_zero));

	/*调用connect函数主动发起对服务器端的连接*/
	if (connect(m_socketHandle, (struct sockaddr *)&serv_addr,
	                sizeof(struct sockaddr)) == -1)
	{
		KLOG("initClientSocket connect error");
		closeSocket();
        return EConnectError;
	}

	KLOG("initClientSocket connect Success");
}

int BoyiaClientSocket::requestData(const CString& data)
{
	KSTRFORMAT("requestData data=%s", data);
	KFORMATLOG("requestData dataLength=%d", data.GetLength());
	int ret = -1;

	if ((ret = send(m_socketHandle, data.GetBuffer(), data.GetLength(), 0)) == -1)
	{
		KLOG("requestData send Error");
		closeSocket();
        return ESendDataError;
	}
	else
	{
		KLOG("requestData receiveData begin");
		if (receiveData() < 0)
		{
			//KLOG("requestData receiveData Error");
			return EReceiveDataError;
		}

		return ESendDataSuccess;
	}
}

int BoyiaClientSocket::receiveData()
{
	int recvLength = -1;
	do
	{
		char* buffer = new char[KMaxReceiveLength];
		recvLength = recv(m_socketHandle, buffer, KMaxReceiveLength, 0);
	    if (recvLength > 0)
		{
			KFORMATLOG("requestData recvLength=%d", recvLength);
			if (m_listener != NULL)
			{
				m_listener->onSocketDataReceive(buffer, recvLength);
			}
		}
	    else if (recvLength <= 0) // 非正常结束
	    {
			//closeSocket();
			AutoLock lock(&m_mutex);
			if (m_socketHandle != -1)
			{
				closeSocket();
				KFORMATLOG("requestData Error recvLength=%d", recvLength);
				if (m_listener)
				{
					m_listener->onSocketDataError(recvLength);
				}
			}
			else
			{
				KFORMATLOG("requestData Completed recvLength=%d", recvLength);
				if (m_listener)
				{
					m_listener->onSocketDataFinished();
				}
			}

			break;
	    }
		else if (m_socketHandle == -1)
		{
			// 正常关闭
			break;
		}
	} while (LTrue);

	return recvLength;
}

void BoyiaClientSocket::closeSocket()
{
	// 客户端正常结束
	if (m_socketHandle != -1)
	{
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
