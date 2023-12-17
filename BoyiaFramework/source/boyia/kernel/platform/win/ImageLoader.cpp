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
        return m_map.get(HashString(url, LFalse));
    }

    LVoid put(const String& url, Gdiplus::Image* image) 
    {
        m_map.put(url, image);
    }

private:
    HashMap<HashString, Gdiplus::Image*> m_map;
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
            m_loadingMap.put(url, array);
        }
    }

    Gdiplus::Image* flushImageLoading(const String& url, const OwnerPtr<String>& data)
    {   
        BoyiaPtr<KVector<LInt>> ptr = m_loadingMap.get(url);
        if (!ptr) {
            return kBoyiaNull;
        }

        Gdiplus::Image* winImage = util::ImageWin::createWinImage(data);
        for (LInt i = 0; i < ptr->size(); i++) {
            LInt clientId = ptr->elementAt(i);
           
            util::ImageWin* image = static_cast<util::ImageWin*>(UIThreadClientMap::instance()->findUIThreadClient(clientId));
            if (image) {
                image->setImage(winImage);
                image->onClientCallback();
            }
        }

        return winImage;
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
    Gdiplus::Image* image = m_loadingMap->flushImageLoading(url, data);
    if (image) {
        m_imageCache->put(url, image);
    }
}

LVoid ImageLoader::loadImage(const String& url, LInt clientId)
{
    Gdiplus::Image* image = m_imageCache->get(url);
    // 若缓存中存在，则直接取出使用
    if (image) {
        util::ImageWin* imageWin = static_cast<util::ImageWin*>(UIThreadClientMap::instance()->findUIThreadClient(clientId));
        if (imageWin) {
            imageWin->setImage(image);
            imageWin->onClientCallback();
        }
        return;
    }

    // 如果正在下载，加入到下载集合中
    if (m_loadingMap->hasLoading(url)) {
        m_loadingMap->appendLoading(url, clientId);
        return;
    }

    // 否则加入到下载集合
    m_loadingMap->appendLoading(url, clientId);
    // 开始下载图片
    AppManager::instance()->network()->loadUrl(url, new ImageLoaderClient(url), LFalse);
}
}