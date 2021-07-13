#ifndef RenderCommandBuffer_h
#define RenderCommandBuffer_h

#include "HashMap.h"
#include "HashUtil.h"
#include "RenderLayer.h"
#include "WeakPtr.h"
#include "BoyiaPtr.h"

namespace yanbo {
using RenderLayerMap = HashMap<HashInt, WeakPtr<RenderLayer>>;
// 渲染指令缓冲
class RenderCommandBuffer {
public:
    // 渲染指令
    LVoid render();
    
    // 添加layer
    LVoid addLayer(LUint parentId, RenderLayer* layer);
    // 修改某一节点上的Layer
    LVoid updateLayer(RenderLayer* layer);
    // 删除节点时删除对应的Layer
    LVoid deleteLayer(LUint parentId, LUint layerId);
    
private:
    // 保存
    RenderLayerMap m_layerMap;
    BoyiaPtr<RenderLayer> m_root;
};
}
#endif
