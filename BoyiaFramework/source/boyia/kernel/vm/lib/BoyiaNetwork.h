#ifndef BoyiaNetwork_h
#define BoyiaNetwork_h

#include "BoyiaCore.h"
#include "NetworkBase.h"
#include "StringBuilder.h"
#include "UIThread.h"

namespace boyia {
// BoyiaNetwork会根据回调的情况，自动释放自己
class BoyiaNetwork : public yanbo::NetworkClient, public yanbo::UIEvent {
public:
    BoyiaNetwork(BoyiaValue* callback, BoyiaValue* obj, LVoid* vm);
    virtual ~BoyiaNetwork();

public:
    LVoid load(const String& url);

    virtual LVoid onDataReceived(const LByte* data, LInt size);
    virtual LVoid onStatusCode(LInt statusCode);
    virtual LVoid onFileLen(LInt len);
    virtual LVoid onRedirectUrl(const String& redirectUrl);
    virtual LVoid onLoadError(LInt error);
    virtual LVoid onLoadFinished();

    virtual LVoid run();

private:
    BoyiaValue m_callback;
    BoyiaValue m_obj;
    yanbo::StringBuilder m_builder;
    OwnerPtr<String> m_data;
    LVoid* m_vm;
};
}
#endif
