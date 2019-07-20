#ifndef BoyiaClientSocket_h
#define BoyiaClientSocket_h

#include "BoyiaClientListener.h"
#include "BoyiaRef.h"
#include "MiniMutex.h"
#include "UtilString.h"

namespace yanbo {

// 客户端socket的封装
class BoyiaClientSocket {
public:
    enum ClientState {
        EParseHostError,
        EConnectError,
        EConnectSuccess,
        ESendDataError,
        ESendDataSuccess,
        EReceiveDataError,
        EReceiveDataSuccess,
    };

public:
    BoyiaClientSocket(BoyiaClientListener* listener);
    virtual ~BoyiaClientSocket();

public:
    LInt connectServer(const CString& hostName, int serverPort);
    LInt sendData(const CString& data);
    LInt recvData();
    LVoid blockClose();

private:
    LVoid closeSocket();
    LVoid initSocket();

    LInt m_socketHandle;
    BoyiaClientListener* m_listener;
    MiniMutex m_mutex;
};
}
#endif
