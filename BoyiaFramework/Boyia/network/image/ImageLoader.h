#ifndef ImageLoader_h
#define ImageLoader_h

#include "KVector.h"
#include "NetworkBase.h"

namespace yanbo {

class ImageClient {
public:
    ImageClient();
    virtual ~ImageClient();
    virtual LVoid setData(const String& data) = 0;
    LVoid setLoadId(LInt id);

private:
    LInt m_loadId;
};

class ImageItem;
class ImageLoader {
public:
    static ImageLoader* instance();
    LVoid loadImage(const String& url, ImageClient* client);

    const KVector<ImageItem*>& map() const;

private:
    ImageLoader();

    NetworkBase* m_loader;
    KVector<ImageItem*> m_map;
};
}

#endif