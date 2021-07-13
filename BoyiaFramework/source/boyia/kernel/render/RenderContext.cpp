#include "RenderContext.h"
#include "HtmlView.h"

namespace yanbo {
RenderContext::RenderContext()
    : m_message(kBoyiaNull)
{
}

LVoid RenderContext::drawLine(const LPoint& p1, const LPoint& p2)
{
}

LVoid RenderContext::drawLine(LInt x0, LInt y0, LInt x1, LInt y1)
{
}

LVoid RenderContext::drawRect(const LRect& aRect)
{
    //HtmlView* view = static_cast<HtmlView*>(m_item);
    //RenderRectCommand* cmd = new RenderRectCommand(aRect, m_color);
    //message()->commandMap.put(view->viewId(), cmd);
}

LVoid RenderContext::drawRect(LInt x, LInt y, LInt w, LInt h)
{
    drawRect(LRect(x, y, w, h));
}

LVoid RenderContext::drawEllipse(const LRect& aRect)
{
}

LVoid RenderContext::drawRoundRect(const LRect& aRect, const LSize& aCornerSize)
{
}

LVoid RenderContext::drawText(const String& aText, const LRect& aRect)
{
}
LVoid RenderContext::drawText(const String& aText, const LPoint& aPoint)
{
}

LVoid RenderContext::drawImage(const LPoint& aTopLeft, const LImage* aBitmap)
{
}
LVoid RenderContext::drawImage(const LImage* image)
{
}
LVoid RenderContext::drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect)
{
}

LVoid RenderContext::drawVideo(const LRect& rect, const LMediaPlayer* mp)
{
}
#if ENABLE(BOYIA_PLATFORM_VIEW)
LVoid RenderContext::drawPlatform(const LRect& rect, LVoid* platformView)
{
}
#endif

LVoid RenderContext::drawText(const String& text, const LRect& rect, TextAlign align)
{
}
LVoid RenderContext::setBrushStyle(BrushStyle aBrushStyle)
{
}
LVoid RenderContext::setPenStyle(PenStyle aPenStyle)
{
}

LVoid RenderContext::setBrushColor(const LColor& aColor)
{
    m_color = aColor;
}
LVoid RenderContext::setPenColor(const LColor& aColor)
{
}

LVoid RenderContext::setFont(const LFont& font)
{
}

LVoid RenderContext::reset()
{
}

LVoid RenderContext::submit()
{
}

LVoid RenderContext::setHtmlView(ViewPainter* item)
{
    m_item = item;
    m_layer = new RenderLayer();
}

LVoid RenderContext::save()
{
}
LVoid RenderContext::clipRect(const LRect& rect)
{
}
LVoid RenderContext::restore()
{
}
}