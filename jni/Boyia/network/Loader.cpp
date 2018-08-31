#include "Loader.h"
#include "MiniThreadPool.h"

namespace yanbo
{
LoaderTask::LoaderTask()
    : m_method(MiniHttpEngine::EHTTP_GET)
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

LVoid LoaderTask::onDataRecevied(const LCharA* buffer, LInt size)
{
	m_buffer.append(String(_CS(buffer), LTrue, size));
	//if (m_client) m_client->onDataRecevied(length);
}

LVoid LoaderTask::onLoadFinished()
{
	if (m_client) m_client->onLoadFinished(*m_buffer.toString().get());
}

LVoid LoaderTask::onLoadError(LInt code)
{
	if (m_client) m_client->onLoadError(code);
}

Loader::Loader()
    : m_task(NULL)
{
	curl_global_init(CURL_GLOBAL_ALL);
	start();
}

LVoid Loader::loadUrl(const String& url, NetworkClient* client)
{
	loadUrl(url, client, true);
}

LVoid Loader::loadUrl(const String& url, NetworkClient* client, bool isWait)
{
	if (isWait)
	{
		if (!m_task) m_task = new LoaderTask();
		// Set Task Info
		m_task->setUrl(url);
		m_task->setClient(client);

		// SendMessage
		MiniMessage* msg = m_queue->obtain();
		msg->type = ELOAD_URL;
		msg->obj = m_task;
		m_queue->push(msg);
		notify();
	}
	else
	{
		MiniThreadPool::getInstance()->sendMiniTask(m_task);
	}
}

LVoid Loader::handleMessage(MiniMessage* msg)
{
    switch (msg->type)
    {
    case ELOAD_URL:
        {
        	LoaderTask* task = (LoaderTask*) msg->obj;
        	task->execute();
        }
        break;
    }
}
}
