#include "BoyiaLoader.h"
#include "MiniThreadPool.h"

namespace yanbo {
LoaderTask::LoaderTask()
    : m_method(NetworkBase::GET)
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
    KFORMATLOG("boyia app LoaderTask::setUrl url=%s", GET_STR(m_url));
}

LVoid LoaderTask::setClient(NetworkClient* client)
{
    m_client = client;
}

LVoid LoaderTask::setPostData(const BoyiaPtr<String>& data)
{
    m_engine.setPostData(data);
}

LVoid LoaderTask::setHeader(const NetworkMap& map)
{
    m_engine.setHeader(map);
}

LVoid LoaderTask::execute()
{
    m_engine.request(m_url, m_method);
}

LVoid LoaderTask::onStatusCode(LInt code)
{
    if (m_client)
        m_client->onStatusCode(code);
}

LVoid LoaderTask::onContentLength(LInt length)
{
    if (m_client)
        m_client->onFileLen(length);
}

LVoid LoaderTask::onDataRecevied(const LByte* buffer, LInt size)
{
    if (m_client)
        m_client->onDataReceived(buffer, size);
}

LVoid LoaderTask::onLoadFinished()
{
    if (m_client)
        m_client->onLoadFinished();
}

LVoid LoaderTask::onLoadError(LInt code)
{
    if (m_client)
        m_client->onLoadError(code);
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

LVoid BoyiaLoader::loadUrl(const String& url, NetworkClient* client, LBool isWait)
{
    LoaderTask* task = new LoaderTask();
    // Set Task Info
    task->setHeader(m_headers);
    task->setUrl(url);
    task->setClient(client);

    if (isWait) {
        // SendMessage
        MiniMessage* msg = m_queue->obtain();
        msg->type = ELOAD_URL;
        msg->obj = task;
        m_queue->push(msg);
        notify();
    } else {
        MiniThreadPool::getInstance()->sendMiniTask(task);
    }
}

LVoid BoyiaLoader::postData(const String& url, NetworkClient* client)
{
    postData(url, client, LTrue);
}

LVoid BoyiaLoader::postData(const String& url, NetworkClient* client, LBool isWait)
{
    LoaderTask* task = new LoaderTask();
    // Set Task Info
    task->setHeader(m_headers);
    task->setUrl(url);
    task->setMethod(NetworkBase::POST);
    //task->setUrl(_CS("http://192.168.0.10:8011/user/login"));
    //const char* data = "name=test&pwd=test";
    task->setPostData(m_data);
    task->setClient(client);

    if (isWait) {
        // SendMessage
        MiniMessage* msg = m_queue->obtain();
        msg->type = ELOAD_URL;
        msg->obj = task;
        m_queue->push(msg);
        notify();
    } else {
        MiniThreadPool::getInstance()->sendMiniTask(task);
    }

    // 置空数据指针
    m_data = NULL;
}

LVoid BoyiaLoader::handleMessage(MiniMessage* msg)
{
    switch (msg->type) {
    case ELOAD_URL: {
        LoaderTask* task = (LoaderTask*)msg->obj;
        task->execute();
        delete task;
    } break;
    }
}

LVoid BoyiaLoader::cancel()
{
}
}