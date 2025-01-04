#ifndef BoyiaNetwork_h
#define BoyiaNetwork_h

#include "BoyiaCore.h"
#include "NetworkBase.h"
#include "StringBuilder.h"
#include "BoyiaAsyncEvent.h"

namespace boyia {
// BoyiaNetwork会根据回调的情况，自动释放自己
class BoyiaNetwork : public BoyiaAsyncEvent, public yanbo::NetworkClient {
public:
    BoyiaNetwork(BoyiaValue* callback, BoyiaValue* obj, BoyiaRuntime* runtime);
    virtual ~BoyiaNetwork();

public:
    LVoid load(const String& url);
    LVoid load(const String& url, const String& params);
    // 设置请求参数
    
    virtual LVoid onDataReceived(const LByte* data, LInt size);
    virtual LVoid onStatusCode(LInt statusCode);
    virtual LVoid onFileLen(LInt len);
    virtual LVoid onRedirectUrl(const String& redirectUrl);
    virtual LVoid onLoadError(LInt error);
    virtual LVoid onLoadFinished();

    virtual LVoid callback();

private:
    BoyiaValue m_callback;
    yanbo::StringBuilder m_builder;
    OwnerPtr<String> m_data;
    String m_params;
    OwnerPtr<yanbo::NetworkBase> m_loader;
};
}
#endif
