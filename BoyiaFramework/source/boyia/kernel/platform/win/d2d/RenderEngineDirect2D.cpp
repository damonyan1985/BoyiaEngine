#include "RenderEngineDirect2D.h"
#include "../CharConvertor.h"
#include "../../PixelRatio.h"

namespace yanbo {

static inline float RealLengthF(LInt length)
{
    return length * yanbo::PixelRatio::ratio();
}

RenderEngineDirect2D::RenderEngineDirect2D()
    : m_hwnd(NULL)
    , m_factory(NULL)
    , m_hwndTarget(NULL)
    , m_bitmapTarget(NULL)
    , m_cacheBitmap(NULL)
    , m_dwriteFactory(NULL)
{
    D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
#if defined(_DEBUG)
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &options, (void**)&m_factory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_dwriteFactory);
    init();
}

RenderEngineDirect2D::~RenderEngineDirect2D()
{
    if (m_cacheBitmap) {
        m_cacheBitmap->Release();
        m_cacheBitmap = NULL;
    }
    if (m_bitmapTarget) {
        m_bitmapTarget->Release();
        m_bitmapTarget = NULL;
    }
    if (m_hwndTarget) {
        m_hwndTarget->Release();
        m_hwndTarget = NULL;
    }
    if (m_dwriteFactory) {
        m_dwriteFactory->Release();
        m_dwriteFactory = NULL;
    }
    if (m_factory) {
        m_factory->Release();
        m_factory = NULL;
    }
}

LVoid RenderEngineDirect2D::init()
{
    m_functions[RenderCommand::kRenderRect] = (RenderFunction)&RenderEngineDirect2D::renderRect;
    m_functions[RenderCommand::kRenderText] = (RenderFunction)&RenderEngineDirect2D::renderText;
    m_functions[RenderCommand::kRenderImage] = (RenderFunction)&RenderEngineDirect2D::renderImage;
    m_functions[RenderCommand::kRenderRoundRect] = (RenderFunction)&RenderEngineDirect2D::renderRoundRect;
    m_functions[RenderCommand::kRenderVideo] = (RenderFunction)&RenderEngineDirect2D::renderVideo;
    m_functions[RenderCommand::kRenderRoundImage] = (RenderFunction)&RenderEngineDirect2D::renderRoundImage;
    m_functions[RenderCommand::kRenderPlatform] = (RenderFunction)&RenderEngineDirect2D::renderVideo;
}

LVoid RenderEngineDirect2D::setContextWin(HWND hwnd)
{
    m_hwnd = hwnd;
    if (m_hwndTarget) {
        m_hwndTarget->Release();
        m_hwndTarget = NULL;
    }
    if (m_bitmapTarget) {
        m_bitmapTarget->Release();
        m_bitmapTarget = NULL;
    }
    if (m_cacheBitmap) {
        m_cacheBitmap->Release();
        m_cacheBitmap = NULL;
    }
}

LVoid RenderEngineDirect2D::reset()
{
    if (!m_hwndTarget || !m_cacheBitmap) {
        return;
    }
    m_hwndTarget->BeginDraw();
    m_hwndTarget->DrawBitmap(m_cacheBitmap, D2D1::RectF(0, 0, m_cacheBitmap->GetPixelSize().width, m_cacheBitmap->GetPixelSize().height),
        1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    m_hwndTarget->EndDraw();
}

LVoid RenderEngineDirect2D::render(RenderLayer* layer)
{
    if (!m_hwnd || !m_factory) {
        return;
    }

    RECT rc;
    ::GetClientRect(m_hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    if (width == 0 || height == 0) {
        return;
    }

    if (!m_hwndTarget) {
        D2D1_SIZE_U size = D2D1::SizeU(width, height);
        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);
        HRESULT hr = m_factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_hwndTarget);
        if (FAILED(hr) || !m_hwndTarget) {
            return;
        }
    }

    m_hwndTarget->Resize(D2D1::SizeU(width, height));

    if (!m_bitmapTarget || m_cacheBitmap == NULL) {
        if (m_bitmapTarget) {
            m_bitmapTarget->Release();
            m_bitmapTarget = NULL;
        }
        if (m_cacheBitmap) {
            m_cacheBitmap->Release();
            m_cacheBitmap = NULL;
        }
        D2D1_SIZE_F sizeF = D2D1::SizeF((float)width, (float)height);
        HRESULT hr = m_hwndTarget->CreateCompatibleRenderTarget(sizeF, &m_bitmapTarget);
        if (FAILED(hr) || !m_bitmapTarget) {
            return;
        }
    }

    D2D1_SIZE_F bitmapSize = m_bitmapTarget->GetSize();
    if (bitmapSize.width < (float)width || bitmapSize.height < (float)height) {
        if (m_cacheBitmap) {
            m_cacheBitmap->Release();
            m_cacheBitmap = NULL;
        }
        if (m_bitmapTarget) {
            m_bitmapTarget->Release();
            m_bitmapTarget = NULL;
        }
        D2D1_SIZE_F sizeF = D2D1::SizeF((float)width, (float)height);
        m_hwndTarget->CreateCompatibleRenderTarget(sizeF, &m_bitmapTarget);
    }

    m_bitmapTarget->BeginDraw();
    m_bitmapTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));
    renderImpl(layer, m_bitmapTarget);
    HRESULT hr = m_bitmapTarget->EndDraw();
    if (FAILED(hr)) {
        return;
    }

    if (m_cacheBitmap) {
        m_cacheBitmap->Release();
        m_cacheBitmap = NULL;
    }
    m_bitmapTarget->GetBitmap(&m_cacheBitmap);
    if (!m_cacheBitmap) {
        return;
    }

    m_hwndTarget->BeginDraw();
    m_hwndTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));
    m_hwndTarget->DrawBitmap(m_cacheBitmap, D2D1::RectF(0, 0, (float)width, (float)height),
        1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    m_hwndTarget->EndDraw();
}

LVoid RenderEngineDirect2D::renderImpl(RenderLayer* layer, ID2D1RenderTarget* rt)
{
    if (!layer || !rt) {
        return;
    }
    if (layer->m_buffer) {
        LInt commandSize = layer->m_buffer->size();
        for (LInt i = 0; i < commandSize; i++) {
            RenderCommand* cmd = layer->m_buffer->elementAt(i).get();
            LInt type = cmd->type();
            if (type >= 0 && type < 7 && m_functions[type]) {
                (this->*(m_functions[type]))(cmd, rt);
            }
        }
    }
    for (LInt i = 0; i < layer->m_children.size(); i++) {
        renderImpl(layer->m_children[i], rt);
    }
}

LVoid RenderEngineDirect2D::renderRect(RenderCommand* cmd, ID2D1RenderTarget* rt)
{
    RenderRectCommand* resource = static_cast<RenderRectCommand*>(cmd);
    float x = RealLengthF(resource->rect.iTopLeft.iX);
    float y = RealLengthF(resource->rect.iTopLeft.iY);
    float w = RealLengthF(resource->rect.GetWidth());
    float h = RealLengthF(resource->rect.GetHeight());
    if (w < 1.0f) w = 1.0f;
    if (h < 1.0f) h = 1.0f;

    ID2D1SolidColorBrush* brush = NULL;
    D2D1_COLOR_F color = D2D1::ColorF(
        resource->color.m_red / 255.0f,
        resource->color.m_green / 255.0f,
        resource->color.m_blue / 255.0f,
        resource->color.m_alpha / 255.0f);
    rt->CreateSolidColorBrush(color, &brush);
    if (brush) {
        rt->FillRectangle(D2D1::RectF(x, y, x + w, y + h), brush);
        brush->Release();
    }
}

LVoid RenderEngineDirect2D::renderText(RenderCommand* cmd, ID2D1RenderTarget* rt)
{
    RenderTextCommand* resource = static_cast<RenderTextCommand*>(cmd);
    WString wtext;
    CharConvertor::CharToWchar(resource->text, wtext);
    if (wtext.GetLength() == 0) {
        return;
    }

    float fontSize = resource->font.getFontSize() * PixelRatio::ratio();
    if (fontSize < 1.0f) fontSize = 12.0f;

    WString wFontFamily;
    CharConvertor::CharToWchar(resource->font.getFamily(), wFontFamily);
    const wchar_t* family = wFontFamily.GetLength() > 0 ? wFontFamily.GetBuffer() : L"Arial";

    IDWriteTextFormat* textFormat = NULL;
    m_dwriteFactory->CreateTextFormat(
        family,
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"",
        &textFormat);
    if (!textFormat) {
        return;
    }

    float x = RealLengthF(resource->rect.iTopLeft.iX);
    float y = RealLengthF(resource->rect.iTopLeft.iY);
    float width = RealLengthF(resource->rect.GetWidth());
    float height = RealLengthF(resource->rect.GetHeight());

    D2D1_RECT_F layoutRect = D2D1::RectF(x, y, x + width, y + height);
    ID2D1SolidColorBrush* brush = NULL;
    D2D1_COLOR_F color = D2D1::ColorF(
        resource->color.m_red / 255.0f,
        resource->color.m_green / 255.0f,
        resource->color.m_blue / 255.0f,
        resource->color.m_alpha / 255.0f);
    rt->CreateSolidColorBrush(color, &brush);
    if (brush) {
        rt->DrawText(wtext.GetBuffer(), (UINT32)wtext.GetLength(), textFormat, layoutRect, brush, D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
        brush->Release();
    }
    textFormat->Release();
}

LVoid RenderEngineDirect2D::renderImage(RenderCommand* cmd, ID2D1RenderTarget* rt)
{
    RenderImageCommand* resource = static_cast<RenderImageCommand*>(cmd);
    if (!resource->image) {
        return;
    }
    ID2D1Bitmap* d2dBitmap = createBitmapFromWicBitmap(rt, (IWICBitmapSource*)resource->image);
    if (!d2dBitmap) {
        return;
    }
    float x = RealLengthF(resource->rect.iTopLeft.iX);
    float y = RealLengthF(resource->rect.iTopLeft.iY);
    float w = RealLengthF(resource->rect.GetWidth());
    float h = RealLengthF(resource->rect.GetHeight());
    rt->DrawBitmap(d2dBitmap, D2D1::RectF(x, y, x + w, y + h), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    d2dBitmap->Release();
}

ID2D1PathGeometry* RenderEngineDirect2D::createRoundedRectGeometry(RenderRoundRectCommand* cmd)
{
    if (!m_factory || !cmd) {
        return NULL;
    }
    float x = RealLengthF(cmd->rect.iTopLeft.iX);
    float y = RealLengthF(cmd->rect.iTopLeft.iY);
    float w = RealLengthF(cmd->rect.GetWidth());
    float h = RealLengthF(cmd->rect.GetHeight());
    float tl = RealLengthF(cmd->topLeftRadius) * 2.0f;
    float tr = RealLengthF(cmd->topRightRadius) * 2.0f;
    float br = RealLengthF(cmd->bottomRightRadius) * 2.0f;
    float bl = RealLengthF(cmd->bottomLeftRadius) * 2.0f;

    ID2D1PathGeometry* path = NULL;
    ID2D1GeometrySink* sink = NULL;
    HRESULT hr = m_factory->CreatePathGeometry(&path);
    if (FAILED(hr) || !path) {
        return NULL;
    }
    hr = path->Open(&sink);
    if (FAILED(hr) || !sink) {
        path->Release();
        return NULL;
    }

    sink->SetFillMode(D2D1_FILL_MODE_WINDING);
    sink->BeginFigure(D2D1::Point2F(x + tl / 2, y), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(x, y + tl / 2), D2D1::SizeF(tl / 2, tl / 2), 0.0f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    sink->AddLine(D2D1::Point2F(x, y + h - bl / 2));
    sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(x + bl / 2, y + h), D2D1::SizeF(bl / 2, bl / 2), 0.0f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    sink->AddLine(D2D1::Point2F(x + w - br / 2, y + h));
    sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(x + w, y + h - br / 2), D2D1::SizeF(br / 2, br / 2), 0.0f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    sink->AddLine(D2D1::Point2F(x + w, y + tr / 2));
    sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(x + w - tr / 2, y), D2D1::SizeF(tr / 2, tr / 2), 0.0f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    sink->AddLine(D2D1::Point2F(x + tl / 2, y));
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();
    sink->Release();

    return path;
}

LVoid RenderEngineDirect2D::renderRoundRect(RenderCommand* cmd, ID2D1RenderTarget* rt)
{
    RenderRoundRectCommand* roundCmd = static_cast<RenderRoundRectCommand*>(cmd);
    ID2D1PathGeometry* path = createRoundedRectGeometry(roundCmd);
    if (!path) {
        return;
    }
    ID2D1SolidColorBrush* brush = NULL;
    D2D1_COLOR_F color = D2D1::ColorF(
        roundCmd->color.m_red / 255.0f,
        roundCmd->color.m_green / 255.0f,
        roundCmd->color.m_blue / 255.0f,
        roundCmd->color.m_alpha / 255.0f);
    rt->CreateSolidColorBrush(color, &brush);
    if (brush) {
        rt->FillGeometry(path, brush);
        brush->Release();
    }
    path->Release();
}

LVoid RenderEngineDirect2D::renderVideo(RenderCommand* cmd, ID2D1RenderTarget* rt)
{
    (void)cmd;
    (void)rt;
}

LVoid RenderEngineDirect2D::renderRoundImage(RenderCommand* cmd, ID2D1RenderTarget* rt)
{
    RenderRoundImageCommand* resource = static_cast<RenderRoundImageCommand*>(cmd);
    if (!resource->image) {
        return;
    }
    ID2D1PathGeometry* path = createRoundedRectGeometry(resource);
    if (!path) {
        return;
    }
    D2D1_LAYER_PARAMETERS layerParams = D2D1::LayerParameters(D2D1::InfiniteRect(), path);
    rt->PushLayer(layerParams, NULL);
    path->Release();

    ID2D1Bitmap* d2dBitmap = createBitmapFromWicBitmap(rt, (IWICBitmapSource*)resource->image);
    if (d2dBitmap) {
        float x = RealLengthF(resource->rect.iTopLeft.iX);
        float y = RealLengthF(resource->rect.iTopLeft.iY);
        float w = RealLengthF(resource->rect.GetWidth());
        float h = RealLengthF(resource->rect.GetHeight());
        rt->DrawBitmap(d2dBitmap, D2D1::RectF(x, y, x + w, y + h), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
        d2dBitmap->Release();
    }
    rt->PopLayer();
}

ID2D1Bitmap* RenderEngineDirect2D::createBitmapFromWicBitmap(ID2D1RenderTarget* rt, IWICBitmapSource* wicSource)
{
    if (!rt || !wicSource) {
        return NULL;
    }
    ID2D1Bitmap* d2dBitmap = NULL;
    HRESULT hr = rt->CreateBitmapFromWicBitmap(wicSource, NULL, &d2dBitmap);
    if (FAILED(hr) || !d2dBitmap) {
        return NULL;
    }
    return d2dBitmap;
}

} // namespace yanbo
