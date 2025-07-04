#ifndef ImageWin_h
#define ImageWin_h

#include "HtmlView.h"
#include "LGdi.h"
#include "UIThreadClientMap.h"
#include <windows.h>
#include <GdiPlus.h>

using namespace Gdiplus;

namespace util {
class ImageWin : public LImage, public yanbo::UIThreadClient {
public:
    ImageWin();
    virtual ~ImageWin();

    static Gdiplus::Image* createWinImage(const OwnerPtr<String>& data);

public:
    virtual LVoid load(const String& aPath, LVoid* image);
    virtual const String& url() const;
    virtual LVoid setLoaded(LBool loaded);
    LVoid drawText(
        const String& text,
        const LRect& rect,
        LGraphicsContext::TextAlign align,
        const LFont& font,
        const LColor& penColor,
        const LColor& brushColor);

    LVoid* item() const;
    LVoid setItem(yanbo::HtmlView* item);
    LVoid* pixels() const;
    LVoid setData(const OwnerPtr<String>& data);
    LVoid setImage(Gdiplus::Image* image);

    virtual LVoid onClientCallback();

private:
    yanbo::HtmlView* m_image;
    OwnerPtr<String> m_data;
    Gdiplus::Image* m_winImage;
};
}

#endif