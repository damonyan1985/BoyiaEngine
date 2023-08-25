#include "RenderEngineAndroid.h"
#include "PixelRatio.h"

#include "JNIUtil.h"
#include "MatrixState.h"
#include "TextureCache.h"
#include "ShaderUtil.h"
#include "GLPainter.h"

#ifdef OPENGLES_3
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

namespace yanbo {
static inline LInt RealLength(LInt length) 
{
    return (LInt)(length * yanbo::PixelRatio::ratio());
}

RenderEngineAndroid::RenderEngineAndroid()
{
}

RenderEngineAndroid::~RenderEngineAndroid()
{
}

LVoid RenderEngineAndroid::init()
{
    // 初始化渲染函数
    m_functions[RenderCommand::kRenderRect] = (RenderFunction)&RenderEngineAndroid::renderRect;
    m_functions[RenderCommand::kRenderText] = (RenderFunction)&RenderEngineAndroid::renderText;
    m_functions[RenderCommand::kRenderImage] = (RenderFunction)&RenderEngineAndroid::renderImage;
    m_functions[RenderCommand::kRenderRoundRect] = (RenderFunction)& RenderEngineAndroid::renderRoundRect;
    m_functions[RenderCommand::kRenderVideo] = (RenderFunction)&RenderEngineAndroid::renderVideo;
    m_functions[RenderCommand::kRenderRoundImage] = (RenderFunction)&RenderEngineAndroid::renderRoundImage;
    m_functions[RenderCommand::kRenderPlatform] = (RenderFunction)&RenderEngineAndroid::renderPlatform;

    m_context.initGL(util::GLContext::EWindow);
}

LVoid RenderEngineAndroid::reset()
{
    TextureCache::getInst()->clear();

    glViewport(0, 0, m_context.viewWidth(), m_context.viewHeight());
    ShaderUtil::setRealScreenSize(m_context.viewWidth(), m_context.viewHeight());
    MatrixState::init();

    LReal32 ratio = 1.0f;
    // 设置透视投影
    MatrixState::setProjectFrustum(-1.0f * ratio, 1.0f * ratio, -1.0f, 1.0f, 1.0f, 50);
    //yanbo::MatrixState::setProjectOrtho(-1.0f * ratio, 1.0f * ratio, -1.0f, 1.0f, -1.0f, 1.0f);

    MatrixState::setCamera(0, 0, 1, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    MatrixState::copyMVMatrix();

    MatrixState::setInitStack();

    GLPainter::init();
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

LVoid RenderEngineAndroid::renderRoundImage(RenderCommand* cmd)
{
}

LVoid RenderEngineAndroid::renderPlatform(RenderCommand* cmd)
{

}

IRenderEngine* IRenderEngine::create()
{
    return new RenderEngineAndroid();
}
}