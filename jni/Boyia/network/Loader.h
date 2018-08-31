#ifndef Loader_h
#define Loader_h

#include "MiniHttpEngine.h"
#include "MiniTaskBase.h"
#include "MiniMessageThread.h"
#include "UtilString.h"
#include "NetworkBase.h"

namespace yanbo
{
class LoaderTask : public MiniTaskBase, HttpCallback
{
public:
	LoaderTask();
	virtual void execute();

	LVoid setMethod(LInt method);
	LVoid setUrl(const String& url);
	LVoid setClient(NetworkClient* client);

	virtual LVoid onStatusCode(LInt code);
	virtual LVoid onContentLength(LInt length);
	virtual LVoid onDataRecevied(const LCharA* buffer, LInt size);
	virtual LVoid onLoadFinished();
	virtual LVoid onLoadError(LInt code);

private:
	LInt            m_method;
	String          m_url;
	MiniHttpEngine  m_engine;
	NetworkClient*  m_client;
	StringBuffer    m_buffer;
};

class Loader : public MiniMessageThread
{
public:
	enum LoadMsgType
	{
		ELOAD_URL,
	};
	Loader();

	virtual LVoid loadUrl(const String& url, NetworkClient* client);
	virtual LVoid loadUrl(const String& url, NetworkClient* client, bool isWait);
    virtual LVoid handleMessage(MiniMessage* msg);

private:
    LoaderTask* m_task;
};
}
#endif
