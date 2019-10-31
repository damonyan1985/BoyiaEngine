#ifndef ImageLoader_h
#define ImageLoader_h

#include "ImageInfo.h"
#include "KVector.h"
#include "NetworkBase.h"

namespace yanbo {

class ImageClient {
public:
    ImageClient();
    virtual ~ImageClient();
    virtual LVoid setImageInfo(const ImageInfo& info) = 0;
    LVoid setLoadId(LInt id);
    LInt getLoadId() const;

private:
    LInt m_loadId;
};

class ImageItem;
class ImageLoadedEvent;
class ImageLoader {
public:
    static ImageLoader* instance();
    LVoid loadImage(const String& url, ImageClient* client);
    KVector<ImageItem*>& map();

private:
    ImageLoader();

    NetworkBase* m_loader;
    KVector<ImageItem*> m_map;
};
}

#endif