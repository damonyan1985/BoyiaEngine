#ifndef RenderLayer_h
#define RenderLayer_h

#include "RenderCommand.h"
#include "KVector.h"
#include "OwnerPtr.h"
#include "HashMap.h"
#include "HashUtil.h"

namespace yanbo {
using RenderCommandBuffer = KVector<BoyiaPtr<RenderCommand>>;
using CollectBufferMap = HashMap<HashPtr, BoyiaPtr<RenderCommandBuffer>>;
class RenderLayer {
public:
    ~RenderLayer();
    LInt layerId() const;
    LVoid addChild(RenderLayer* layer);
    LVoid setCommand(RenderCommandBuffer* buffer);

    static LVoid clearBuffer(KVector<LUintPtr>* buffers);

public:
    LInt m_layerId;
    KVector<OwnerPtr<RenderLayer>> m_children;
    RenderCommandBuffer* m_buffer;
    // 使用hashmap回收command buffer
    static CollectBufferMap s_collectBufferMap;
};
}

#endif

