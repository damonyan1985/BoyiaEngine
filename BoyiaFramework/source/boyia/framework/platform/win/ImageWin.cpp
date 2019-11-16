#include "ImageWin.h"
#include "ImageView.h"
#include "LColor.h"
#include "LGdi.h"
#include "SalLog.h"
#include "UIView.h"

namespace util {

ImageWin::ImageWin()
    : m_image(NULL)
    , m_pixels(NULL)
{
}

ImageWin::~ImageWin()
{
}

LVoid ImageWin::load(const String& path, LVoid* image)
{
}

LImage* LImage::create(LVoid* item)
{
    ImageWin* image = new ImageWin();
    image->setItem(static_cast<yanbo::HtmlView*>(item));
    return image;
}

void ImageWin::drawText(const String& text,
    const LRect& rect,
    LGraphicsContext::TextAlign align,
    const LFont& font,
    const LRgb& penColor,
    const LRgb& brushColor)
{
}

LVoid ImageWin::setLoaded(LBool loaded)
{
    LImage::setLoaded(loaded);
    if (m_image && loaded) {
        //unlockPixels();
        yanbo::UIView::getInstance()->getLoader()->repaint(m_image);
    }
}

LVoid* ImageWin::item() const
{
    return m_image;
}

LVoid ImageWin::setItem(yanbo::HtmlView* item)
{
    m_image = item;
}

LVoid ImageWin::unlockPixels()
{
}

const String& ImageWin::url() const
{
    return static_cast<yanbo::ImageView*>(m_image)->url();
}

LVoid* ImageWin::pixels() const
{
    return m_pixels;
}

LVoid ImageWin::onClientCallback()
{
    setLoaded(LTrue);
}
}
