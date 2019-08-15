#include "BoyiaLoader.h"
#include "BoyiaHttpEngine.h"
#include "FileUtil.h"
#include "MiniTaskBase.h"
#include "MiniThreadPool.h"
#include "PlatformBridge.h"
#include "SalLog.h"

namespace yanbo {

const String kSdkPrefix(_CS("boyiasdk://"), LTrue, 11);
const String kSourcePrefix(_CS("boyia://"), LTrue, 8);

class HttpTask : public MiniTaskBase, HttpCallback {
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
        KFORMATLOG("boyia app HttpTask::setUrl url=%s", GET_STR(m_url));
    }

    LVoid setPostData(const BoyiaPtr<String>& data)
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

class FileTask : public MiniTaskBase {
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

BoyiaLoader::BoyiaLoader()
{
    curl_global_init(CURL_GLOBAL_ALL);
    start();
}

LVoid BoyiaLoader::loadUrl(const String& url, NetworkClient* client)
{
    loadUrl(url, client, LTrue);
}

LVoid BoyiaLoader::loadUrl(const String& url, NetworkClient* client, LBool isWait)
{
    MiniTaskBase* task = NULL;
    if (url.StartWith(kSdkPrefix)) {
        String sdkUrl = url.Mid(kSdkPrefix.GetLength());
        String sdkPath = _CS(PlatformBridge::getAppPath()) + sdkUrl;
        FileTask* fileTask = new FileTask(client);
        fileTask->setUrl(sdkPath);
        sdkPath.ReleaseBuffer();
        task = fileTask;
    } else if (url.StartWith(kSourcePrefix)) {
        String sourceUrl = url.Mid(kSourcePrefix.GetLength());
        String sourcePath = _CS(PlatformBridge::getAppRoot()) + sourceUrl;
        FileTask* fileTask = new FileTask(client);
        fileTask->setUrl(sourcePath);
        sourcePath.ReleaseBuffer();
        task = fileTask;
    } else {
        HttpTask* httpTask = new HttpTask(client);
        // Set Task Info
        httpTask->setHeader(m_headers);
        httpTask->setUrl(url);

        task = httpTask;
    }

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
        BOYIA_LOG("BoyiaLoader---handleMessage()---%d", 0);
        MiniTaskBase* task = (MiniTaskBase*)msg->obj;
        BOYIA_LOG("BoyiaLoader---handleMessage()---%d", 1);
        BOYIA_LOG("BoyiaLoader---handleMessage()---task=%d", (LIntPtr)task);
        task->execute();
        BOYIA_LOG("BoyiaLoader---handleMessage()---%d", 2);
        delete task;
        BOYIA_LOG("BoyiaLoader---handleMessage()---%d", 5);
    } break;
    }
}

LVoid BoyiaLoader::cancel()
{
}

NetworkBase* NetworkBase::create()
{
    return new BoyiaLoader();
}
}
