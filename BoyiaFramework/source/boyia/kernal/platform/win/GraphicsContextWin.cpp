#include "GraphicsContextWin.h"
#include "HtmlView.h"
#include "UIView.h"
#include "ImageWin.h"

namespace util {
class ItemPainter : public BoyiaRef {
public:
    ItemPainter()
        : cmds(0, 20)
        , item(kBoyiaNull)
    {
    }

    LVoid clear() 
    {
        for (LInt i = 0; i < cmds.size(); i++) {
            cmds[i]->inUse = LFalse;
        }

        cmds.clear();
    }

    KVector<PaintCommand*> cmds;
    LVoid* item;
};

GraphicsContextWin::GraphicsContextWin()
	: m_hwnd(0)
    , m_item(kBoyiaNull)
    , m_clipRect(kBoyiaNull)
    , m_cacheBitmap(kBoyiaNull)
{
    Gdiplus::GdiplusStartupInput startupInput;
    GdiplusStartup(&m_gdiplusToken, &startupInput, NULL);
}

GraphicsContextWin::~GraphicsContextWin()
{
    if (m_cacheBitmap) {
        delete m_cacheBitmap;
    }
    Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

LVoid GraphicsContextWin::setContextWin(HWND hwnd)
{
	m_hwnd = hwnd;
}

LVoid GraphicsContextWin::restore()
{
    m_clipRect = kBoyiaNull;
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
    cmd->type = PaintCommand::kPaintRect;
    cmd->color = m_brushColor;
    cmd->rect = aRect;
    painter->cmds.addElement(cmd);
}

LVoid GraphicsContextWin::drawRect(LInt x, LInt y, LInt w, LInt h)
{
    drawRect(LRect(x, y, w, h));
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
    currentPainter()->clear();
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
    ItemPainter* painter = currentPainter();
    PaintCommand* cmd = PaintCommandAllocator::instance()->alloc();
    cmd->image = ((ImageWin*)image)->image();
    cmd->type = PaintCommand::kPaintImage;
    cmd->color = m_brushColor;
    cmd->rect = image->rect();
    painter->cmds.addElement(cmd);
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
    m_brushColor = aColor;
}

LVoid GraphicsContextWin::setPenColor(const LRgb& aColor)
{
    m_penColor = aColor;
}

LVoid GraphicsContextWin::setFont(const LFont& font)
{
}

LVoid GraphicsContextWin::drawVideo(const LRect& rect, const LMediaPlayer* mp)
{
}

LVoid GraphicsContextWin::drawText(const String& text, const LRect& rect, TextAlign align)
{
    ItemPainter* painter = currentPainter();
    PaintCommand* cmd = PaintCommandAllocator::instance()->alloc();
    cmd->text = text;
    cmd->type = PaintCommand::kPaintText;
    cmd->color = m_penColor;
    cmd->rect = rect;
    painter->cmds.addElement(cmd);
}

LVoid GraphicsContextWin::clipRect(const LRect& rect)
{
    m_clipRect = (LRect*)& rect;
}

LVoid GraphicsContextWin::paint(LVoid* ptr, Gdiplus::Graphics& gc)
{
    yanbo::HtmlView* item = (yanbo::HtmlView*)ptr;
    ItemPainter* painter = (ItemPainter*)item->painter();
    if (!painter) {
        return;
    }

    KVector<PaintCommand*>& cmds = painter->cmds;

    for (LInt i = 0; i < cmds.size(); i++) {
        cmds[i]->paint(gc);
    }

    yanbo::HtmlViewList::Iterator iter = item->m_children.begin();
    yanbo::HtmlViewList::Iterator iterEnd = item->m_children.end();

    for (; iter != iterEnd; ++iter) {
        paint(*iter, gc);
    }
}

LVoid GraphicsContextWin::submit()
{
    if (!m_cacheBitmap) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        m_cacheBitmap = new Gdiplus::Bitmap(int(rc.right), int(rc.bottom));
    }

    Gdiplus::Graphics cacheGc(m_cacheBitmap);
    paint(yanbo::UIView::getInstance()->getDocument()->getRenderTreeRoot(), cacheGc);
    
    HDC dc = ::GetDC(m_hwnd);
    Gdiplus::Graphics gc(m_hwnd);
    Gdiplus::CachedBitmap cachedBmp(m_cacheBitmap, &gc);
    gc.DrawCachedBitmap(&cachedBmp, 0, 0);
    ::ReleaseDC(m_hwnd, dc);
}

HWND GraphicsContextWin::hwnd() const
{
    return m_hwnd;
}

LVoid GraphicsContextWin::repaint() const
{
    if (m_cacheBitmap) {
        HDC dc = ::GetDC(m_hwnd);
        Gdiplus::Graphics gc(m_hwnd);
        Gdiplus::CachedBitmap cachedBmp(m_cacheBitmap, &gc);
        gc.DrawCachedBitmap(&cachedBmp, 0, 0);
        ::ReleaseDC(m_hwnd, dc);
    }
}

LGraphicsContext* LGraphicsContext::create() 
{
	return new GraphicsContextWin();
}
}