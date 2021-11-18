#ifndef PixelRatio_h
#define PixelRatio_h

#include "LGraphic.h"

namespace yanbo {
class PixelRatio {
public:
    enum PixelDirection {
        kPixelVertical,
        kPixelHorizontal
    };

    static void setWindowSize(int width, int height);
    static void setLogicWindowSize(int width, int height);
    
    static bool isInWindow(const LRect& rect);
    static bool isInClipRect(const LRect& rect, const LRect& clipRect);

    static void setRatioDirection(PixelDirection direction);
    static float ratio();
    static float vhRatio();

    static int viewX(int x);
    static int viewY(int y);

    static int logicWidth();
    static int logicHeight();

    static LVoid clipRect(const LRect& srcRect, const LRect& clipRect, LRect& destRect);

private:
    static float s_width;
    static float s_height;

    static float s_logicWidth;
    static float s_logicHeight;

    static PixelDirection s_direction;
};
}
#endif
