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

LVoid RefCount::ref()
{
    ++m_shareCount;
}

LVoid RefCount::deref()
{
    --m_shareCount;
}

LInt RefCount::shareCount()
{
    return m_shareCount;
}

}