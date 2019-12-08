#ifndef PixelRatio_h
#define PixelRatio_h

namespace yanbo {
class PixelRatio {
public:
    static void setWindowSize(int width, int height);
    static float ratio();

private:
    static float s_width;
    static float s_height;
};
}
#endif
