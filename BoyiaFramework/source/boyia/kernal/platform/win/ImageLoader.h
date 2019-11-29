#ifndef  ImageLoader_h

#include "UtilString.h"

namespace yanbo {
class ImageLoader {
public:
    static ImageLoader* instance();
    LVoid loadImage(const String& url, LInt clientId);
};
}

#endif
