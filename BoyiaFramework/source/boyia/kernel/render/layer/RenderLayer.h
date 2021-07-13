#ifndef RenderLayer_h
#define RenderLayer_h

#include "RenderCommand.h"
#include "KVector.h"

namespace yanbo {
// RenderLayer与HtmlView一一对应，表示渲染层面的节点
// 每个RenderLayer保存一组由HtmlView paint时产生的渲染指令
class RenderLayer : public BoyiaRef {
public:
    LInt layerId();
    LVoid addChild(RenderLayer* layer);

private:
    LInt m_layerId;
    // layer的次序
    LInt m_layerOrder;

    KVector<BoyiaPtr<RenderLayer>> m_children;
    KVector<BoyiaPtr<RenderCommand>> m_buffer;
};
}

#endif

