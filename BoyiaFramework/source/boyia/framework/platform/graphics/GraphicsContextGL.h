#ifndef GraphicsContextGL_h
#define GraphicsContextGL_h

#include "BoyiaPtr.h"
#include "GLContext.h"
#include "GLPainter.h"
#include "KList.h"
#include "LGdi.h"

namespace util {

typedef KList<BoyiaPtr<yanbo::GLPainter>> ListPainter;
// ItemPainter顾名思义，一个HtmlView对应的所有GLPainter
struct ItemPainter {
    ListPainter painters;
    LVoid* item;
};

class GraphicsContextGL : public LGraphicsContext {
public:
    GraphicsContextGL();
    virtual ~GraphicsContextGL();

public:
    //LVoid initGLEnvironment(LInt width, LInt height);

    virtual LVoid drawLine(const LPoint& p1, const LPoint& p2);
    virtual LVoid drawLine(LInt x0, LInt y0, LInt x1, LInt y1);
    virtual LVoid drawRect(const LRect& aRect);
    virtual LVoid drawRect(LInt x, LInt y, LInt w, LInt h);

    virtual LVoid drawEllipse(const LRect& aRect);

    virtual LVoid drawRoundRect(const LRect& aRect, const LSize& aCornerSize);

    virtual LVoid drawText(const String& aText, const LRect& aRect);
    virtual LVoid drawText(const String& aText, const LPoint& aPoint);

    virtual LVoid drawImage(const LPoint& aTopLeft, const LImage* aBitmap);
    virtual LVoid drawImage(const LImage* image);
    virtual LVoid drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect);

    virtual LVoid drawVideo(const LRect& rect, const LMediaPlayer* mp);
    virtual LVoid drawText(const String& text, const LRect& rect, TextAlign align);
    virtual LVoid setBrushStyle(BrushStyle aBrushStyle);
    virtual LVoid setPenStyle(PenStyle aPenStyle);

    virtual LVoid setBrushColor(const LRgb& aColor);
    virtual LVoid setPenColor(const LRgb& aColor);
    virtual LVoid setFont(const LFont& font);

    virtual LVoid reset();
    virtual LVoid submit();
    virtual LVoid setHtmlView(LVoid* item);
    virtual LVoid save();
    virtual LVoid clipRect(const LRect& rect);
    virtual LVoid restore();

    LVoid setContextWin(LVoid* win);

private:
    ItemPainter* currentPainter();
    LVoid fillBuffer(LVoid* ptr);

    LRgb m_brushColor;
    LRgb m_penColor;
    LFont m_font;
    LVoid* m_item;
    LRect* m_clipRect;
    GLContext m_context;
};
}
#endif