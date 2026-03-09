#include "ImageLoader.h"
#include "AppManager.h"
#include "ImageWin.h"
#include "ImageGDIPlus.h"
#include "ImageD2D.h"
#include "StringBuilder.h"
#include "UIThreadClientMap.h"
#include "HashMap.h"
#include "HashUtil.h"
#include <windows.h>
#include <GdiPlus.h>
#include <wincodec.h>

namespace yanbo {

static LVoid setImageToClient(UIThreadClient* client, LVoid* image)
{
    if (!client) {
        return;
    }

    util::ImageWin* winImage = static_cast<util::ImageWin*>(client);
    if (winImage) {
        winImage->setImage(image);
        winImage->onClientCallback();
        return;
    }
}

class HashImageCacheMap {
public:
    LVoid* get(const String& url)
    {
        return m_cacheMap.get(HashString(url, LFalse));
    }

    LVoid put(const String& url, LVoid* image)
    {
        m_cacheMap.put(url, image);
    }

private:
    HashMap<HashString, LVoid*> m_cacheMap;
};

class HashImageLoadingMap {
public:
    LBool hasLoading(const String& url)
    {
        return m_loadingMap.contains(HashString(url, LFalse));
    }

    LVoid appendLoading(const String& url, LInt clientId)
    {
        if (hasLoading(url)) {
            m_loadingMap.get(HashString(url, LFalse))->addElement(clientId);
        } else {
            KVector<LInt>* array = new KVector<LInt>(0, 50);
            array->addElement(clientId);
            m_loadingMap.put(HashString(url, LFalse), array);
        }
    }

    LVoid* flushImageLoading(const String& url, const OwnerPtr<String>& data)
    {
        BoyiaPtr<KVector<LInt>> ptr = m_loadingMap.get(HashString(url, LFalse));
        if (!ptr) {
            return kBoyiaNull;
        }

        LVoid* image = kBoyiaNull;
        if (util::ImageWin::isHardwareAccelerated()) {
            image = util::ImageD2D::createD2DImage(data);
        } else {
            image = util::ImageGDIPlus::createWinImage(data);
        }

        if (!image) {
            return kBoyiaNull;
        }

        for (LInt i = 0; i < ptr->size(); i++) {
            LInt clientId = ptr->elementAt(i);
            UIThreadClient* client = UIThreadClientMap::instance()->findUIThreadClient(clientId);
            setImageToClient(client, image);
        }

        return image;
    }

private:
    HashMap<HashString, BoyiaPtr<KVector<LInt>>> m_loadingMap;
};

class ImageLoaderClient : public NetworkClient, public UIEvent {
public:
    ImageLoaderClient(const String& url)
        : m_url(url)
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
        ImageLoader::instance()->flushImageLoading(m_url, data);
    }

private:
    String m_url;
    StringBuilder m_builder;
};

ImageLoader* ImageLoader::instance()
{
    static ImageLoader sImageLoader;
    return &sImageLoader;
}

ImageLoader::ImageLoader()
{
    m_imageCache = new HashImageCacheMap();
    m_loadingMap = new HashImageLoadingMap();
}

LVoid ImageLoader::flushImageLoading(const String& url, const OwnerPtr<String>& data)
{
    LVoid* image = m_loadingMap->flushImageLoading(url, data);
    if (image) {
        m_imageCache->put(url, image);
    }
}

LVoid ImageLoader::loadImage(const String& url, LInt clientId)
{
    LVoid* image = m_imageCache->get(url);
    if (image) {
        UIThreadClient* client = UIThreadClientMap::instance()->findUIThreadClient(clientId);
        setImageToClient(client, image);
        return;
    }

    if (m_loadingMap->hasLoading(url)) {
        m_loadingMap->appendLoading(url, clientId);
        return;
    }

    m_loadingMap->appendLoading(url, clientId);
    AppManager::instance()->network()->loadUrl(url, new ImageLoaderClient(url), LFalse);
}
}
