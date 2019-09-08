#ifndef NetworkBase_h
#define NetworkBase_h

#include "KListMap.h"
#include "OwnerPtr.h"
#include "UtilString.h"
namespace yanbo {

// software abstraction layer API
// network abstract class
class NetworkClient {
public:
    enum LoadError {
        kNetworkSuccess,
        kNetworkUnConnect,
        kNetworkFileError,
        kNetworkMemoryLack
    };

public:
    virtual LVoid onDataReceived(const LByte* data, LInt size) = 0;
    virtual LVoid onStatusCode(LInt statusCode) = 0;
    virtual LVoid onFileLen(LInt len) = 0;
    virtual LVoid onRedirectUrl(const String& redirectUrl) = 0;
    virtual LVoid onLoadError(LInt error) = 0;
    virtual LVoid onLoadFinished() = 0;
};

typedef KListMap<String, String> NetworkMap;
class NetworkBase {
public:
    enum HTTPMethod {
        GET = 0,
        POST,
    };

#if ENABLE(BOYIA_KERNEL)
    static NetworkBase* create()
    {
        return NULL;
    }
#else
    static NetworkBase* create(); // instance a platform NetworkBase
#endif

    NetworkBase()
    {
    }
    virtual ~NetworkBase() {}

public:
    virtual LVoid syncLoadUrl(const String& url, String& content) = 0;
    virtual LVoid loadUrl(const String& url, NetworkClient* client) = 0;
    virtual LVoid loadUrl(const String& url, NetworkClient* client, LBool isWait) = 0;

    virtual LVoid postData(const String& url, NetworkClient* client) = 0;
    virtual LVoid postData(const String& url, NetworkClient* client, LBool isWait) = 0;
    virtual LVoid cancel() = 0;

    virtual LVoid setPostData(const OwnerPtr<String>& data)
    {
        m_data = data;
    }

    virtual LVoid putHeader(const String& key, const String& value)
    {
        m_headers.put(key, value);
    }

    LVoid clearHeaders()
    {
        m_headers.clear();
    }

protected:
    NetworkMap m_headers;
    OwnerPtr<String> m_data; // Post data
};
}
#endif
