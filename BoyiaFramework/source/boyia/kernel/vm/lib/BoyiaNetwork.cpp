#include "BoyiaNetwork.h"
#include "AppManager.h"
#include "BaseThread.h"
#include "BoyiaLib.h"
#include "UIView.h"

namespace boyia {
BoyiaNetwork::BoyiaNetwork(BoyiaValue* callback, BoyiaValue* obj, LVoid* vm)
    : m_vm(vm)
{
    ValueCopy(&m_callback, callback);
    ValueCopy(&m_obj, obj);
}

BoyiaNetwork::~BoyiaNetwork()
{
}

LVoid BoyiaNetwork::load(const String& url)
{
    yanbo::UIView::getInstance()->network()->loadUrl(url, this, LFalse);
}

LVoid BoyiaNetwork::onDataReceived(const LByte* data, LInt size)
{
    LByte* destData = new LByte[size];
    util::LMemcpy(destData, data, size);
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

LVoid BoyiaNetwork::run()
{
    BOYIA_LOG("BoyiaNetwork::onLoadFinished %d", 1);
    BoyiaValue value;
    value.mValueType = BY_STRING;
    value.mValue.mStrVal.mPtr = (LInt8*)m_data->GetBuffer();
    value.mValue.mStrVal.mLen = m_data->GetLength();
    BOYIA_LOG("BoyiaNetwork::onLoadFinished, data=%s", (const char*)m_data->GetBuffer());
    SaveLocalSize(m_vm);
    LocalPush(&m_callback, m_vm);
    LocalPush(&value, m_vm);
    BoyiaValue* obj = m_obj.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_obj;
    NativeCall(obj, m_vm);
}
}
