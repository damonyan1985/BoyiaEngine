#include "RenderEngineAndroid.h"
#include "PixelRatio.h"

namespace yanbo {
static inline LInt RealLength(LInt length) 
{
    return (LInt)(length * yanbo::PixelRatio::ratio());
}

RenderEngineAndroid::RenderEngineAndroid()
{
    init();
}

RenderEngineAndroid::~RenderEngineAndroid()
{
}

LVoid RenderEngineWin::init()
{
    // 初始化渲染函数
    m_functions[RenderCommand::kRenderRect] = (RenderFunction)&RenderEngineAndroid::renderRect;
    m_functions[RenderCommand::kRenderText] = (RenderFunction)&RenderEngineAndroid::renderText;
    m_functions[RenderCommand::kRenderImage] = (RenderFunction)&RenderEngineAndroid::renderImage;
    m_functions[RenderCommand::kRenderRoundRect] = (RenderFunction)& RenderEngineAndroid::renderRoundRect;
    m_functions[RenderCommand::kRenderVideo] = (RenderFunction)&RenderEngineAndroid::renderVideo;
    m_functions[RenderCommand::kRenderRoundImage] = (RenderFunction)&RenderEngineAndroid::renderRoundImage;
}

LVoid RenderEngineAndroid::reset()
{
}

LVoid RenderEngineAndroid::render(RenderLayer* layer)
{
}

LVoid RenderEngineAndroid::renderImpl(RenderLayer* layer)
{
    if (!layer) {
        return;
    }

    // 先渲染当前layer
    if (layer->m_buffer) {
        LInt commandSize = layer->m_buffer->size();
        for (LInt i = 0; i < commandSize; i++) {
            RenderCommand* cmd = layer->m_buffer->elementAt(i).get();
            (this->*(m_functions[cmd->type()]))(cmd);
        }
    }
    
    // 再渲染子layer
    for (LInt i = 0; i < layer->m_children.size(); i++) {
        renderImpl(layer->m_children[i]);
    }
}

LVoid RenderEngineAndroid::renderRect(RenderCommand* cmd)
{
}

LVoid RenderEngineAndroid::renderText(RenderCommand* cmd)
{
}

LVoid RenderEngineAndroid::renderImage(RenderCommand* cmd)
{
}

LVoid RenderEngineAndroid::renderRoundRect(RenderCommand* cmd)
{
}

LVoid RenderEngineAndroid::renderVideo(RenderCommand* cmd)
{
}

LVoid RenderEnRenderEngineAndroidgineWin::renderRoundImage(RenderCommand* cmd)
{
}

IRenderEngine* IRenderEngine::create()
{
    return new RenderEngineAndroid();
}
}