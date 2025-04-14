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

    // 外部传入的物理宽高
    static void setWindowSize(LReal width, LReal height);
    // 内核使用逻辑高度
    static void setLogicWindowSize(LReal width, LReal height);
    
    static bool isInWindow(const LRect& rect);
    static bool isInClipRect(const LRect& rect, const LRect& clipRect);

    static void setRatioDirection(PixelDirection direction);
    static float ratio();
    static float vhRatio();

    // 物理坐标转引擎的逻辑坐标
    static int viewX(int x);
    static int viewY(int y);

    // 引擎的逻辑坐标转屏幕物理坐标
    static int rawX(int x);
    static int rawY(int y);

    // 引擎逻辑宽高
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
