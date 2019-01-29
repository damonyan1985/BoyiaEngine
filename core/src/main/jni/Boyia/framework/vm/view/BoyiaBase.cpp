#include "BoyiaBase.h"
#include "PlatformLib.h"
#include "LEvent.h"
#include "SalLog.h"
#include "BoyiaLib.h"

extern LVoid GCAppendRef(LVoid* address, LUint8 type);
// C++对象垃圾回收基类
namespace boyia
{
BoyiaBase::BoyiaBase()
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

BoyiaBase::~BoyiaBase()
{
}

void BoyiaBase::onPressDown(void* view)
{
    if (m_type & LTouchEvent::ETOUCH_DOWN)
    {
    	KLOG("BoyiaBase::onPressDown");
        // 处理touchdown
    	BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_DOWN >> 1];
    	SaveLocalSize();
    	LocalPush(val);
    	BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? NULL : &m_boyiaView;
    	NativeCall(obj);
    }
}
void BoyiaBase::onPressMove(void* view)
{
	if (m_type & LTouchEvent::ETOUCH_MOVE)
	{
		// 处理touchmove
		BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_MOVE >> 1];

		SaveLocalSize();
		LocalPush(val);
		BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? NULL : &m_boyiaView;
		NativeCall(obj);
	}
}

void BoyiaBase::onPressUp(void* view)
{
	KFORMATLOG("BoyiaBase::onPressUp m_type=%d ETOUCH_UP=%d", m_type, LTouchEvent::ETOUCH_UP);
	if (m_type & LTouchEvent::ETOUCH_UP)
	{
		// 处理touchup
    	BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_UP >> 1];
    	SaveLocalSize();
    	LocalPush(val);
    	BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? NULL : &m_boyiaView;
    	NativeCall(obj);
	}
}

void BoyiaBase::onKeyDown(int keyCode, void* view)
{

}

void BoyiaBase::onKeyUp(int keyCode, void* view)
{

}

void BoyiaBase::addListener(LInt type, BoyiaValue* callback)
{
    m_type |= type;
    KFORMATLOG("BoyiaBase::addListener m_type=%d", m_type);
    ValueCopy(&m_callbacks[type >> 1], callback);
}

void BoyiaBase::setBoyiaView(BoyiaValue* value)
{
	ValueCopy(&m_boyiaView, value);
}
}
