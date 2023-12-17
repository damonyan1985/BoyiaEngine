#ifndef ImageLoader_h
#define ImageLoader_h

#include "UtilString.h"
#include "OwnerPtr.h"

namespace yanbo {
class HashImageCacheMap;
class HashImageLoadingMap;
class ImageLoader {
public:
    static ImageLoader* instance();
    LVoid loadImage(const String& url, LInt clientId);
    LVoid flushImageLoading(const String& url, const OwnerPtr<String>& data);

private:
    ImageLoader();
   
    HashImageCacheMap* m_imageCache;
    HashImageLoadingMap* m_loadingMap;
};
}

#endif
