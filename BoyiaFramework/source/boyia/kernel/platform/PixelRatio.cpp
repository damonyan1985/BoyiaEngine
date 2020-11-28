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
}