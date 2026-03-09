#include "ImageWin.h"
#include "AppManager.h"
#include "ImageLoader.h"
#include "ImageView.h"
#include "LGdi.h"
#include "ImageGDIPlus.h"
#include "ImageD2D.h"
#include "IRenderEngine.h"
#include "RenderEngineWin.h"
#include "RenderEngineDirect2D.h"

namespace util {
LBool ImageWin::s_isHardwareAccelerated = LFalse;

ImageWin::ImageWin()
    : m_image(kBoyiaNull)
    , m_data(kBoyiaNull)
{
}

ImageWin::~ImageWin()
{
}

LVoid ImageWin::load(const String& path, LVoid* image)
{
    (void)image;
    yanbo::UIThreadClientMap::instance()->registerClient(this);
    yanbo::ImageLoader::instance()->loadImage(path, getClientId());
}

LVoid ImageWin::drawText(const String& text,
    const LRect& rect,
    LGraphicsContext::TextAlign align,
    const LFont& font,
    const LColor& penColor,
    const LColor& brushColor)
{
}

LVoid ImageWin::setLoaded(LBool loaded)
{
    LImage::setLoaded(loaded);
    if (m_image && loaded) {
        yanbo::AppManager::instance()->uiThread()->drawUI(m_image);
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

const String& ImageWin::url() const
{
    return static_cast<yanbo::ImageView*>(m_image)->url();
}

LVoid ImageWin::onClientCallback()
{
    setLoaded(LTrue);
}

LVoid ImageWin::setIsHardwareAccelerated(LBool isHardwareAccelerated) 
{
    s_isHardwareAccelerated = isHardwareAccelerated;
}

LBool ImageWin::isHardwareAccelerated() 
{
    return s_isHardwareAccelerated;
}

LImage* LImage::create(LVoid* item)
{
    ImageWin* image = kBoyiaNull;
    if (ImageWin::isHardwareAccelerated()) {
        image = new ImageD2D();
    } else {
        image = new ImageGDIPlus();
    }

    image->setItem(static_cast<yanbo::HtmlView*>(item));
    
    return image;
}

} // namespace util

namespace yanbo {
IRenderEngine* IRenderEngine::create()
{
    if (util::ImageWin::isHardwareAccelerated()) {
        return new RenderEngineDirect2D();
    } else {
        return new RenderEngineWin();
    }
}

}
