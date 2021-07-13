#include "RenderCommandBuffer.h"

namespace yanbo {
LVoid RenderCommandBuffer::render() 
{

}

LVoid RenderCommandBuffer::addLayer(LUint parentId, RenderLayer* layer)
{
    m_layerMap.put(layer->layerId(), layer);
    m_layerMap.get(parentId)->addChild(layer);
}
}