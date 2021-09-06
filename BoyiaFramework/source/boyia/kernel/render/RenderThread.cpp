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
        RenderLayer::clearBuffer(reinterpret_cast<KVector<LUintPtr>*>(msg->arg0));
    } break;
    default: {
    } break;
    }
}

LVoid RenderThread::renderInit()
{
    Message* msg = m_queue->obtain();
    msg->type = kRenderInit;

    m_queue->push(msg);
    notify();
}

LVoid RenderThread::renderReset()
{
    Message* msg = m_queue->obtain();
    msg->type = kRenderReset;

    m_queue->push(msg);
    notify();
}

LVoid RenderThread::renderLayerTree(RenderLayer* rootLayer, KVector<LUintPtr>* collector)
{
    Message* msg = m_queue->obtain();
    msg->type = kRenderLayerTree;
    msg->obj = rootLayer;
    msg->arg0 = (LIntPtr)collector;

    m_queue->push(msg);
    notify();
}

IRenderEngine* RenderThread::getRenderer() const
{
    return m_renderer;
}

}