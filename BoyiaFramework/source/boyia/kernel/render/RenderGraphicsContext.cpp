#include "RenderGraphicsContext.h"
#include "TextView.h"
#include "UIView.h"

namespace yanbo {
class ItemPainter : public BoyiaRef {
public:
    ItemPainter()
        : buffer(new RenderCommandBuffer(0, 20))
    {
    }

    // UI线程不管理buffer释放
    LVoid clear()
    {
        buffer = new RenderCommandBuffer(0, 20);
    }

    RenderCommandBuffer* buffer;
};

RenderGraphicsContext::RenderGraphicsContext()
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
    ItemPainter* painter = currentPainter();
    RenderRectCommand* cmd = new RenderRectCommand(aRect, m_color);

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
}
LVoid RenderGraphicsContext::setBrushStyle(BrushStyle aBrushStyle)
{
}
LVoid RenderGraphicsContext::setPenStyle(PenStyle aPenStyle)
{
}

LVoid RenderGraphicsContext::setBrushColor(const LColor& aColor)
{
    m_color = aColor;
}
LVoid RenderGraphicsContext::setPenColor(const LColor& aColor)
{
}

LVoid RenderGraphicsContext::setFont(const LFont& font)
{
}

LVoid RenderGraphicsContext::reset()
{
}

LVoid RenderGraphicsContext::submit(LVoid* view, RenderLayer* parentLayer)
{
    // 组装RenderLayer树
    yanbo::HtmlView* item = (yanbo::HtmlView*)view;

    if (item->isText()) {
        yanbo::TextView* text = static_cast<yanbo::TextView*>(item);
        
        for (LInt i = 0; i < text->lineSize(); i++) {
            RenderLayer* layer = new RenderLayer();
            layer->setCommand(static_cast<ItemPainter*>(text->linePainter(i)->painter())->buffer);
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
    RenderLayer* layer = new RenderLayer();
    submit(yanbo::UIView::current()->getDocument()->getRenderTreeRoot(), layer);
}

LVoid RenderGraphicsContext::setHtmlView(ViewPainter* item)
{
    m_item = item;
    currentPainter()->clear();
}

LVoid RenderGraphicsContext::save()
{
}
LVoid RenderGraphicsContext::clipRect(const LRect& rect)
{
}
LVoid RenderGraphicsContext::restore()
{
}
}