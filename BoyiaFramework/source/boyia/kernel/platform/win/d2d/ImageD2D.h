#ifndef ImageD2D_h
#define ImageD2D_h

#include "../ImageWin.h"
#include <windows.h>
#include <wincodec.h>

#pragma comment(lib, "windowscodecs.lib")

namespace util {

class ImageD2D : public ImageWin {
public:
    ImageD2D();
    virtual ~ImageD2D();

    static IWICBitmap* createD2DImage(const OwnerPtr<String>& data);

public:
    virtual LVoid* pixels() const;
    virtual LVoid setData(const OwnerPtr<String>& data);
    virtual LVoid setImage(LVoid* image);

private:
    IWICBitmap* m_wicBitmap;
};

} // namespace util

#endif
