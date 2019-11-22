#ifndef GraphicsContextWin_h
#define GraphicsContextWin_h

#include "LGdi.h"
#include "KVector.h"
#include <windows.h>

namespace util {
class PaintCommand;
class GraphicsContextWin : public LGraphicsContext {
public:
	GraphicsContextWin();
	~GraphicsContextWin();

	LVoid setContextWin(HWND hwnd);

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

private:
	KVector<PaintCommand> m_cmds;
	HWND m_hwnd;
};
}

#endif
