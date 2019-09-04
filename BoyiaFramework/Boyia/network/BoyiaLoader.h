#ifndef Loader_h
#define Loader_h

#include "MessageThread.h"
#include "NetworkBase.h"
#include "UtilString.h"

namespace yanbo {
class BoyiaLoader : public MessageThread, public NetworkBase {
public:
    enum LoadMsgType {
        ELOAD_URL,
    };
    BoyiaLoader();

    virtual LVoid syncLoadUrl(const String& url, String& content);
    virtual LVoid loadUrl(const String& url, NetworkClient* client);
    virtual LVoid loadUrl(const String& url, NetworkClient* client, LBool isWait);

    virtual LVoid postData(const String& url, NetworkClient* client);
    virtual LVoid postData(const String& url, NetworkClient* client, LBool isWait);
    virtual LVoid cancel();
    virtual LVoid handleMessage(Message* msg);
};
}
#endif
