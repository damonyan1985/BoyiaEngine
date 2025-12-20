#include "RenderLayer.h"

namespace yanbo {

RenderLayer::~RenderLayer()
{
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