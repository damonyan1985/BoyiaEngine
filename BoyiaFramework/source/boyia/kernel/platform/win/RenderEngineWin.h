#ifndef RenderEngineWin_h
#define RenderEngineWin_h

#include "IRenderEngine.h"
#include <windows.h>
#include <GdiPlus.h>

namespace yanbo {
class RenderEngineWin : public IRenderEngine {
public:
    RenderEngineWin();
    virtual ~RenderEngineWin();
    virtual LVoid init();
    virtual LVoid reset();
    virtual LVoid render(RenderLayer* layer);

private:
    ULONG_PTR m_gdiplusToken;
    Gdiplus::Bitmap* m_cacheBitmap;
};
}
#endif
