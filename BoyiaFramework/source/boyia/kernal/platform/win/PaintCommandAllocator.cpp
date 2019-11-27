#include "PaintCommandAllocator.h"

namespace util {
const LInt kPaintCommandDefaultCapacity = 2048;

PaintCommandAllocator::PaintCommandAllocator()
    : m_cmds(kBoyiaNull)
{
    initAllocator();
}

PaintCommandAllocator::~PaintCommandAllocator()
{
    if (m_cmds) {
        delete[] m_cmds;
    }
}

LVoid PaintCommandAllocator::initAllocator()
{
    if (m_cmds) {
        delete[] m_cmds;
    }
    m_cmds = new PaintCommand[kPaintCommandDefaultCapacity];
}

PaintCommand* PaintCommandAllocator::alloc() const
{
    for (LInt i = 0; i < kPaintCommandDefaultCapacity; ++i) {
        if (!m_cmds[i].inUse) {
            return &m_cmds[i];
        }
    }

    return NULL;
}
}