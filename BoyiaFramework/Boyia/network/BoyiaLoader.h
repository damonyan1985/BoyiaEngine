#ifndef Loader_h
#define Loader_h

#include "BoyiaHttpEngine.h"
#include "MiniMessageThread.h"
#include "MiniTaskBase.h"
#include "NetworkBase.h"
#include "UtilString.h"

namespace yanbo {

class LoaderTask : public MiniTaskBase, HttpCallback {
public:
    LoaderTask();
    virtual void execute();

    LVoid setHeader(const NetworkMap& map);
    LVoid setMethod(LInt method);
    LVoid setUrl(const String& url);
    LVoid setClient(NetworkClient* client);
    LVoid setPostData(const BoyiaPtr<String>& data);

    virtual LVoid onStatusCode(LInt code);
    virtual LVoid onContentLength(LInt length);
    virtual LVoid onDataRecevied(const LByte* buffer, LInt size);
    virtual LVoid onLoadFinished();
    virtual LVoid onLoadError(LInt code);

private:
    LInt m_method;
    String m_url;
    BoyiaHttpEngine m_engine;
    NetworkClient* m_client;
};

class BoyiaLoader : public MiniMessageThread, public NetworkBase {
public:
    enum LoadMsgType {
        ELOAD_URL,
    };
    BoyiaLoader();

    virtual LVoid loadUrl(const String& url, NetworkClient* client);
    virtual LVoid loadUrl(const String& url, NetworkClient* client, LBool isWait);

    virtual LVoid postData(const String& url, NetworkClient* client);
    virtual LVoid postData(const String& url, NetworkClient* client, LBool isWait);
    virtual LVoid cancel();
    virtual LVoid handleMessage(MiniMessage* msg);
};
}
#endif