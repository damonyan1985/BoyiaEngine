#ifndef RenderLayer_h
#define RenderLayer_h

#include "RenderCommand.h"
#include "KVector.h"
#include "OwnerPtr.h"

namespace yanbo {
using RenderCommandBuffer = KVector<BoyiaPtr<RenderCommand>>;
class RenderLayer : public BoyiaRef {
public:
    LInt layerId() const;
    LVoid addChild(RenderLayer* layer);
    LVoid setCommand(RenderCommandBuffer* buffer);

public:
    LInt m_layerId;
    KVector<BoyiaPtr<RenderLayer>> m_children;
    //OwnerPtr<RenderCommandBuffer> m_buffer;
    RenderCommandBuffer* m_buffer;
};
}

#endif

