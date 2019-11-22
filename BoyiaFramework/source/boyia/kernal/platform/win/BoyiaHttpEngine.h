#ifndef BoyiaHttpEngine_h
#define BoyiaHttpEngine_h

#include "NetworkBase.h"
#include <windows.h>
#include <WinInet.h>

namespace yanbo {
class HttpCallback {
public:
    virtual LVoid onStatusCode(LInt code) = 0;
    virtual LVoid onContentLength(LInt length) = 0;
    virtual LVoid onDataReceived(const LByte* buffer, LInt size) = 0;
    virtual LVoid onLoadFinished() = 0;
    virtual LVoid onLoadError(LInt code) = 0;
};

class BoyiaHttpEngine {
public:
    BoyiaHttpEngine(HttpCallback* callback);
    ~BoyiaHttpEngine();

    LVoid setHeader(const NetworkMap& headers);
    LVoid request(const char* url, LInt method);
    LVoid setPostData(const OwnerPtr<String>& data);

    static size_t writeCallback(LVoid* buffer, size_t size, size_t membyte, LVoid* param);

private:
    HttpCallback* m_callback;
    LInt m_size;
    OwnerPtr<String> m_data;
};
}

#endif