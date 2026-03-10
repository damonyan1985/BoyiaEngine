#ifndef ImageGDIPlus_h
#define ImageGDIPlus_h

#include "../ImageWin.h"
#include <GdiPlus.h>

namespace util {

class ImageGDIPlus : public ImageWin {
public:
    ImageGDIPlus();
    virtual ~ImageGDIPlus();

    static Gdiplus::Image* createWinImage(const OwnerPtr<String>& data);

public:
    virtual LVoid* pixels() const;
    virtual LVoid setData(const OwnerPtr<String>& data);
    virtual LVoid setImage(LVoid* image);

private:
    Gdiplus::Image* m_winImage;
};

} // namespace util

#endif
