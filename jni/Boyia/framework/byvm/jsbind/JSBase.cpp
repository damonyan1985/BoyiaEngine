#include "JSBase.h"
#include "PlatformLib.h"
#include "LEvent.h"
#include "SalLog.h"
#include "BoyiaLib.h"

extern LVoid GCAppendRef(LVoid* address, LUint8 type);
// C++对象垃圾回收基类
namespace boyia
{
JSBase::JSBase()
    : m_type(0)
{
	BoyiaValue value;
	value.mValueType = BY_NAVCLASS;
	value.mValue.mIntVal = (LIntPtr) this;
	// 放入临时变量中进行存储
	// 则不会再GC线程中被误清除
	SetNativeResult(&value);
	GCAppendRef(this, BY_NAVCLASS);
}

JSBase::~JSBase()
{
}

void JSBase::onPressDown(void* view)
{
    if (m_type & LTouchEvent::ETOUCH_DOWN)
    {
    	KLOG("JSBase::onPressDown");
        // 处理touchdown
    	BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_DOWN >> 1];
    	SaveLocalSize();
    	LocalPush(val);
    	BoyiaValue* obj = m_jsView.mValue.mObj.mPtr == 0 ? NULL : &m_jsView;
    	NativeCall(obj);
    }
}
void JSBase::onPressMove(void* view)
{
	if (m_type & LTouchEvent::ETOUCH_MOVE)
	{
		// 处理touchmove
		BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_MOVE >> 1];

		SaveLocalSize();
		LocalPush(val);
		BoyiaValue* obj = m_jsView.mValue.mObj.mPtr == 0 ? NULL : &m_jsView;
		NativeCall(obj);
	}
}

void JSBase::onPressUp(void* view)
{
	KFORMATLOG("JSBase::onPressUp m_type=%d ETOUCH_UP=%d", m_type, LTouchEvent::ETOUCH_UP);
	if (m_type & LTouchEvent::ETOUCH_UP)
	{
		// 处理touchup
    	BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_UP >> 1];
    	SaveLocalSize();
    	LocalPush(val);
    	BoyiaValue* obj = m_jsView.mValue.mObj.mPtr == 0 ? NULL : &m_jsView;
    	NativeCall(obj);
	}
}

void JSBase::onKeyDown(int keyCode, void* view)
{

}

void JSBase::onKeyUp(int keyCode, void* view)
{

}

void JSBase::addListener(LInt type, BoyiaValue* callback)
{
    m_type |= type;
    KFORMATLOG("JSBase::addListener m_type=%d", m_type);
    ValueCopy(&m_callbacks[type >> 1], callback);
}

void JSBase::setJSView(BoyiaValue* value)
{
	ValueCopy(&m_jsView, value);
}
}
