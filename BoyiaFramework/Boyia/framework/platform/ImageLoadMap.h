#ifndef ImageLoadeMap_h
#define ImageLoadeMap_h

#include "KVector.h"

namespace yanbo {

class ImageClient {
public:
    ImageClient();
    virtual ~ImageClient();
    virtual LVoid onImageLoaded() = 0;
    LVoid setLoadId(LInt id);
    LInt getLoadId() const;

private:
    LInt m_loadId;
};

class ImageItem;
class ImageLoadMap {
public:
    static ImageLoadMap* instance();
    LVoid registerImage(ImageClient* client);
    KVector<ImageItem*>& map();
    LVoid removeItem(LInt id);
    LVoid clientCallback(ImageClient* client);

private:
    ImageLoadMap();
    ImageItem* getImageItem(LInt id);

    KVector<ImageItem*> m_map;
};
}

#endif