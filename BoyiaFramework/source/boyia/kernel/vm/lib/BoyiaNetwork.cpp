#include "BoyiaNetwork.h"
#include "AppManager.h"
#include "BaseThread.h"
#include "BoyiaAsyncEvent.h"
#include "BoyiaLib.h"
#include "UIView.h"

namespace boyia {
BoyiaNetwork::BoyiaNetwork(BoyiaValue* callback, BoyiaValue* obj, BoyiaRuntime* runtime)
    : BoyiaAsyncEvent(obj, runtime)
{
    ValueCopy(&m_callback, callback);
}

BoyiaNetwork::~BoyiaNetwork()
{
}

LVoid BoyiaNetwork::load(const String& url)
{
    yanbo::AppManager::instance()->network()->loadUrl(url, this, LFalse);
}

LVoid BoyiaNetwork::load(const String& url, const String& params)
{
    JSONParser parser(params, LFalse);
    cJSON* header = parser.get("header");
    cJSON* body = parser.get("body");
    const char* method = parser.get("method")->valuestring;
    
    yanbo::NetworkBase* network = yanbo::AppManager::instance()->network();
    // 设置请求头
    if (header) {
        cJSON* child = header->child;
        while (child) {
            network->putHeader(_CS(child->string), _CS(child->valuestring));
            child = child->next;
        }
    }
    
    if (strcmp(method, "get") == 0) {
        network->loadUrl(url, this, LFalse);
    } else if (strcmp(method, "post") == 0) {
        network->setPostData(new String(_CS(body->valuestring)));
        network->postData(url, this, LFalse);
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
    yanbo::AppManager::instance()->uiThread()->sendUIEvent(this);
}

LVoid BoyiaNetwork::callback()
{
    BOYIA_LOG("BoyiaNetwork::onLoadFinished %d", 1);
    BoyiaValue value;
    CreateNativeString(&value,
        (LInt8*)m_data->GetBuffer(), m_data->GetLength(), m_runtime->vm());
    BOYIA_LOG("BoyiaNetwork::onLoadFinished, data=%s", (const char*)m_data->GetBuffer());
    // 释放字符串控制权
    m_data->ReleaseBuffer();
    // 保存当前栈
    SaveLocalSize(m_runtime->vm());
    // callback函数压栈
    LocalPush(&m_callback, m_runtime->vm());
    // 参数压栈
    LocalPush(&value, m_runtime->vm());
    BoyiaValue* obj = m_obj.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_obj;
    // 调用callback函数
    NativeCall(obj, m_runtime->vm());
}
}
