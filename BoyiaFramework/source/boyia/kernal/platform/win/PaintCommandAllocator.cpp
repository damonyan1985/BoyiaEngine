#include "PaintCommandAllocator.h"

namespace util {
const LInt kPaintCommandDefaultCapacity = 2000;
const LInt kCommandUnit = 20;
const LInt kCommandSize = kPaintCommandDefaultCapacity / kCommandUnit;

PaintCommandAllocator::PaintCommandAllocator()
    : m_cmds(NULL)
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
    for (LInt i = 0; i < kCommandSize; ++i) {
        if (!m_cmds[i * kCommandUnit].inUse) {
            return &m_cmds[i * kCommandUnit];
        }
    }

    return NULL;
}
}