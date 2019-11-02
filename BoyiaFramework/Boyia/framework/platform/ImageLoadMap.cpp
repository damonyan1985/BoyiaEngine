#include "ImageLoadMap.h"
#include "StringBuilder.h"

namespace yanbo {

class ImageItem {
public:
    ImageItem()
        : client(NULL)
        , id(0)
        , next(NULL)
    {
    }

    ImageClient* client;
    LInt id;
    ImageItem* next;
};

ImageClient::ImageClient()
    : m_loadId(0)
{
}

ImageClient::~ImageClient()
{
    ImageLoadMap::instance()->removeItem(m_loadId);
}

LVoid ImageClient::setLoadId(LInt id)
{
    m_loadId = id;
}

LInt ImageClient::getLoadId() const
{
    return m_loadId;
}

// only call in ui thread
static LInt sImageId = 0;
static LInt ImageIdCreate()
{
    return ++sImageId;
}

// create a size and capactiy vector
ImageLoadMap::ImageLoadMap()
    : m_map(kEnlargeCapacity)
{
    // Init all pointer to NULL
    for (LInt i = 0; i < m_map.capacity(); ++i) {
        m_map[i] = NULL;
    }
}

LVoid ImageLoadMap::clientCallback(LInt id)
{
    KFORMATLOG("ImageLoadMap::clientCallback clientId=%d", id);
    ImageItem* item = getImageItem(id);
    if (item) {
        KFORMATLOG("ImageLoadMap::clientCallback item->client=%ld", (long)item->client);
        item->client->onImageLoaded();
        delete item;
    }
}

ImageLoadMap* ImageLoadMap::instance()
{
    static ImageLoadMap sImageLoadMap;
    return &sImageLoadMap;
}

KVector<ImageItem*>& ImageLoadMap::map()
{
    return m_map;
}

ImageItem* ImageLoadMap::getImageItem(LInt id)
{
    LInt index = id % m_map.capacity();
    ImageItem* item = m_map[index];
    KFORMATLOG("ImageLoadMap::getImageItem item=%ld", (long)item);
    // Item maybe delete by uithread before this even run
    if (!item || !item->client) {
        return NULL;
    }

    KFORMATLOG("ImageLoadMap::getImageItem item->id=%d", item->id);
    if (item->id == id) {
        m_map[index] = item->next;
        return item;
    } else {
        ImageItem* next = item->next;
        while (next) {
            KFORMATLOG("ImageLoadMap::getImageItem next->id=%d", next->id);
            if (next->id == id) {
                item->next = next->next;
                return next;
            }

            item = next;
            next = next->next;
        }
    }
}

LVoid ImageLoadMap::removeItem(LInt id)
{
    ImageItem* item = getImageItem(id);
    if (item) {
        delete item;
    }
}

// Only use in ui thread
LVoid ImageLoadMap::registerImage(ImageClient* client)
{
    KFORMATLOG("ImageLoadMap::registerImage client=%ld", (long)client);
    LInt id = ImageIdCreate();
    ImageItem* item = new ImageItem();

    KFORMATLOG("ImageLoadMap::registerImage id=%d", (long)id);
    client->setLoadId(id);
    item->id = id;
    item->client = client;
    //item->m_map.addElement(item);
    LInt index = id % m_map.capacity();
    ImageItem* mapItem = m_map[index];
    m_map[index] = item;
    if (mapItem) {
        item->next = mapItem;
    }
}
}