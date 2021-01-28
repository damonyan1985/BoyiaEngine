#include "BoyiaBase.h"
#include "BoyiaLib.h"
#include "LEvent.h"
#include "PlatformLib.h"
#include "SalLog.h"
#include "AppManager.h"

extern LVoid GCAppendRef(LVoid* address, LUint8 type, LVoid* vm);
// C++对象垃圾回收基类
namespace boyia {
BoyiaBase::BoyiaBase(BoyiaRuntime* runtime)
    : m_type(0)
    , m_runtime(runtime)
    , m_gcFlag(0)
{
    BoyiaValue value;
    value.mValueType = BY_NAVCLASS;
    value.mValue.mIntVal = (LIntPtr)this;
    // 放入临时变量中进行存储
    // 则不会再GC线程中被误清除
    SetNativeResult(&value, m_runtime->vm());
    GCAppendRef(this, BY_NAVCLASS, m_runtime->vm());
}

BoyiaBase::~BoyiaBase()
{
}

BoyiaRuntime* BoyiaBase::runtime() const
{
    return m_runtime;
}

LVoid BoyiaBase::onPressDown(LVoid* view)
{
    if (m_type & LTouchEvent::ETOUCH_DOWN) {
        KLOG("BoyiaBase::onPressDown");
        // 处理touchdown
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_DOWN >> 1];
        SaveLocalSize(m_runtime->vm());
        LocalPush(val, m_runtime->vm());
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj, m_runtime->vm());
    }
}
LVoid BoyiaBase::onPressMove(LVoid* view)
{
    if (m_type & LTouchEvent::ETOUCH_MOVE) {
        // 处理touchmove
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_MOVE >> 1];

        SaveLocalSize(m_runtime->vm());
        LocalPush(val, m_runtime->vm());
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj, m_runtime->vm());
    }
}

LVoid BoyiaBase::onPressUp(LVoid* view)
{
    KFORMATLOG("BoyiaBase::onPressUp m_type=%d ETOUCH_UP=%d", m_type, LTouchEvent::ETOUCH_UP);
    if (m_type & LTouchEvent::ETOUCH_UP) {
        // 处理touchup
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_UP >> 1];
        SaveLocalSize(m_runtime->vm());
        LocalPush(val, m_runtime->vm());
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj, m_runtime->vm());
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

LVoid BoyiaBase::setGcFlag(LInt flag)
{
    m_gcFlag = flag;
}

LInt BoyiaBase::gcFlag() const
{
    return m_gcFlag;
}
}
