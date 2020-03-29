#include "PixelRatio.h"

namespace yanbo {
float PixelRatio::s_width = 0;
float PixelRatio::s_height = 0;
void PixelRatio::setWindowSize(int width, int height) {
    s_width = width;
    s_height = height;
}

float PixelRatio::ratio() {
    return s_width / 720;
}
}