#include "ImageWin.h"
#include "AppManager.h"
#include "ImageLoader.h"
#include "ImageView.h"
#include "LColorUtil.h"
#include "LGdi.h"
#include "SalLog.h"
#include <Objidl.h>

namespace util {

Gdiplus::Image* ImageWin::createWinImage(const OwnerPtr<String>& data)
{
    HGLOBAL hmem = GlobalAlloc(GMEM_FIXED, data->GetLength());
    BYTE* pmem = (BYTE*)GlobalLock(hmem);
    if (!pmem) {
        return kBoyiaNull;
    }
    memcpy(pmem, data->GetBuffer(), data->GetLength());
    IStream* pstm;
    HRESULT ht = ::CreateStreamOnHGlobal(hmem, FALSE, &pstm);
    if (ht != S_OK) {
        GlobalFree(hmem);
        return kBoyiaNull;
    }

    Gdiplus::Image* image = Gdiplus::Image::FromStream(pstm);
    ::GlobalUnlock(hmem);

    return image;
}

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
    const LColor& penColor,
    const LColor& brushColor)
{
}

// setLoaded在UIthread中执行
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

LVoid ImageWin::setData(const OwnerPtr<String>& data)
{
    m_winImage = createWinImage(data);
}

LVoid ImageWin::setImage(Gdiplus::Image* image)
{
    m_winImage = image;
}

const String& ImageWin::url() const
{
    return static_cast<yanbo::ImageView*>(m_image)->url();
}

LVoid* ImageWin::pixels() const
{
    return m_winImage;
}

LVoid ImageWin::onClientCallback()
{
    setLoaded(LTrue);
}

}
