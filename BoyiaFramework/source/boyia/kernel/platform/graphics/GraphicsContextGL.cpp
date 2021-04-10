#include "GraphicsContextGL.h"

#if ENABLE(BOYIA_ANDROID)
#include "GLContext.h"
#include "HtmlView.h"
#include "ImageAndroid.h"
#include "JNIUtil.h"
#include "MatrixState.h"
#include "MediaPlayerAndroid.h"
#include "ShaderUtil.h"
#include "TextView.h"
#include "TextureCache.h"
#include "UIView.h"
#include <GLES3/gl3.h>
#include <android/bitmap.h>
#include <stdlib.h>
//#include <GLES2/gl2ext.h>

namespace util {

typedef KList<BoyiaPtr<yanbo::GLPainter>> ListPainter;
// ItemPainter顾名思义，一个HtmlView对应的所有GLPainter
class ItemPainter : public BoyiaRef {
public:
    ItemPainter()
        : item(kBoyiaNull)
    {
    }

    ~ItemPainter()
    {
    }

    ListPainter painters;
    LVoid* item;
};
GraphicsContextGL::GraphicsContextGL()
    : m_item(kBoyiaNull)
    , m_clipRect(kBoyiaNull)
{
}

GraphicsContextGL::~GraphicsContextGL()
{
    m_context.destroyGL();
}

LVoid GraphicsContextGL::setContextWin(LVoid* win)
{
    m_context.setWindow(win);
}

LVoid GraphicsContextGL::clipRect(const LRect& rect)
{
    m_clipRect = (LRect*)&rect;
}

LVoid GraphicsContextGL::save()
{
}

LVoid GraphicsContextGL::restore()
{
    m_clipRect = kBoyiaNull;
}

LVoid GraphicsContextGL::setHtmlView(ViewPainter* item)
{
    m_item = item;
    ItemPainter* painter = currentPainter();
    painter->painters.clear();
}

ItemPainter* GraphicsContextGL::currentPainter()
{
    ViewPainter* item = (ViewPainter*)m_item;
    ItemPainter* painter = (ItemPainter*)item->painter();
    if (!painter) {
        painter = new ItemPainter();
        painter->item = item;
        item->setPainter(painter);
    }

    return painter;
}

LVoid GraphicsContextGL::drawLine(const LPoint& p1, const LPoint& p2)
{
    ItemPainter* painter = currentPainter();
    BoyiaPtr<yanbo::GLPainter> line = new yanbo::GLPainter();
    line->setLine(p1, p2);
    line->setColor(m_penColor);
    painter->painters.push(line);
}

LVoid GraphicsContextGL::drawLine(LInt x0, LInt y0, LInt x1, LInt y1)
{
    drawLine(LPoint(x0, y0), LPoint(x1, y1));
}

LVoid GraphicsContextGL::drawRect(const LRect& aRect)
{
    ItemPainter* painter = currentPainter();
    BoyiaPtr<yanbo::GLPainter> shapeRect = new yanbo::GLPainter();
    shapeRect->setRect(aRect);
    //m_brushColor.m_alpha = 0x33;
    shapeRect->setColor(m_brushColor);
    painter->painters.push(shapeRect);
}

LVoid GraphicsContextGL::drawRect(LInt x, LInt y, LInt w, LInt h)
{
    drawRect(LRect(x, y, w, h));
}

LVoid GraphicsContextGL::drawEllipse(const LRect& aRect)
{
}

LVoid GraphicsContextGL::drawRoundRect(const LRect& aRect, const LSize& aCornerSize)
{
}

LVoid GraphicsContextGL::drawText(const String& aText, const LRect& aRect)
{
}

LVoid GraphicsContextGL::drawText(const String& aText, const LPoint& aPoint)
{
}

LVoid GraphicsContextGL::drawImage(const LPoint& aTopLeft, const LImage* aBitmap)
{
}

LVoid GraphicsContextGL::drawImage(const LImage* image)
{
    if (!image) {
        return;
    }

    yanbo::Texture* tex = yanbo::TextureCache::getInst()->putImage(image);

    ItemPainter* painter = currentPainter();
    BoyiaPtr<yanbo::GLPainter> paint = new yanbo::GLPainter();
    paint->setColor(m_brushColor);
    //paint->setImage(tex, image->rect(), *m_clipRect);
    if (m_clipRect) {
        paint->setImage(tex, image->rect(), *m_clipRect);
    } else {
        paint->setImage(tex, image->rect());
    }
    painter->painters.push(paint);

    KLOG("GraphicsContextGL::drawBitmap end");
}

LVoid GraphicsContextGL::drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect)
{
}

LVoid GraphicsContextGL::drawVideo(const LRect& rect, const LMediaPlayer* mp)
{
    MediaPlayerAndroid* amp = (MediaPlayerAndroid*)mp;
    if (!amp->texId())
        return;

    BoyiaPtr<yanbo::Texture> tex = new yanbo::Texture();
    tex->width = rect.GetWidth();
    tex->height = rect.GetHeight();
    tex->texId = amp->texId();

    //KFORMATLOG("GraphicsContextGL drawVideo error=%d", glGetError());

    ItemPainter* painter = currentPainter();
    BoyiaPtr<yanbo::GLPainter> paint = new yanbo::GLPainter();
    paint->setColor(LRgb(0, 0, 0, 0xFF));

    paint->setVideo(tex.get(), rect);
    amp->updateTexture(paint->stMatrix());
    //KFORMATLOG("GraphicsContextGL drawVideo error=%d texId=%d", glGetError(), tex->texId);

    painter->painters.push(paint);
}

LVoid GraphicsContextGL::drawText(const String& text, const LRect& rect, TextAlign align)
{
    BoyiaPtr<ImageAndroid> image = new ImageAndroid();
    image->drawText(text, rect, m_font, m_penColor);
    image->unlockPixels();
    image->setRect(rect);
    //image->setItem((yanbo::HtmlView*)m_item);

    yanbo::Texture* tex = yanbo::TextureCache::getInst()->findText((ViewPainter*)m_item);
    if (!tex) {
        tex = yanbo::TextureCache::getInst()->createText((ViewPainter*)m_item, image.get(), 0);
    } else {
        yanbo::TextureCache::getInst()->updateText(tex, image.get());
    }

    ItemPainter* painter = currentPainter();
    BoyiaPtr<yanbo::GLPainter> paint = new yanbo::GLPainter();
    paint->setColor(m_brushColor);
    if (m_clipRect) {
        paint->setImage(tex, rect, *m_clipRect);
    } else {
        paint->setImage(tex, rect);
    }

    painter->painters.push(paint);

    //drawImage(image.get());
}

LVoid GraphicsContextGL::setBrushStyle(BrushStyle aBrushStyle)
{
}

LVoid GraphicsContextGL::setPenStyle(PenStyle aPenStyle)
{
}

LVoid GraphicsContextGL::setBrushColor(const LRgb& aColor)
{
    m_brushColor = aColor;
}

LVoid GraphicsContextGL::setPenColor(const LRgb& aColor)
{
    m_penColor = aColor;
}

LVoid GraphicsContextGL::setFont(const LFont& font)
{
    m_font = font;
}

LVoid GraphicsContextGL::reset()
{
    yanbo::TextureCache::getInst()->clear();
    //GLContext::initGLContext(GLContext::EWindow);
    m_context.initGL(GLContext::EWindow);
    glViewport(0, 0, m_context.viewWidth(), m_context.viewHeight());
    yanbo::ShaderUtil::setRealScreenSize(m_context.viewWidth(), m_context.viewHeight());
    yanbo::MatrixState::init();

    //glViewport(0, 0, width, height);
    //LReal32 ratio = (LReal32) width / height;
    //LReal32 ratio = (LReal32) height / width;
    LReal32 ratio = 1.0f;
    // 设置透视投影
    yanbo::MatrixState::setProjectFrustum(-1.0f * ratio, 1.0f * ratio, -1.0f, 1.0f, 1.0f, 50);
    //yanbo::MatrixState::setProjectOrtho(-1.0f * ratio, 1.0f * ratio, -1.0f, 1.0f, -1.0f, 1.0f);

    yanbo::MatrixState::setCamera(0, 0, 1, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    yanbo::MatrixState::copyMVMatrix();

    yanbo::MatrixState::setInitStack();

    yanbo::GLPainter::init();
}

LVoid GraphicsContextGL::fillBuffer(LVoid* ptr)
{
    yanbo::HtmlView* item = (yanbo::HtmlView*)ptr;
    // 如果是文本，则使用linetext来绘制
    if (item->isText()) {
        yanbo::TextView* text = static_cast<yanbo::TextView*>(item);
        for (LInt i = 0; i < text->lineSize(); i++) {
            ItemPainter* painter = static_cast<ItemPainter*>(text->linePainter(i)->painter());
            (*painter->painters.begin())->appendToBuffer();
        }

        return;
    }

    ItemPainter* painter = (ItemPainter*)item->painter();
    if (!painter) {
        return;
    }

    ListPainter::Iterator glIter = painter->painters.begin();
    ListPainter::Iterator glIterEnd = painter->painters.end();

    for (; glIter != glIterEnd; ++glIter) {
        (*glIter)->appendToBuffer();
    }

    yanbo::HtmlViewList::Iterator iter = item->m_children.begin();
    yanbo::HtmlViewList::Iterator iterEnd = item->m_children.end();

    for (; iter != iterEnd; ++iter) {
        yanbo::HtmlView* childItem = *iter;
        fillBuffer(childItem);
    }
}

LVoid GraphicsContextGL::submit()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    yanbo::GLPainter::reset();
    fillBuffer(yanbo::UIView::current()->getDocument()->getRenderTreeRoot());
    yanbo::GLPainter::bindVBO();
    glEnable(GL_BLEND);
    yanbo::GLPainter::paintCommand();
    glDisable(GL_BLEND);

    yanbo::GLPainter::unbindVBO();

    m_context.postBuffer();
}

LGraphicsContext* LGraphicsContext::create()
{
    return new GraphicsContextGL();
}
}

#endif;
