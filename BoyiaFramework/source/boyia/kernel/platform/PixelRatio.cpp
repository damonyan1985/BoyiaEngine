#include "PixelRatio.h"

namespace yanbo {
float PixelRatio::s_width = 0;
float PixelRatio::s_height = 0;

float PixelRatio::s_logicWidth = 0;
float PixelRatio::s_logicHeight = 0;

PixelRatio::PixelDirection PixelRatio::s_direction = PixelRatio::kPixelHorizontal;

void PixelRatio::setWindowSize(int width, int height)
{
    s_width = width;
    s_height = height;
}

void PixelRatio::setLogicWindowSize(int width, int height)
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
    case PixelRatio::kPixelHorizontal:
        return s_width / s_logicWidth;
    case PixelRatio::kPixelVertical:
        return s_height / s_logicHeight;
    default:
        return 1;
    }
}

int PixelRatio::viewX(int x)
{
    // switch (s_direction) {
    // case PixelRatio::kPixelHorizontal:
    //     return x * s_logicWidth / s_width;
    // case PixelRatio::kPixelVertical:
    //     return x * s_logicHeight / s_height;
    // default:
    //     return x;
    // }
    return x / ratio();
}

int PixelRatio::viewY(int y)
{
    //return viewX(y);
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

}