#ifndef RenderEngineDirect2D_h
#define RenderEngineDirect2D_h

#include "../../../render/IRenderEngine.h"
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace yanbo {

class RenderEngineDirect2D;
typedef LVoid (RenderEngineDirect2D::*RenderFunction)(RenderCommand* cmd, ID2D1RenderTarget* rt);

class RenderEngineDirect2D : public IRenderEngine {
public:
    RenderEngineDirect2D();
    virtual ~RenderEngineDirect2D();

    virtual LVoid init();
    virtual LVoid reset();
    virtual LVoid render(RenderLayer* layer);

    LVoid setContextWin(HWND hwnd);

private:
    LVoid renderImpl(RenderLayer* layer, ID2D1RenderTarget* rt);
    LVoid renderRect(RenderCommand* cmd, ID2D1RenderTarget* rt);
    LVoid renderImage(RenderCommand* cmd, ID2D1RenderTarget* rt);
    LVoid renderText(RenderCommand* cmd, ID2D1RenderTarget* rt);
    LVoid renderRoundRect(RenderCommand* cmd, ID2D1RenderTarget* rt);
    LVoid renderVideo(RenderCommand* cmd, ID2D1RenderTarget* rt);
    LVoid renderRoundImage(RenderCommand* cmd, ID2D1RenderTarget* rt);

    ID2D1Bitmap* createBitmapFromWicBitmap(ID2D1RenderTarget* rt, IWICBitmapSource* wicSource);
    ID2D1PathGeometry* createRoundedRectGeometry(RenderRoundRectCommand* cmd);

    HWND m_hwnd;
    ID2D1Factory* m_factory;
    ID2D1HwndRenderTarget* m_hwndTarget;
    ID2D1BitmapRenderTarget* m_bitmapTarget;
    ID2D1Bitmap* m_cacheBitmap;
    IDWriteFactory* m_dwriteFactory;
    RenderFunction m_functions[7];
};

} // namespace yanbo

#endif
