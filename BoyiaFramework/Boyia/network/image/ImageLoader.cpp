#include "ImageLoader.h"
#include "BoyiaLoader.h"
#include "StringBuilder.h"
#include "UIThread.h"

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
    KVector<ImageItem*>& map = ImageLoader::instance()->map();
    LInt index = m_loadId % map.capacity();
    ImageItem* item = map[index];
    if (item->id == m_loadId) {
        map[index] = item->next;
        delete item;
    } else {
        ImageItem* next = item->next;
        while (next) {
            if (next->id == m_loadId) {
                item->next = next->next;
                delete next;
                break;
            }

            item = next;
            next = next->next;
        }
    }
}

LVoid ImageClient::setLoadId(LInt id)
{
    m_loadId = id;
}

LInt ImageClient::getLoadId() const
{
    return m_loadId;
}

class ImageLoadedEvent : public UIEvent {
public:
    ImageLoadedEvent(LInt id, OwnerPtr<String> data)
        : m_id(id)
        , m_data(data)
    {
    }

    virtual LVoid run()
    {
        KVector<ImageItem*>& map = ImageLoader::instance()->map();
        LInt index = m_id % map.capacity();
        ImageItem* item = map[index];

        // Item maybe delete by uithread before this even run
        if (!item || !item->client) {
            return;
        }

        if (item->id == m_id) {
            map[index] = item->next;
            item->client->setData(*m_data.get());
            delete item;
        } else {
            ImageItem* next = item->next;
            while (next) {
                if (next->id == m_id) {
                    item->next = next->next;
                    next->client->setData(*m_data.get());
                    delete next;
                    break;
                }

                item = next;
                next = next->next;
            }
        }
    }

private:
    OwnerPtr<String> m_data;
    LInt m_id;
};

class ImageLoaderClient : public NetworkClient {
public:
    ImageLoaderClient(LInt id)
        : m_id(id)
    {
    }
    virtual LVoid onDataReceived(const LByte* data, LInt size)
    {
        LByte* destData = new LByte[size];
        util::LMemcpy(destData, data, size);
        m_builder.append(destData, 0, size, LFalse);
    }
    virtual LVoid onStatusCode(LInt statusCode)
    {
    }
    virtual LVoid onFileLen(LInt len)
    {
    }
    virtual LVoid onRedirectUrl(const String& redirectUrl)
    {
    }
    virtual LVoid onLoadError(LInt error)
    {
        delete this;
    }
    virtual LVoid onLoadFinished()
    {
        OwnerPtr<String> data = m_builder.toString();
        // m_client->setData(*data.get());
        // send to ui thread
        UIThread::instance()->sendUIEvent(new ImageLoadedEvent(m_id, data));
        delete this;
    }

private:
    LInt m_id;
    StringBuilder m_builder;
};

// only call in ui thread
static LInt sImageId = 0;
static LInt ImageIdCreate()
{
    return ++sImageId;
}

// create a size and capactiy vector
ImageLoader::ImageLoader()
    : m_map(kEnlargeCapacity)
    , m_loader(new BoyiaLoader())
{
    // Init all pointer to NULL
    for (LInt i = 0; i < m_map.capacity(); ++i) {
        m_map[i] = NULL;
    }
}

ImageLoader* ImageLoader::instance()
{
    static ImageLoader sImageLoader;
    return &sImageLoader;
}

KVector<ImageItem*>& ImageLoader::map()
{
    return m_map;
}

// Only use in ui thread
LVoid ImageLoader::loadImage(const String& url, ImageClient* client)
{
    LInt id = ImageIdCreate();
    ImageItem* item = new ImageItem();

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

    m_loader->loadUrl(url, new ImageLoaderClient(id));
}
}
