#include "RefCount.h"

namespace util {
RefCount::RefCount()
    : m_shareCount(0)
    , m_weakCount(0)
{
}

RefCount::~RefCount()
{
}

LVoid RefCount::release()
{
    // 如果引用计数为0，则释放自己
    if (!shareCount() && !weakCount()) {
        delete this;
    }
}

LVoid RefCount::ref()
{
    ++m_shareCount;
}

LVoid RefCount::deref()
{
    --m_shareCount;
}

LInt RefCount::shareCount() const
{
    return m_shareCount;
}

LInt RefCount::weakCount() const
{
    return m_weakCount;
}

LVoid RefCount::attachWeak()
{
    ++m_weakCount;
}

LVoid RefCount::detchWeak()
{
    --m_weakCount;
}

}