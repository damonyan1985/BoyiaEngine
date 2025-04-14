#include "PixelRatio.h"

namespace yanbo {
float PixelRatio::s_width = 0;
float PixelRatio::s_height = 0;

float PixelRatio::s_logicWidth = 0;
float PixelRatio::s_logicHeight = 0;

// 纵向默认以宽为基准
PixelRatio::PixelDirection PixelRatio::s_direction = PixelRatio::kPixelVertical;

// 外部传入的物理宽高
void PixelRatio::setWindowSize(LReal width, LReal height)
{
    s_width = width;
    s_height = height;
}

// 内核使用逻辑高度
void PixelRatio::setLogicWindowSize(LReal width, LReal height)
{
    s_logicWidth = width;
    s_logicHeight = height;
}

void PixelRatio::setRatioDirection(PixelDirection direction)
{
    s_direction = direction;
}

float PixelRatio::vhRatio()
{
    return (s_logicWidth / s_logicHeight) / (s_width / s_height);
}

float PixelRatio::ratio()
{
    switch (s_direction) {
    case PixelRatio::kPixelVertical:
        return s_width / s_logicWidth;
    case PixelRatio::kPixelHorizontal:
        return s_height / s_logicHeight;
    default:
        return 1;
    }
}

int PixelRatio::viewX(int x)
{
    return x / ratio();
}

int PixelRatio::viewY(int y)
{
    return y / ratio();
}

int PixelRatio::logicWidth()
{
    return s_logicWidth;
}

int PixelRatio::logicHeight()
{
    return s_logicHeight;
}

bool PixelRatio::isInWindow(const LRect& rect)
{
    if (rect.iBottomRight.iX < 0
        || rect.iBottomRight.iY < 0
        || rect.iTopLeft.iX > logicWidth()
        || rect.iTopLeft.iY > logicHeight()) {
        return false;
    }
    
    return true;
}

bool PixelRatio::isInClipRect(const LRect& rect, const LRect& clipRect)
{
    return !(rect.iBottomRight.iX < clipRect.iTopLeft.iX
             || rect.iBottomRight.iY < clipRect.iTopLeft.iY
             || rect.iTopLeft.iX > clipRect.iBottomRight.iX
             || rect.iTopLeft.iY > clipRect.iBottomRight.iY);
}

LVoid PixelRatio::clipRect(const LRect& srcRect, const LRect& clipRect, LRect& destRect)
{
    LInt left = clipRect.iTopLeft.iX <= srcRect.iTopLeft.iX ?
        srcRect.iTopLeft.iX : clipRect.iTopLeft.iX;
    LInt top = clipRect.iTopLeft.iY <= srcRect.iTopLeft.iY ?
        srcRect.iTopLeft.iY : clipRect.iTopLeft.iY;

    LInt right = clipRect.iBottomRight.iX >= srcRect.iBottomRight.iX ?
        srcRect.iBottomRight.iX : clipRect.iBottomRight.iX;
    LInt bottom = clipRect.iBottomRight.iY >= srcRect.iBottomRight.iY ?
        srcRect.iBottomRight.iY : clipRect.iBottomRight.iY;

    destRect.Set(LPoint(left, top), LPoint(right, bottom));    
}

int PixelRatio::rawX(int x)
{
    return x * ratio();
}

int PixelRatio::rawY(int y)
{
    return y * ratio();
}

}
