#include "BoyiaBase.h"
#include "AppManager.h"
#include "BoyiaLib.h"
#include "LEvent.h"
#include "PlatformLib.h"
#include "SalLog.h"

extern LVoid GCAppendRef(LVoid* address, LUint8 type, LVoid* vm);
// C++对象垃圾回收基类
namespace boyia {
BoyiaBase::BoyiaBase(BoyiaRuntime* runtime)
    : m_runtime(runtime)
    , m_gcFlag(0)
{
    if (!m_runtime) {
        return;
    }
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

LVoid BoyiaBase::setGcFlag(LInt flag)
{
    m_gcFlag = flag;
}

LInt BoyiaBase::gcFlag() const
{
    return m_gcFlag;
}
}
