#ifndef GraphicsContextGL_h
#define GraphicsContextGL_h

#include "BoyiaPtr.h"
#include "GLContext.h"
#include "GLPainter.h"
#include "KList.h"
#include "LGdi.h"

namespace util {
class ItemPainter;
class GraphicsContextGL : public LGraphicsContext {
public:
    GraphicsContextGL();
    virtual ~GraphicsContextGL();

public:
    virtual LVoid drawLine(const LPoint& p1, const LPoint& p2);
    virtual LVoid drawLine(LInt x0, LInt y0, LInt x1, LInt y1);
    virtual LVoid drawRect(const LRect& aRect);
    virtual LVoid drawRect(LInt x, LInt y, LInt w, LInt h);

    virtual LVoid drawEllipse(const LRect& aRect);

    virtual LVoid drawRoundRect(const LRect& aRect, const LSize& aCornerSize);
    virtual LVoid drawRoundRect(const LRect& aRect, LInt topLeftRadius, LInt topRightRadius, LInt bottomRightRadius, LInt bottomLeftRadius);

    virtual LVoid drawText(const String& aText, const LRect& aRect);
    virtual LVoid drawText(const String& aText, const LPoint& aPoint);

    virtual LVoid drawImage(const LPoint& aTopLeft, const LImage* aBitmap);
    virtual LVoid drawImage(const LImage* image);
    virtual LVoid drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect);
    virtual LVoid drawRoundImage(const LImage* image, LInt topLeftRadius, LInt topRightRadius, LInt bottomRightRadius, LInt bottomLeftRadius);
    virtual LVoid drawVideo(const LRect& rect, const LMediaPlayer* mp);
    virtual LVoid drawCamera(const LRect& rect, const LCamera* camera);
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

    LVoid setContextWin(LVoid* win);

private:
    ItemPainter* currentPainter();
    LVoid fillBuffer(LVoid* ptr);

    LColor m_brushColor;
    LColor m_penColor;
    LFont m_font;
    LVoid* m_item;
    LRect* m_clipRect;
    GLContext m_context;
    LBool m_hasGLInit;
};
}
#endif
