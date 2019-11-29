#include "ImageWin.h"
#include "ImageView.h"
#include "LColor.h"
#include "LGdi.h"
#include "SalLog.h"
#include "UIView.h"
#include "ImageLoader.h"
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

LVoid ImageWin::setData(const OwnerPtr<String>& data)
{
    m_data = data;
}

LVoid ImageWin::unlockPixels()
{
    HGLOBAL hmem = GlobalAlloc(GMEM_FIXED, m_data->GetLength());
    BYTE* pmem = (BYTE*)GlobalLock(hmem);
    if (!pmem) {
        return;
    }
    memcpy(pmem, m_data->GetBuffer(), m_data->GetLength());
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
    return m_data->GetBuffer();
}

LVoid ImageWin::onClientCallback()
{
    setLoaded(LTrue);
}

/*
LVoid ImageWin::onDataReceived(const LByte* data, LInt size)
{
    LByte* destData = new LByte[size];
    util::LMemcpy(destData, data, size);
    m_builder.append(destData, 0, size, LFalse);
}

LVoid ImageWin::onStatusCode(LInt statusCode)
{

}

LVoid ImageWin::onFileLen(LInt len)
{

}

LVoid ImageWin::onRedirectUrl(const String& redirectUrl)
{

}

LVoid ImageWin::onLoadError(LInt error)
{

}

LVoid ImageWin::onLoadFinished()
{
    //m_data = m_builder.toString();
    //UIThread::instance()->sendUIEvent(this);
}
*/
}
