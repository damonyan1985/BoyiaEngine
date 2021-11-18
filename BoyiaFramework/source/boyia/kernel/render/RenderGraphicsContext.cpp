#include "RenderGraphicsContext.h"
#include "TextView.h"
#include "UIView.h"
#include "PixelRatio.h"

namespace yanbo {
class ItemPainter : public BoyiaRef {
public:
    ItemPainter()
        : buffer(new RenderCommandBuffer(0, 20))
    {
    }

    // 每次重绘创建RenderCommandBuffer，并将旧buffer放入回收器
    // 回收器交给渲染线程来进行管理
    LVoid clear(KVector<LUintPtr>* collector)
    {
        if (collector) {
            collector->addElement((LUintPtr)buffer);
        }
        buffer = new RenderCommandBuffer(0, 20);
    }

    RenderCommandBuffer* buffer;
};

RenderGraphicsContext::RenderGraphicsContext()
    : m_clipRect(kBoyiaNull)
    , m_collectBuffers(new KVector<LUintPtr>(0, 20))
{
}

RenderGraphicsContext::~RenderGraphicsContext()
{
}

ItemPainter* RenderGraphicsContext::currentPainter()
{
    ViewPainter* item = (ViewPainter*)m_item;
    ItemPainter* painter = (ItemPainter*)item->painter();
    if (!painter) {
        painter = new ItemPainter();
        item->setPainter(painter);
    }

    return painter;
}

LVoid RenderGraphicsContext::drawLine(const LPoint& p1, const LPoint& p2)
{
}

LVoid RenderGraphicsContext::drawLine(LInt x0, LInt y0, LInt x1, LInt y1)
{
}

LVoid RenderGraphicsContext::drawRect(const LRect& aRect)
{
//    if (!PixelRatio::isInWindow(aRect)) {
//        return;
//    }
    LRect destRect = aRect;
    if (m_clipRect) {
        // 如果绘制区域不在裁剪范围内
        if (!PixelRatio::isInClipRect(aRect, *m_clipRect)) {
            return;
        }
        
        PixelRatio::clipRect(aRect, *m_clipRect, destRect);
    }
    
    ItemPainter* painter = currentPainter();
    RenderRectCommand* cmd = new RenderRectCommand(destRect, m_brushColor);

    painter->buffer->addElement(cmd);
}

LVoid RenderGraphicsContext::drawRect(LInt x, LInt y, LInt w, LInt h)
{
    drawRect(LRect(x, y, w, h));
}

LVoid RenderGraphicsContext::drawEllipse(const LRect& aRect)
{
}

LVoid RenderGraphicsContext::drawRoundRect(const LRect& aRect, const LSize& aCornerSize)
{
}

LVoid RenderGraphicsContext::drawRoundRect(const LRect& aRect, LInt topLeftRadius, LInt topRightRadius, LInt bottomRightRadius, LInt bottomLeftRadius)
{
    if (!PixelRatio::isInWindow(aRect)) {
        return;
    }
    
    ItemPainter* painter = currentPainter();
    RenderRoundRectCommand* cmd =
        new RenderRoundRectCommand(aRect, m_brushColor, topLeftRadius, topRightRadius, bottomRightRadius, bottomLeftRadius);

    painter->buffer->addElement(cmd);
}

LVoid RenderGraphicsContext::drawText(const String& aText, const LRect& aRect)
{
}
LVoid RenderGraphicsContext::drawText(const String& aText, const LPoint& aPoint)
{
}

LVoid RenderGraphicsContext::drawImage(const LPoint& aTopLeft, const LImage* aBitmap)
{
}

LVoid RenderGraphicsContext::drawImage(const LImage* image)
{
    if (!image->pixels() || !PixelRatio::isInWindow(image->rect())) {
        return;
    }
    
    ItemPainter* painter = currentPainter();
    RenderImageCommand* cmd = new RenderImageCommand(image->rect(), m_brushColor, image->pixels());
    cmd->url = image->url();
    painter->buffer->addElement(cmd);
}

LVoid RenderGraphicsContext::drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect)
{
}

LVoid RenderGraphicsContext::drawVideo(const LRect& rect, const LMediaPlayer* mp)
{
}

#if ENABLE(BOYIA_PLATFORM_VIEW)
LVoid RenderGraphicsContext::drawPlatform(const LRect& rect, LVoid* platformView)
{
}
#endif

LVoid RenderGraphicsContext::drawText(const String& text, const LRect& rect, TextAlign align)
{
    if (!PixelRatio::isInWindow(rect)) {
        return;
    }
    
    ItemPainter* painter = currentPainter();
    
    RenderTextCommand* cmd = new RenderTextCommand(rect, m_penColor, m_font, text);
    painter->buffer->addElement(cmd);
}

LVoid RenderGraphicsContext::setBrushStyle(BrushStyle aBrushStyle)
{
}
LVoid RenderGraphicsContext::setPenStyle(PenStyle aPenStyle)
{
}

LVoid RenderGraphicsContext::setBrushColor(const LColor& aColor)
{
    m_brushColor = aColor;
}

LVoid RenderGraphicsContext::setPenColor(const LColor& aColor)
{
    m_penColor = aColor;
}

LVoid RenderGraphicsContext::setFont(const LFont& font)
{
    m_font = font;
}

LVoid RenderGraphicsContext::reset()
{
}

LVoid RenderGraphicsContext::submit(LVoid* view, RenderLayer* parentLayer)
{
    yanbo::HtmlView* item = (yanbo::HtmlView*)view;
    
    if (!item->canDraw()) {
        return;
    }

    if (item->isText()) {
        yanbo::TextView* text = static_cast<yanbo::TextView*>(item);
        
        for (LInt i = 0; i < text->lineSize(); i++) {
            RenderLayer* layer = new RenderLayer();
            LVoid* painter = text->linePainter(i)->painter();
            if (!painter) {
                continue;
            }
            
            layer->setCommand(static_cast<ItemPainter*>(painter)->buffer);
            parentLayer->addChild(layer);
        }

        return;
    }

    ItemPainter* painter = (ItemPainter*)item->painter();
    if (!painter) {
        return;
    }

    RenderLayer* layer = new RenderLayer();
    layer->setCommand(painter->buffer);
    parentLayer->addChild(layer);

    yanbo::HtmlViewList::Iterator iter = item->m_children.begin();
    yanbo::HtmlViewList::Iterator iterEnd = item->m_children.end();

    for (; iter != iterEnd; ++iter) {
        submit(*iter, layer);
    }
}

LVoid RenderGraphicsContext::submit()
{
     // 生成RenderLayer树，并提交给渲染线程
    RenderLayer* layer = new RenderLayer();
    submit(yanbo::UIView::current()->getDocument()->getRenderTreeRoot(), layer);
    RenderThread::instance()->renderLayerTree(layer, m_collectBuffers);
    // 每次提交后重置回收器
    m_collectBuffers = new KVector<LUintPtr>(0, 20);
}

// 设置setHtmlView表示需要重新绘制，因而需要清空buffer
LVoid RenderGraphicsContext::setHtmlView(ViewPainter* item)
{
    m_item = item;
    currentPainter()->clear(m_collectBuffers);
}

LVoid RenderGraphicsContext::save()
{
}

LVoid RenderGraphicsContext::clipRect(const LRect& rect)
{
    m_clipRect = (LRect*)&rect;
}

LVoid RenderGraphicsContext::restore()
{
    m_clipRect = kBoyiaNull;
}

}

LGraphicsContext* LGraphicsContext::create()
{
    return new yanbo::RenderGraphicsContext();
}
