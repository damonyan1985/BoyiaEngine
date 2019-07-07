#ifndef BoyiaSocket_h
#define BoyiaSocket_h

#include "BoyiaClientSocket.h"

namespace yanbo {
class BoyiaRecvThread;
class BoyiaSocket {
public:
    BoyiaSocket();
    LVoid connect(const CString& hostName, int serverPort);

    LVoid sendData(const CString& text);

private:
    BoyiaClientSocket* m_socket;
    BoyiaRecvThread* m_recv;
};
}

#endif