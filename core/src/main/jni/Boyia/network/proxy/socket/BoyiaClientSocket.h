#ifndef BoyiaClientSocket_h
#define BoyiaClientSocket_h

#include "UtilString.h"
#include "BoyiaRef.h"
#include "BoyiaClientListener.h"
#include "MiniMutex.h"

namespace yanbo
{
// 客户端socket的封装
class BoyiaClientSocket : public BoyiaRef
{
public:
	enum ClientState
	{
        EParseHostError,
        ECreateSocketError,
        EConnectError,
        ESendDataError,
        ESendDataSuccess,
        EReceiveDataError,
        EReceiveDataSuccess,
	};
public:
	BoyiaClientSocket(BoyiaClientListener* listener);
	virtual ~BoyiaClientSocket();

public:
	int initClientSocket(const CString& hostName, int serverPort);
	int requestData(const CString& data);
    int receiveData();
	void blockClose();

private:
	void closeSocket();

	int m_socketHandle;
	BoyiaClientListener *m_listener;
	MiniMutex m_mutex;
};
}
#endif
