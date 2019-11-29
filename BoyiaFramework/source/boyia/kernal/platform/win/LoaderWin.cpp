#include "LoaderWin.h"

#if ENABLE(BOYIA_WINDOWS)
#include "BoyiaHttpEngine.h"
#include "FileUtil.h"
#include "PlatformBridge.h"
#include "SalLog.h"
#include "TaskBase.h"
#include "ThreadPool.h"

namespace yanbo {
class HttpTask : public TaskBase, HttpCallback {
public:
    HttpTask(NetworkClient* client)
        : m_method(NetworkBase::GET)
        , m_engine(this)
        , m_client(client)
    {
    }

    virtual void execute()
    {
        m_engine.request(m_url, m_method);
    }

    LVoid setHeader(const NetworkMap& map)
    {
        m_engine.setHeader(map);
    }

    LVoid setMethod(LInt method)
    {
        m_method = method;
    }

    LVoid setUrl(const String& url)
    {
        m_url = url;
        BOYIA_LOG("boyia app HttpTask::setUrl url=%s", GET_STR(m_url));
    }

    LVoid setPostData(const OwnerPtr<String>& data)
    {
        m_engine.setPostData(data);
    }

    virtual LVoid onStatusCode(LInt code)
    {
        if (m_client)
            m_client->onStatusCode(code);
    }

    virtual LVoid onContentLength(LInt length)
    {
        if (m_client)
            m_client->onFileLen(length);
    }

    virtual LVoid onDataReceived(const LByte* buffer, LInt size)
    {
        if (m_client)
            m_client->onDataReceived(buffer, size);
    }

    virtual LVoid onLoadFinished()
    {
        if (m_client)
            m_client->onLoadFinished();
    }

    virtual LVoid onLoadError(LInt code)
    {
        if (m_client)
            m_client->onLoadError(code);
    }

private:
    LInt m_method;
    String m_url;
    BoyiaHttpEngine m_engine;
    NetworkClient* m_client;
};

class FileTask : public TaskBase {
public:
    FileTask(NetworkClient* client)
        : m_client(client)
    {
    }

    LVoid setUrl(const String& url)
    {
        // shallow copy
        m_url.Copy(url.GetBuffer(), LTrue, url.GetLength());
    }

    virtual LVoid execute()
    {
        BOYIA_LOG("FileTask---execute()---%d", 0);
        if (m_client) {
            String content;
            BOYIA_LOG("FileTask---execute()---%s", GET_STR(m_url));
            FileUtil::readFile(m_url, content);
            BOYIA_LOG("FileTask---execute()---%s", GET_STR(content));

            m_client->onDataReceived(content.GetBuffer(), content.GetLength());
            BOYIA_LOG("FileTask---execute()---%d", 3);
            m_client->onLoadFinished();
            BOYIA_LOG("FileTask---execute()---%d", 4);
        }
    }

private:
    String m_url;
    NetworkClient* m_client;
};

LoaderWin::LoaderWin()
{
}
LoaderWin::~LoaderWin()
{
}

LVoid LoaderWin::syncLoadUrl(const String& url, String& content)
{
    FileUtil::syncLoadUrl(url, content);
}

LVoid LoaderWin::loadUrl(const String& url, NetworkClient* client)
{
    loadUrl(url, client, LTrue);
}

static TaskBase* loadBoyiaUrl(const String& url, NetworkClient* client)
{
    TaskBase* task = kBoyiaNull;
    const String& schema = FileUtil::fileSchema();
    if (url.StartWith(schema)) {
        String sourceUrl = url.Mid(schema.GetLength());
        String sourcePath = _CS(PlatformBridge::getAppRoot()) + sourceUrl;
        FileTask* fileTask = new FileTask(client);
        fileTask->setUrl(sourcePath);
        sourcePath.ReleaseBuffer();
        task = fileTask;
    }

    return task;
}

LVoid LoaderWin::loadUrl(const String& url, NetworkClient* client, LBool isWait)
{
    TaskBase* task = loadBoyiaUrl(url, client);
    if (!task) {
        HttpTask* httpTask = new HttpTask(client);
        // Set Task Info
        httpTask->setHeader(m_headers);
        httpTask->setUrl(url);
        task = httpTask;
    }

    if (isWait) {
        // SendMessage
        Message* msg = m_queue->obtain();
        msg->type = NetworkBase::GET;
        msg->obj = task;
        m_queue->push(msg);
        notify();
    } else {
        ThreadPool::getInstance()->sendTask(task);
    }
}

LVoid LoaderWin::postData(const String& url, NetworkClient* client)
{
    postData(url, client, LTrue);
}

LVoid LoaderWin::postData(const String& url, NetworkClient* client, LBool isWait)
{
    HttpTask* task = new HttpTask(client);
    // Set Task Info
    task->setHeader(m_headers);
    task->setUrl(url);
    task->setMethod(NetworkBase::POST);
    //task->setUrl(_CS("http://192.168.0.10:8011/user/login"));
    //const char* data = "name=test&pwd=test";
    task->setPostData(m_data);
    //task->setClient(client);

    if (isWait) {
        // SendMessage
        Message* msg = m_queue->obtain();
        msg->type = NetworkBase::POST;
        msg->obj = task;
        m_queue->push(msg);
        notify();
    }
    else {
        ThreadPool::getInstance()->sendTask(task);
    }

    // ÖÃ¿ÕÊý¾ÝÖ¸Õë
    m_data = NULL;
}

LVoid LoaderWin::cancel()
{
}

LVoid LoaderWin::handleMessage(Message* msg)
{
    switch (msg->type) {
    case NetworkBase::GET:
    case NetworkBase::POST: {
        BOYIA_LOG("BoyiaLoader---handleMessage()---%d", 0);
        TaskBase* task = (TaskBase*)msg->obj;
        BOYIA_LOG("BoyiaLoader---handleMessage()---%d", 1);
        BOYIA_LOG("BoyiaLoader---handleMessage()---task=%d", (LIntPtr)task);
        task->execute();
        BOYIA_LOG("BoyiaLoader---handleMessage()---%d", 2);
        delete task;
        BOYIA_LOG("BoyiaLoader---handleMessage()---%d", 5);
    } break;
    }
}

NetworkBase* NetworkBase::create()
{
	return new LoaderWin();
}
}

#endif