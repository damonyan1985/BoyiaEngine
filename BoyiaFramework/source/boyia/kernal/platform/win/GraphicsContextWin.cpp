#include "GraphicsContextWin.h"
#include "HtmlView.h"
#include <GdiPlus.h>

namespace util {
class ItemPainter {
public:
    ItemPainter()
        : cmds(0, 20)
        , item(kBoyiaNull)
    {
    }
    KVector<PaintCommand*> cmds;
    LVoid* item;
};
GraphicsContextWin::GraphicsContextWin()
	: m_hwnd(0)
    , m_item(kBoyiaNull)
    , m_painters(0, 1024)
{
}

GraphicsContextWin::~GraphicsContextWin()
{
    
}

LVoid GraphicsContextWin::setContextWin(HWND hwnd)
{
	m_hwnd = hwnd;
}

LVoid GraphicsContextWin::restore()
{
}

LVoid GraphicsContextWin::reset()
{
}

LVoid GraphicsContextWin::save()
{
}

LVoid GraphicsContextWin::drawLine(const LPoint& p1, const LPoint& p2)
{
}

LVoid GraphicsContextWin::drawLine(LInt x0, LInt y0, LInt x1, LInt y1)
{
}

LVoid GraphicsContextWin::drawRect(const LRect& aRect)
{
    ItemPainter* painter = currentPainter();
    PaintCommand* cmd = PaintCommandAllocator::instance()->alloc();
    painter->cmds.addElement(cmd);
}

LVoid GraphicsContextWin::drawRect(LInt x, LInt y, LInt w, LInt h)
{
}

LVoid GraphicsContextWin::drawEllipse(const LRect& aRect)
{
}

LVoid GraphicsContextWin::drawRoundRect(const LRect& aRect, const LSize& aCornerSize)
{
}

LVoid GraphicsContextWin::setHtmlView(LVoid* item)
{
    m_item = item;
    currentPainter()->cmds.clear();
}

ItemPainter* GraphicsContextWin::currentPainter()
{
    yanbo::HtmlView* item = (yanbo::HtmlView*)m_item;
    ItemPainter* painter = (ItemPainter*)item->painter();
    if (!painter) {
        painter = new ItemPainter();
        painter->item = item;
        item->setPainter(painter);
    }

    return painter;
}

LVoid GraphicsContextWin::drawText(const String& aText, const LRect& aRect)
{
}

LVoid GraphicsContextWin::drawText(const String& aText, const LPoint& aPoint)
{
}

LVoid GraphicsContextWin::drawImage(const LPoint& aTopLeft, const LImage* aBitmap)
{
}

LVoid GraphicsContextWin::drawImage(const LImage* image)
{
}

LVoid GraphicsContextWin::drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect)
{
}

LVoid GraphicsContextWin::setBrushStyle(BrushStyle aBrushStyle)
{
}

LVoid GraphicsContextWin::setPenStyle(PenStyle aPenStyle)
{
}

LVoid GraphicsContextWin::setBrushColor(const LRgb& aColor)
{
}

LVoid GraphicsContextWin::setPenColor(const LRgb& aColor)
{
}

LVoid GraphicsContextWin::setFont(const LFont& font)
{
}

LVoid GraphicsContextWin::drawVideo(const LRect& rect, const LMediaPlayer* mp)
{
}

LVoid GraphicsContextWin::drawText(const String& text, const LRect& rect, TextAlign align)
{
}

LVoid GraphicsContextWin::clipRect(const LRect& rect)
{
}

LVoid GraphicsContextWin::submit()
{
	HDC dc = ::GetDC(m_hwnd);
    Gdiplus::Graphics gc(m_hwnd);
    for (LInt i = 0; i < m_painters.size(); i++) {
        for (LInt n = 0; n < m_painters[i]->cmds.size(); n++) {
            m_painters[i]->cmds[n]->paint(gc);
        }
    }
	::ReleaseDC(m_hwnd, dc);
}

LGraphicsContext* LGraphicsContext::create() 
{
	return new GraphicsContextWin();
}
}