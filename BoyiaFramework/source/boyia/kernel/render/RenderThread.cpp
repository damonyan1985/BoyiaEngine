#include "RenderThread.h"

namespace yanbo {
RenderThread::RenderThread()
    : m_renderer(IRenderEngine::create())
{
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
        m_renderer->render(kBoyiaNull);
    } break;
    default: {
    } break;
    }
}
}