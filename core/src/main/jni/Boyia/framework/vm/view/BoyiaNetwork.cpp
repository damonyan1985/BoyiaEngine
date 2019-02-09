#include "BoyiaNetwork.h"
#include "UIView.h"
#include "BoyiaLib.h"

namespace boyia
{
BoyiaNetwork::BoyiaNetwork(BoyiaValue* callback, BoyiaValue* obj)
{
	ValueCopy(&m_callback, callback);
	ValueCopy(&m_obj, obj);
}

BoyiaNetwork::~BoyiaNetwork()
{
}

void BoyiaNetwork::load(const String& url)
{
	yanbo::UIView::getInstance()->network()->loadUrl(url, this, false);
}

void BoyiaNetwork::onDataReceived(const LByte* data, LInt size)
{
	m_builder.append(data, 0, size, LFalse);
}

void BoyiaNetwork::onStatusCode(LInt statusCode)
{
}

void BoyiaNetwork::onFileLen(LInt len)
{
}

void BoyiaNetwork::onRedirectUrl(const String& redirectUrl)
{
}

void BoyiaNetwork::onLoadError(LInt error)
{
    delete this;
}

void BoyiaNetwork::onLoadFinished()
{
	BoyiaPtr<String> sptr = m_builder.toString();
	KFORMATLOG("BoyiaNetwork::onLoadFinished %d", 1);
	BoyiaValue value;
	value.mValueType = BY_STRING;
	value.mValue.mStrVal.mPtr = (LInt8*)sptr->GetBuffer();
	value.mValue.mStrVal.mLen = sptr->GetLength();
	KFORMATLOG("BoyiaNetwork::onLoadFinished, data=%s", (const char*)sptr->GetBuffer());
	SaveLocalSize();
	LocalPush(&m_callback);
	LocalPush(&value);
	BoyiaValue* obj = m_obj.mValue.mObj.mPtr == 0 ? NULL : &m_obj;
	NativeCall(obj);
	delete this;
}

}
