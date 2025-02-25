#include "BoyiaNetwork.h"
#include "BaseThread.h"
#include "BoyiaAsyncEvent.h"
#include "BoyiaLib.h"
#include "BoyiaError.h"
#include "JSONParser.h"

namespace boyia {
BoyiaNetwork::BoyiaNetwork(BoyiaValue* callback, BoyiaValue* obj, BoyiaRuntime* runtime)
    : BoyiaAsyncEvent(obj, runtime)
    , m_loader(yanbo::NetworkBase::create())
{
    ValueCopy(&m_callback, callback);
}

BoyiaNetwork::~BoyiaNetwork()
{
}

LVoid BoyiaNetwork::load(const String& url)
{
    m_loader->loadUrl(url, this, LFalse);
}

LVoid BoyiaNetwork::load(const String& url, const String& params)
{
    JSONParser parser(params, JSONParser::kSourceJsonText);
    cJSON* header = parser.get("headers");
    cJSON* body = parser.get("body");
    const char* method = parser.get("method")->valuestring;
    
    yanbo::NetworkBase* network = m_loader.get();
    // 清空请求头
    network->clearHeaders();
    // 设置请求头, header被要求是对象
    if (header && header->type == cJSON_Object) {
        cJSON* child = header->child;
        while (child) {
            network->putHeader(_CS(child->string), _CS(child->valuestring));
            child = child->next;
        }
    }
    
    if (strcmp(method, "get") == 0) {
        network->loadUrl(url, this, LFalse);
    } else if (strcmp(method, "post") == 0) {
        // 只有存在body时才发送请求
        if (body && body->type == cJSON_String) {
            BOYIA_LOG("BoyiaNetwork::load post body=%s", body->valuestring);
            network->setPostData(new String(_CS(body->valuestring)));
            network->postData(url, this, LFalse);
        }
    }
}

LVoid BoyiaNetwork::onDataReceived(const LByte* data, LInt size)
{
    LByte* destData = new LByte[size];
    LMemcpy(destData, data, size);
    m_builder.append(destData, 0, size, LFalse);
}

LVoid BoyiaNetwork::onStatusCode(LInt statusCode)
{
}

LVoid BoyiaNetwork::onFileLen(LInt len)
{
}

LVoid BoyiaNetwork::onRedirectUrl(const String& redirectUrl)
{
}

LVoid BoyiaNetwork::onLoadError(LInt error)
{
    delete this;
}

LVoid BoyiaNetwork::onLoadFinished()
{
    m_data = m_builder.toString();
}

LVoid BoyiaNetwork::callback()
{    
    BoyiaAsyncEvent::callbackString(*m_data.get(), &m_callback, m_runtime);
}
}
