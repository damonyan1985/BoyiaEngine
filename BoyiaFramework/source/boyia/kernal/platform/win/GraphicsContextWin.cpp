#include "GraphicsContextWin.h"
#include <GdiPlus.h>

namespace util {
GraphicsContextWin::GraphicsContextWin()
	: m_hwnd(0)
    , m_cmds(0, 1024)
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
    for (LInt i = 0; i < m_cmds.size(); i++) {
        switch (m_cmds[i]->type) {
        case PaintCommand::kPaintRect: {
            Gdiplus::Rect rect(
                m_cmds[i]->rect.iTopLeft.iX,
                m_cmds[i]->rect.iTopLeft.iY,
                m_cmds[i]->rect.GetWidth(),
                m_cmds[i]->rect.GetHeight()
            );

            Gdiplus::SolidBrush brush(Gdiplus::Color(
                m_cmds[i]->color.m_alpha,
                m_cmds[i]->color.m_red,
                m_cmds[i]->color.m_green,
                m_cmds[i]->color.m_blue
            ));
            gc.FillRectangle(&brush, rect);
        }   break;
        case PaintCommand::kPaintImage:
            break;
        case PaintCommand::kPaintCircle:
            break;
        case PaintCommand::kPaintText:
            break;
        }
    }
	::ReleaseDC(m_hwnd, dc);
}

LGraphicsContext* LGraphicsContext::create() 
{
	return new GraphicsContextWin();
}
}