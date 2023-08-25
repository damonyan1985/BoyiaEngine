#include "RenderEngineAndroid.h"
#include "PixelRatio.h"

#include "JNIUtil.h"
#include "MatrixState.h"
#include "TextureCache.h"
#include "ShaderUtil.h"
#include "ImageAndroid.h"

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
    BoyiaPtr<GLPainter> painter = new GLPainter();
    painter->setRect(cmd->rect);
    painter->setColor(cmd->color);
    m_painters.push(painter);
}

LVoid RenderEngineAndroid::renderText(RenderCommand* cmd)
{
    RenderTextCommand* textCmd = static_cast<RenderTextCommand*>(cmd);
    LRect& rect = textCmd->rect;
    LColor& color = textCmd->color;
    LFont& textFont = textCmd->font;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }

    BoyiaPtr<util::ImageAndroid> image = new util::ImageAndroid();
    image->drawText(textCmd->text, rect, textFont, color);
    image->unlockPixels();
    image->setRect(rect);

    Texture* tex = TextureCache::getInst()->putImage(image.get());

    BoyiaPtr<GLPainter> paint = new GLPainter();
    paint->setColor(textCmd->color);
    paint->setImage(tex, rect);

    m_painters.push(paint);
}

LVoid RenderEngineAndroid::renderImage(RenderCommand* cmd)
{
    RenderImageCommand* imageCmd = static_cast<RenderImageCommand*>(cmd);
    LRect& rect = imageCmd->rect;
    LColor& color = imageCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    Texture* tex = TextureCache::getInst()->putImage(imageCmd->url, imageCmd->image, rect.GetWidth(), rect.GetHeight());

    BoyiaPtr<GLPainter> paint = new GLPainter();
    paint->setColor(color);
    paint->setImage(tex, rect);

    m_painters.push(paint);
}

LVoid RenderEngineAndroid::renderRoundRect(RenderCommand* cmd)
{
    RenderRoundRectCommand* roundCmd = static_cast<RenderRoundRectCommand*>(cmd);
    
    LRect& rect = roundCmd->rect;
    LColor& color = roundCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }

    BoyiaPtr<GLPainter> painter = new GLPainter();
    painter->setRoundRect(rect, 
        roundCmd->topLeftRadius, 
        roundCmd->topRightRadius, 
        roundCmd->bottomRightRadius, 
        roundCmd->bottomLeftRadius);
    painter->setColor(color);
    m_painters.push(painter);
}

LVoid RenderEngineAndroid::renderVideo(RenderCommand* cmd)
{
    RenderVideoCommand* videoCmd = static_cast<RenderVideoCommand*>(cmd);
    LRect& rect = videoCmd->rect;
    LColor& color = videoCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }

    LIntPtr texId = videoCmd->playerId;
    yanbo::Texture* texture = TextureCache::getInst()->findExternal((LUintPtr)texId);
    if (!texture) {
        texture = TextureCache::getInst()->createExternal(texId, rect.GetWidth(), rect.GetHeight());
        texture->attach(texId);
    }

    BoyiaPtr<GLPainter> paint = new GLPainter();
    paint->setColor(LColor(0, 0, 0, 0xFF));

    paint->setExternal(texture, rect);
    m_painters.push(paint);    

    // 获取矩阵变换数组
    jfloatArray arr = (jfloatArray)JNIUtil::callStaticObjectMethod(
        "com/boyia/app/core/BoyiaBridge",
        "updateTexture",
        "(J)[F", (jlong)texId);

    if (arr) {
        JNIEnv* env = JNIUtil::getEnv();
        int count = env->GetArrayLength(arr);
        jfloat* ptr = env->GetFloatArrayElements(arr, JNI_FALSE);   
        LMemcpy(paint->stMatrix(), ptr, sizeof(float) * count); 
        env->ReleaseFloatArrayElements(arr, ptr, 0);
        env->DeleteLocalRef(arr);
    }
}

LVoid RenderEngineAndroid::renderRoundImage(RenderCommand* cmd)
{
    RenderRoundImageCommand* imageCmd = static_cast<RenderRoundImageCommand*>(cmd);
    LRect& rect = imageCmd->rect;
    LColor& color = imageCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }

    Texture* tex = TextureCache::getInst()->putImage(imageCmd->url, imageCmd->image, rect.GetWidth(), rect.GetHeight());

    BoyiaPtr<GLPainter> paint = new GLPainter();
    paint->setColor(color);
    paint->setImage(tex, rect, rect, 
            imageCmd->topLeftRadius, 
            imageCmd->topRightRadius, 
            imageCmd->bottomRightRadius, 
            imageCmd->bottomLeftRadius);
    m_painters.push(paint);
}

LVoid RenderEngineAndroid::renderPlatform(RenderCommand* cmd)
{
}

IRenderEngine* IRenderEngine::create()
{
    return new RenderEngineAndroid();
}
}