#include "ImageD2D.h"
#include "ImageLoader.h"
#include <Objidl.h>

namespace util {

static IWICImagingFactory* getWicFactory()
{
    static IWICImagingFactory* s_factory = NULL;
    if (!s_factory) {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&s_factory));
        if (FAILED(hr) || !s_factory) {
            return NULL;
        }
    }
    return s_factory;
}

IWICBitmap* ImageD2D::createD2DImage(const OwnerPtr<String>& data)
{
    if (!data || data->GetLength() == 0) {
        return NULL;
    }
    IWICImagingFactory* factory = getWicFactory();
    if (!factory) {
        return NULL;
    }

    HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, data->GetLength());
    if (!hmem) {
        return NULL;
    }
    void* pmem = GlobalLock(hmem);
    if (!pmem) {
        GlobalFree(hmem);
        return NULL;
    }
    memcpy(pmem, data->GetBuffer(), data->GetLength());
    GlobalUnlock(hmem);

    IStream* pstm = NULL;
    HRESULT hr = CreateStreamOnHGlobal(hmem, TRUE, &pstm);
    if (FAILED(hr) || !pstm) {
        GlobalFree(hmem);
        return NULL;
    }

    IWICBitmapDecoder* decoder = NULL;
    hr = factory->CreateDecoderFromStream(
        pstm,
        NULL,
        WICDecodeMetadataCacheOnLoad,
        &decoder);
    pstm->Release();
    if (FAILED(hr) || !decoder) {
        return NULL;
    }

    IWICBitmapFrameDecode* frame = NULL;
    hr = decoder->GetFrame(0, &frame);
    decoder->Release();
    if (FAILED(hr) || !frame) {
        return NULL;
    }

    IWICBitmap* bitmap = NULL;
    hr = factory->CreateBitmapFromSource(frame, WICBitmapCacheOnDemand, &bitmap);
    frame->Release();
    if (FAILED(hr) || !bitmap) {
        return NULL;
    }
    return bitmap;
}

ImageD2D::ImageD2D()
    : ImageWin()
    , m_wicBitmap(NULL)
{
}

ImageD2D::~ImageD2D()
{
    if (m_wicBitmap) {
        m_wicBitmap->Release();
        m_wicBitmap = NULL;
    }
}

LVoid ImageD2D::setData(const OwnerPtr<String>& data)
{
    if (m_wicBitmap) {
        m_wicBitmap->Release();
        m_wicBitmap = NULL;
    }
    m_wicBitmap = createD2DImage(data);
    if (m_wicBitmap) {
        UINT w = 0, h = 0;
        m_wicBitmap->GetSize(&w, &h);
        m_width = (LInt)w;
        m_height = (LInt)h;
    }
}

LVoid ImageD2D::setImage(LVoid* image)
{
    if (m_wicBitmap && m_wicBitmap != image) {
        m_wicBitmap->Release();
    }
    m_wicBitmap = static_cast<IWICBitmap*>(image);
    if (m_wicBitmap) {
        UINT w = 0, h = 0;
        m_wicBitmap->GetSize(&w, &h);
        m_width = (LInt)w;
        m_height = (LInt)h;
    }
}

LVoid* ImageD2D::pixels() const
{
    return m_wicBitmap;
}

} // namespace util
