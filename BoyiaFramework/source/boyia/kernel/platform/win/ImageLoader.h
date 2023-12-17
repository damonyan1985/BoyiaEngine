#ifndef ImageLoader_h
#define ImageLoader_h

#include "UtilString.h"

namespace yanbo {
class HashImageCacheMap;
class HashImageLoadingMap;
class ImageLoader {
public:
    static ImageLoader* instance();
    LVoid loadImage(const String& url, LInt clientId);

private:
    HashImageCacheMap* m_imageMap;
    HashImageLoadingMap* m_loadingMap;
};
}

#endif
