#include "RenderEngineWin.h"

namespace yanbo {
RenderEngineWin::RenderEngineWin()
    : m_cacheBitmap(kBoyiaNull)
{
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
    Gdiplus::GdiplusStartupInput startupInput;
    GdiplusStartup(&m_gdiplusToken, &startupInput, NULL);
}

LVoid RenderEngineWin::reset()
{

}

LVoid RenderEngineWin::render(RenderLayer* layer)
{

}

IRenderEngine* IRenderEngine::create()
{
    return new RenderEngineWin();
}
}