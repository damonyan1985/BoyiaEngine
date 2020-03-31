#include "BoyiaBase.h"
#include "BoyiaLib.h"
#include "LEvent.h"
#include "PlatformLib.h"
#include "SalLog.h"

extern LVoid GCAppendRef(LVoid* address, LUint8 type);
// C++对象垃圾回收基类
namespace boyia {
BoyiaBase::BoyiaBase()
    : m_type(0)
{
    BoyiaValue value;
    value.mValueType = BY_NAVCLASS;
    value.mValue.mIntVal = (LIntPtr)this;
    // 放入临时变量中进行存储
    // 则不会再GC线程中被误清除
    SetNativeResult(&value);
    GCAppendRef(this, BY_NAVCLASS);
}

BoyiaBase::~BoyiaBase()
{
}

LVoid BoyiaBase::onPressDown(LVoid* view)
{
    if (m_type & LTouchEvent::ETOUCH_DOWN) {
        KLOG("BoyiaBase::onPressDown");
        // 处理touchdown
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_DOWN >> 1];
        SaveLocalSize();
        LocalPush(val);
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj);
    }
}
LVoid BoyiaBase::onPressMove(LVoid* view)
{
    if (m_type & LTouchEvent::ETOUCH_MOVE) {
        // 处理touchmove
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_MOVE >> 1];

        SaveLocalSize();
        LocalPush(val);
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj);
    }
}

LVoid BoyiaBase::onPressUp(LVoid* view)
{
    KFORMATLOG("BoyiaBase::onPressUp m_type=%d ETOUCH_UP=%d", m_type, LTouchEvent::ETOUCH_UP);
    if (m_type & LTouchEvent::ETOUCH_UP) {
        // 处理touchup
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_UP >> 1];
        SaveLocalSize();
        LocalPush(val);
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj);
    }
}

LVoid BoyiaBase::onKeyDown(LInt keyCode, LVoid* view)
{
}

LVoid BoyiaBase::onKeyUp(LInt keyCode, LVoid* view)
{
}

LVoid BoyiaBase::addListener(LInt type, BoyiaValue* callback)
{
    m_type |= type;
    KFORMATLOG("BoyiaBase::addListener m_type=%d", m_type);
    ValueCopy(&m_callbacks[type >> 1], callback);
}

LVoid BoyiaBase::setBoyiaView(BoyiaValue* value)
{
    ValueCopy(&m_boyiaView, value);
}
}