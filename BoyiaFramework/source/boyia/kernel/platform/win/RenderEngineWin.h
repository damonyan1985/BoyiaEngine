#ifndef RenderEngineWin_h
#define RenderEngineWin_h

#include "IRenderEngine.h"
#include <windows.h>
#include <GdiPlus.h>

namespace yanbo {
// Windows平台渲染引擎
class RenderEngineWin;
typedef LVoid (RenderEngineWin::*RenderFunction)(RenderCommand* cmd, Gdiplus::Graphics& gc);
class RenderEngineWin : public IRenderEngine {
public:
    RenderEngineWin();
    virtual ~RenderEngineWin();
    virtual LVoid init();
    virtual LVoid reset();
    virtual LVoid render(RenderLayer* layer);

    LVoid setContextWin(HWND hwnd);

private:
    LVoid renderImpl(RenderLayer* layer, Gdiplus::Graphics& gc);
    LVoid renderRect(RenderCommand* cmd, Gdiplus::Graphics& gc);
    LVoid renderImage(RenderCommand* cmd, Gdiplus::Graphics& gc);
    LVoid renderText(RenderCommand* cmd, Gdiplus::Graphics& gc);
    LVoid renderRoundRect(RenderCommand* cmd, Gdiplus::Graphics& gc);

    HWND m_hwnd;
    ULONG_PTR m_gdiplusToken;
    Gdiplus::Bitmap* m_cacheBitmap;
    RenderFunction m_functions[6];
};
}
#endif
