#include "RenderEngineWin.h"
#include "CharConvertor.h"
#include "PixelRatio.h"

namespace yanbo {
RenderEngineWin::RenderEngineWin()
    : m_cacheBitmap(kBoyiaNull)
{
    // 初始化GDI+库
    Gdiplus::GdiplusStartupInput startupInput;
    GdiplusStartup(&m_gdiplusToken, &startupInput, NULL);

    init();
}

RenderEngineWin::~RenderEngineWin()
{
    if (m_cacheBitmap) {
        delete m_cacheBitmap;
    }
    Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

LVoid RenderEngineWin::init()
{
    // 初始化渲染函数
    m_functions[RenderCommand::kRenderRect] = (RenderFunction)&RenderEngineWin::renderRect;
    m_functions[RenderCommand::kRenderText] = (RenderFunction)&RenderEngineWin::renderText;
    m_functions[RenderCommand::kRenderImage] = (RenderFunction)&RenderEngineWin::renderImage;
    m_functions[RenderCommand::kRenderRoundRect] = (RenderFunction)& RenderEngineWin::renderRoundRect;
}

LVoid RenderEngineWin::reset()
{
    if (m_cacheBitmap) {
        HDC dc = ::GetDC(m_hwnd);
        Gdiplus::Graphics gc(m_hwnd);
        Gdiplus::CachedBitmap cachedBmp(m_cacheBitmap, &gc);
        gc.DrawCachedBitmap(&cachedBmp, 0, 0);
        ::ReleaseDC(m_hwnd, dc);
    }
}

LVoid RenderEngineWin::render(RenderLayer* layer)
{
    if (!m_cacheBitmap) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        m_cacheBitmap = new Gdiplus::Bitmap(int(rc.right), int(rc.bottom));
    }

    // 创建缓冲绘图对象
    Gdiplus::Graphics cacheGc(m_cacheBitmap);
    // 开始渲染layer
    renderImpl(layer, cacheGc);
    HDC dc = ::GetDC(m_hwnd);

    Gdiplus::Graphics gc(m_hwnd);
    Gdiplus::CachedBitmap cachedBmp(m_cacheBitmap, &gc);
    gc.DrawCachedBitmap(&cachedBmp, 0, 0);
    ::ReleaseDC(m_hwnd, dc);
}

LVoid RenderEngineWin::setContextWin(HWND hwnd)
{
    m_hwnd = hwnd;
}

LVoid RenderEngineWin::renderImpl(RenderLayer* layer, Gdiplus::Graphics& gc)
{
    if (!layer) {
        return;
    }

    // 先渲染当前layer
    if (layer->m_buffer) {
        LInt commandSize = layer->m_buffer->size();
        for (LInt i = 0; i < commandSize; i++) {
            RenderCommand* cmd = layer->m_buffer->elementAt(i).get();
            (this->*(m_functions[cmd->type()]))(cmd, gc);
        }
    }
    
    // 再渲染子layer
    for (LInt i = 0; i < layer->m_children.size(); i++) {
        renderImpl(layer->m_children[i], gc);
    }
}

LVoid RenderEngineWin::renderRect(RenderCommand* cmd, Gdiplus::Graphics& gc)
{
    float ratio = yanbo::PixelRatio::ratio();
    RenderRectCommand* resource = static_cast<RenderRectCommand*>(cmd);
    float width = resource->rect.GetWidth() * yanbo::PixelRatio::ratio();
    width = width < 1 ? 1 : width;
    float height = resource->rect.GetHeight() * yanbo::PixelRatio::ratio();
    height = height < 1 ? 1 : height;
    Gdiplus::RectF rect(
        resource->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
        resource->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
        width,
        height
    );

    Gdiplus::SolidBrush brush(Gdiplus::Color(
        resource->color.m_alpha,
        resource->color.m_red,
        resource->color.m_green,
        resource->color.m_blue
    ));
    gc.FillRectangle(&brush, rect);
}

LVoid RenderEngineWin::renderText(RenderCommand* cmd, Gdiplus::Graphics& gc)
{
    RenderTextCommand* resource = static_cast<RenderTextCommand*>(cmd);
    wstring wtext = yanbo::CharConvertor::CharToWchar(GET_STR(resource->text));
    Gdiplus::Font font(L"Arial", resource->font.getFontSize() * yanbo::PixelRatio::ratio(),
        Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

    Gdiplus::StringFormat format(Gdiplus::StringAlignmentNear);
    Gdiplus::RectF rect(
        resource->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
        resource->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
        resource->rect.GetWidth(),
        resource->rect.GetHeight()
    );

    Gdiplus::SolidBrush brush(Gdiplus::Color(
        resource->color.m_alpha,
        resource->color.m_red,
        resource->color.m_green,
        resource->color.m_blue
    ));

    gc.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    gc.DrawString(wtext.c_str(), wtext.length(), &font, rect, &format, &brush);
}

LVoid RenderEngineWin::renderImage(RenderCommand* cmd, Gdiplus::Graphics& gc)
{
    RenderImageCommand* resource = static_cast<RenderImageCommand*>(cmd);
    Gdiplus::Rect rect(
        resource->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
        resource->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
        resource->rect.GetWidth() * yanbo::PixelRatio::ratio(),
        resource->rect.GetHeight() * yanbo::PixelRatio::ratio()
    );
    gc.DrawImage(static_cast<Gdiplus::Image*>(resource->image), rect);
}

LVoid RenderEngineWin::renderRoundRect(RenderCommand* cmd, Gdiplus::Graphics& gc)
{
    
}

IRenderEngine* IRenderEngine::create()
{
    return new RenderEngineWin();
}
}