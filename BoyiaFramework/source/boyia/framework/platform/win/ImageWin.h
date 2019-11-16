#ifndef ImageWin_h
#define ImageWin_h

#include "HtmlView.h"
#include "LGdi.h"
#include "UIThreadClientMap.h"

namespace util {
class ImageWin : public LImage, public yanbo::UIThreadClient {
public:
    ImageWin();
    virtual ~ImageWin();

public:
    virtual LVoid load(const String& aPath, LVoid* image);
    virtual const String& url() const;
    virtual LVoid setLoaded(LBool loaded);
    void drawText(
        const String& text,
        const LRect& rect,
        LGraphicsContext::TextAlign align,
        const LFont& font,
        const LRgb& penColor,
        const LRgb& brushColor);

    LVoid* item() const;
    LVoid setItem(yanbo::HtmlView* item);
    LVoid unlockPixels();
    LVoid* pixels() const;

    virtual LVoid onClientCallback();

private:
    yanbo::HtmlView* m_image;
    LVoid* m_pixels;
};
}

#endif