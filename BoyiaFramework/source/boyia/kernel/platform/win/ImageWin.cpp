#include "ImageWin.h"
#include "AppManager.h"
#include "ImageLoader.h"
#include "ImageView.h"
#include "LColor.h"
#include "LGdi.h"
#include "SalLog.h"
#include <Objidl.h>

namespace util {

ImageWin::ImageWin()
    : m_image(kBoyiaNull)
    , m_winImage(kBoyiaNull)
    , m_data(kBoyiaNull)
{
}

ImageWin::~ImageWin()
{
}

LVoid ImageWin::load(const String& path, LVoid* image)
{
    yanbo::UIThreadClientMap::instance()->registerClient(this);
    //yanbo::AppManager::instance()->network()->loadUrl(path, this);
    yanbo::ImageLoader::instance()->loadImage(path, getClientId());
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

// setLoaded在UIthread中执行
LVoid ImageWin::setLoaded(LBool loaded)
{
    LImage::setLoaded(loaded);
    if (m_image && loaded) {
        //unlockPixels();
        //yanbo::UIView::getInstance()->getLoader()->repaint(m_image);
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

LVoid ImageWin::setData(const OwnerPtr<String>& data)
{
    HGLOBAL hmem = GlobalAlloc(GMEM_FIXED, data->GetLength());
    BYTE* pmem = (BYTE*)GlobalLock(hmem);
    if (!pmem) {
        return;
    }
    memcpy(pmem, data->GetBuffer(), data->GetLength());
    IStream* pstm;
    HRESULT ht = ::CreateStreamOnHGlobal(hmem, FALSE, &pstm);
    if (ht != S_OK) {
        GlobalFree(hmem);
        return;
    }
    m_winImage = Gdiplus::Image::FromStream(pstm);
    ::GlobalUnlock(hmem);
}

const String& ImageWin::url() const
{
    return static_cast<yanbo::ImageView*>(m_image)->url();
}

Gdiplus::Image* ImageWin::image() const
{
    return m_winImage;
}

LVoid* ImageWin::pixels() const
{
    return kBoyiaNull;
}

LVoid ImageWin::onClientCallback()
{
    setLoaded(LTrue);
}

}
