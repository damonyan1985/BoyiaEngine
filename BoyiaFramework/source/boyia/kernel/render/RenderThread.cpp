#include "RenderThread.h"
#include "RenderLayer.h"

namespace yanbo {
RenderThread::RenderThread()
    : m_renderer(IRenderEngine::create())
{
    start();
}

RenderThread* RenderThread::instance()
{
    static RenderThread renderThread;
    return &renderThread;
}

LVoid RenderThread::handleMessage(Message* msg)
{
    switch (msg->type) {
    case kRenderInit: {
        m_renderer->init();
    } break;
    case kRenderReset: {
        m_renderer->reset();
    } break;
    case kRenderLayerTree: {
        OwnerPtr<RenderLayer> layer = static_cast<RenderLayer*>(msg->obj);
        m_renderer->render(layer);

        if (msg->arg0) {
            clearCommandBuffer(reinterpret_cast<KVector<LUintPtr>*>(msg->arg0));
        }
    } break;
    default: {
    } break;
    }
}

LVoid RenderThread::renderInit()
{
    Message* msg = obtain();
    msg->type = kRenderInit;
    postMessage(msg);
}

LVoid RenderThread::renderReset()
{
    Message* msg = obtain();
    msg->type = kRenderReset;

    postMessage(msg);
}

LVoid RenderThread::renderLayerTree(RenderLayer* rootLayer, KVector<LUintPtr>* collector)
{
    Message* msg = obtain();
    msg->type = kRenderLayerTree;
    msg->obj = rootLayer;
    msg->arg0 = (LIntPtr)collector;
    postMessage(msg);
}

LVoid RenderThread::clearCommandBuffer(KVector<LUintPtr>* buffer) {
    for (LInt i = 0; i < buffer->size(); i++) {
        delete reinterpret_cast<RenderCommandBuffer*>(buffer->elementAt(i));
    }

    delete buffer;
}

IRenderEngine* RenderThread::getRenderer() const
{
    return m_renderer;
}

}
