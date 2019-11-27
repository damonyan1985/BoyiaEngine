#include "ImageWin.h"
#include "ImageView.h"
#include "LColor.h"
#include "LGdi.h"
#include "SalLog.h"
#include "UIView.h"
#include <Objidl.h>

namespace util {

ImageWin::ImageWin()
    : m_image(kBoyiaNull)
    , m_pixels(kBoyiaNull)
    , m_winImage(kBoyiaNull)
    , m_dataLen(0)
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
        unlockPixels();
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

LVoid ImageWin::setData(LVoid* data, LInt dataLen)
{
    m_pixels = data;
    m_dataLen = dataLen;
}

LVoid ImageWin::unlockPixels()
{
    HGLOBAL hmem = GlobalAlloc(GMEM_FIXED, m_dataLen);
    BYTE* pmem = (BYTE*)GlobalLock(hmem);
    if (!pmem) {
        return;
    }
    memcpy(pmem, m_pixels, m_dataLen);
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

LVoid* ImageWin::pixels() const
{
    return m_pixels;
}

LVoid ImageWin::onClientCallback()
{
    setLoaded(LTrue);
}
}
