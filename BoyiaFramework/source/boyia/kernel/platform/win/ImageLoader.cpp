#include "ImageLoader.h"
#include "AppManager.h"
#include "ImageWin.h"
#include "StringBuilder.h"
#include "UIThreadClientMap.h"
#include "HashMap.h"
#include "HashUtil.h"
#include <windows.h>
#include <GdiPlus.h>

namespace yanbo {
class HashImageCacheMap {
public:
    Gdiplus::Image* get(const String& url)
    {
        return m_map.get(HashString(url, LFalse)).get();
    }

    LVoid put(const String& url, Gdiplus::Image* image) 
    {
        m_map.put(url, image);
    }

private:
    HashMap<HashString, OwnerPtr<Gdiplus::Image>> m_map;
};

class HashImageLoadingMap {
public:
    LBool hasLoading(const String& url)
    {
        return m_loadingMap.contains(url);
    }
private:
    HashMap<HashString, OwnerPtr<KVector<LInt>>> m_loadingMap;
};

class ImageLoaderClient : public NetworkClient, public UIEvent {
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
        UIThread::instance()->sendUIEvent(this);
    }

    virtual LVoid run()
    {
        OwnerPtr<String> data = m_builder.toString();
        util::ImageWin* image = static_cast<util::ImageWin*>(UIThreadClientMap::instance()->findUIThreadClient(m_id));
        if (image) {
            image->setData(data);
            image->onClientCallback();
        }
    }

private:
    LInt m_id;
    StringBuilder m_builder;
};

ImageLoader* ImageLoader::instance()
{
    static ImageLoader sImageLoader;
    return &sImageLoader;
}

LVoid ImageLoader::loadImage(const String& url, LInt clientId)
{
    AppManager::instance()->network()->loadUrl(url, new ImageLoaderClient(clientId), LFalse);
}
}