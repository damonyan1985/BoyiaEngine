#ifndef ImageLoader_h
#define ImageLoader_h

#include "UtilString.h"

namespace yanbo {
class ImageLoaderIOS {
public:
    static ImageLoaderIOS* instance();
    LVoid loadImage(const String& url, LInt clientId);
};
}

#endif
