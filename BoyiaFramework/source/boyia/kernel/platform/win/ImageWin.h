#ifndef ImageWin_h
#define ImageWin_h

#include "HtmlView.h"
#include "LGdi.h"
#include "UIThreadClientMap.h"
#include <windows.h>

namespace util {

class ImageWin : public LImage, public yanbo::UIThreadClient {
public:
    ImageWin();
    virtual ~ImageWin();

public:
    virtual LVoid load(const String& aPath, LVoid* image);
    virtual const String& url() const;
    virtual LVoid setLoaded(LBool loaded);
    virtual LVoid drawText(
        const String& text,
        const LRect& rect,
        LGraphicsContext::TextAlign align,
        const LFont& font,
        const LColor& penColor,
        const LColor& brushColor);

    static LVoid setIsHardwareAccelerated(LBool isHardwareAccelerated);
    static LBool isHardwareAccelerated();    

    LVoid* item() const;
    LVoid setItem(yanbo::HtmlView* item);
    virtual LVoid* pixels() const = 0;
    virtual LVoid setData(const OwnerPtr<String>& data) = 0;
    virtual LVoid setImage(LVoid* image) = 0;

    virtual LVoid onClientCallback();

protected:
    yanbo::HtmlView* m_image;
    OwnerPtr<String> m_data;

    static LBool s_isHardwareAccelerated;
};

} // namespace util

#endif
