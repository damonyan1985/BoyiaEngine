#include "BoyiaLoader.h"
#include "MiniThreadPool.h"

namespace yanbo
{
LoaderTask::LoaderTask()
    : m_method(BoyiaHttpEngine::EHTTP_GET)
    , m_engine(this)
    , m_client(NULL)
{
}

LVoid LoaderTask::setMethod(LInt method)
{
	m_method = method;
}

LVoid LoaderTask::setUrl(const String& url)
{
    m_url = url;
}

LVoid LoaderTask::setClient(NetworkClient* client)
{
	m_client = client;
}

LVoid LoaderTask::execute()
{
	m_engine.request(m_url, m_method);
}

LVoid LoaderTask::onStatusCode(LInt code)
{
	if (m_client) m_client->onStatusCode(code);
}

LVoid LoaderTask::onContentLength(LInt length)
{
	if (m_client) m_client->onFileLen(length);
}

LVoid LoaderTask::onDataRecevied(const LByte* buffer, LInt size)
{
	if (m_client) m_client->onDataReceived(buffer, size);
}

LVoid LoaderTask::onLoadFinished()
{
	if (m_client) m_client->onLoadFinished(_CS(""));
}

LVoid LoaderTask::onLoadError(LInt code)
{
	if (m_client) m_client->onLoadError(code);
}

BoyiaLoader::BoyiaLoader()
{
	curl_global_init(CURL_GLOBAL_ALL);
	start();
}

LVoid BoyiaLoader::loadUrl(const String& url, NetworkClient* client)
{
	loadUrl(url, client, true);
}

LVoid BoyiaLoader::loadUrl(const String& url, NetworkClient* client, bool isWait)
{
    LoaderTask* task = new LoaderTask();
    // Set Task Info
    task->setUrl(url);
    task->setClient(client);

	if (isWait)
	{
		// SendMessage
		MiniMessage* msg = m_queue->obtain();
		msg->type = ELOAD_URL;
		msg->obj = task;
		m_queue->push(msg);
		notify();
	}
	else
	{
		MiniThreadPool::getInstance()->sendMiniTask(task);
	}
}

LVoid BoyiaLoader::postData(const String& url, NetworkClient* client, bool isWait)
{

}

LVoid BoyiaLoader::handleMessage(MiniMessage* msg)
{
    switch (msg->type)
    {
    case ELOAD_URL:
        {
        	LoaderTask* task = (LoaderTask*) msg->obj;
        	task->execute();
        	delete task;
        }
        break;
    }
}

LVoid BoyiaLoader::cancel()
{
	
}
}
