#ifndef NetworkBase_h
#define NetworkBase_h

#include "KListMap.h"
#include "UtilString.h"
namespace yanbo
{

// software abstraction layer API
// network abstract class
class NetworkClient
{
public:
	enum LoadError
	{
	    NETWORK_UNCONNECT,
		NETWORK_FILE_ERROR,
		NETWORK_MEMORY_LACK
	};

public:
	virtual LVoid onDataReceived(const LByte* data, LInt size) = 0;
	virtual LVoid onStatusCode(LInt statusCode) = 0;
	virtual LVoid onFileLen(LInt len) = 0;
	virtual LVoid onRedirectUrl(const String& redirectUrl) = 0;
	virtual LVoid onLoadError(LInt error) = 0;
	virtual LVoid onLoadFinished(const String& data) = 0;
};

typedef KListMap<String, String> NetworkMap;
class NetworkBase
{
public:
	enum HTTPMethod {
		GET = 0,
		POST,
	};

	NetworkBase() {}
	virtual ~NetworkBase() {}

public:
	virtual LVoid loadUrl(const String& url, NetworkClient* client) = 0;
	virtual LVoid loadUrl(const String& url, NetworkClient* client, bool isWait) = 0;
	virtual LVoid postData(const String& url, NetworkClient* client, bool isWait) = 0;
	virtual LVoid cancel() = 0;
	virtual LVoid putParam(const String& key, const String& value)
	{
		m_params.put(key, value);
	}

	virtual LVoid putHeader(const String& key, const String& value)
	{
		m_headers.put(key, value);
	}

	void clearHeaders()
	{
		m_headers.clear();
	}

	void clearParams()
	{
		m_params.clear();
	}

protected:
	NetworkMap m_params;
	NetworkMap m_headers;
};

}
#endif
