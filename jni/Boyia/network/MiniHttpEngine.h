#ifndef MiniHttpEngine_h
#define MiniHttpEngine_h

//#include "UtilString.h"
#include "NetworkBase.h"
#include "StringBuffer.h"
#include "curl/curl.h"

namespace yanbo
{
class HttpCallback
{
public:
	virtual LVoid onStatusCode(LInt code) = 0;
	virtual LVoid onContentLength(LInt length) = 0;
	virtual LVoid  onDataRecevied(const LCharA* buffer, LInt size) = 0;
	virtual LVoid onLoadFinished() = 0;
	virtual LVoid onLoadError(LInt code) = 0;
};
class MiniHttpEngine
{
public:
	enum EMethod
	{
		EHTTP_GET,
		EHTTP_POST
	};

	MiniHttpEngine(HttpCallback* callback);
	~MiniHttpEngine();

	LVoid setHeader(const NetworkMap& headers);
    LVoid request(const String& url, LInt method);

    static size_t writeCallback(LVoid* buffer, size_t size, size_t membyte, LVoid* param);

private:
    CURL*           m_curl;
    HttpCallback*   m_callback;
    LInt            m_size;
};
}

#endif
