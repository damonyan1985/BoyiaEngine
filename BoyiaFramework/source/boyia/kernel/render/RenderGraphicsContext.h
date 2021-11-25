#ifndef RenderGraphicsContext_h
#define RenderGraphicsContext_h

#include "LGdi.h"
#include "RenderThread.h"
#include "RenderLayer.h"
#include "VsyncWaiter.h"

namespace yanbo {
class ItemPainter;
class RenderGraphicsContext : public LGraphicsContext {
public:
    RenderGraphicsContext();
    virtual ~RenderGraphicsContext();
    virtual LVoid drawLine(const LPoint& p1, const LPoint& p2);
    virtual LVoid drawLine(LInt x0, LInt y0, LInt x1, LInt y1);
    virtual LVoid drawRect(const LRect& aRect);
    virtual LVoid drawRect(LInt x, LInt y, LInt w, LInt h);

    virtual LVoid drawEllipse(const LRect& aRect);

    virtual LVoid drawRoundRect(const LRect& aRect, const LSize& aCornerSize);
    
    // 虚函数的默认形参根据指针类型来决定
    virtual LVoid drawRoundRect(const LRect& aRect, LInt topLeftRadius, LInt topRightRadius, LInt bottomRightRadius, LInt bottomLeftRadius);

    virtual LVoid drawText(const String& aText, const LRect& aRect);
    virtual LVoid drawText(const String& aText, const LPoint& aPoint);

    virtual LVoid drawImage(const LPoint& aTopLeft, const LImage* aBitmap);
    virtual LVoid drawImage(const LImage* image);
    virtual LVoid drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect);

    virtual LVoid drawVideo(const LRect& rect, const LMediaPlayer* mp);
#if ENABLE(BOYIA_PLATFORM_VIEW)
    virtual LVoid drawPlatform(const LRect& rect, LVoid* platformView);
#endif

    virtual LVoid drawText(const String& text, const LRect& rect, TextAlign align);
    virtual LVoid setBrushStyle(BrushStyle aBrushStyle);
    virtual LVoid setPenStyle(PenStyle aPenStyle);

    virtual LVoid setBrushColor(const LColor& aColor);
    virtual LVoid setPenColor(const LColor& aColor);
    virtual LVoid setFont(const LFont& font);

    virtual LVoid reset();
    virtual LVoid submit();
    virtual LVoid setHtmlView(ViewPainter* item);
    virtual LVoid save();
    virtual LVoid clipRect(const LRect& rect);
    virtual LVoid restore();

private:
    LVoid submit(LVoid* view, RenderLayer* parentLayer);
    VsyncWaiter* vsyncWaiter() const;
    
    ItemPainter* currentPainter();

    LRect* m_clipRect;
    LFont m_font;
    LVoid* m_item;
    LColor m_brushColor;
    LColor m_penColor;
    KVector<LUintPtr>* m_collectBuffers;
    mutable VsyncWaiter* m_vsync;
};
}

#endif
