#include "ImageGDIPlus.h"
#include "ImageLoader.h"
#include "LGdi.h"
#include <Objidl.h>

namespace util {

Gdiplus::Image* ImageGDIPlus::createWinImage(const OwnerPtr<String>& data)
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

ImageGDIPlus::ImageGDIPlus()
    : ImageWin()
    , m_winImage(kBoyiaNull)
{
}

ImageGDIPlus::~ImageGDIPlus()
{
}

LVoid ImageGDIPlus::setData(const OwnerPtr<String>& data)
{
    m_winImage = createWinImage(data);
}

LVoid ImageGDIPlus::setImage(LVoid* image)
{
    m_winImage = static_cast<Gdiplus::Image*>(image);
}

LVoid* ImageGDIPlus::pixels() const
{
    return m_winImage;
}

} // namespace util
