#include "RenderLayer.h"

namespace yanbo {
CollectBufferMap RenderLayer::s_collectBufferMap;

LVoid RenderLayer::clearBuffer(KVector<LUintPtr>* buffers, LBool needDelete)
{
    if (!buffers) {
        return;
    }

    for (LInt i = 0; i < buffers->size(); i++) {
        s_collectBufferMap.remove(HashPtr(buffers->elementAt(i)));
    }

    if (needDelete) {
        delete buffers;
    }
}

RenderLayer::~RenderLayer()
{
    s_collectBufferMap.put(HashPtr((LUintPtr)m_buffer), m_buffer);
}

LInt RenderLayer::layerId() const
{
    return m_layerId;
}

LVoid RenderLayer::addChild(RenderLayer* layer)
{
    m_children.addElement(layer);
}

LVoid RenderLayer::setCommand(RenderCommandBuffer* buffer)
{
    m_buffer = buffer;
}

}