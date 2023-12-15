#ifndef BoyiaHttpEngine_h
#define BoyiaHttpEngine_h

#include "NetworkBase.h"

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
    LVoid request(const String& url, LInt method);
    LVoid setPostData(const OwnerPtr<String>& data);

private:
    HttpCallback* m_callback;
    LInt m_size;
    OwnerPtr<String> m_data;
    String m_header;
};
}

#endif