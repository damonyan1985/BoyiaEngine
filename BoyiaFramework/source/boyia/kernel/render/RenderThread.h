#ifndef RenderThread_h
#define RenderThread_h

#include "MessageThread.h"
#include "IRenderEngine.h"

namespace yanbo {
// 渲染与UI逻辑进行分离
class RenderThread : public MessageThread {
public:
    enum MessageType {
        kRenderInit = 1,
        kRenderReset,
        kRenderLayerTree,
    };
    static RenderThread* instance();

    RenderThread();
    virtual LVoid handleMessage(Message* msg);
    
    LVoid renderInit();
    LVoid renderReset();
    LVoid renderLayerTree(RenderLayer* rootLayer);

    IRenderEngine* getRenderer() const;

private:
    IRenderEngine* m_renderer;
};
}

#endif
