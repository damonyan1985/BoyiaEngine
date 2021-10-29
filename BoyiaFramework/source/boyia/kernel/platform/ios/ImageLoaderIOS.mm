#include "ImageLoaderIOS.h"
#include "AppManager.h"
#include "ImageIOS.h"
#include "StringBuilder.h"
#include "UIThreadClientMap.h"

namespace yanbo {
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
        util::ImageIOS* image = static_cast<util::ImageIOS*>(UIThreadClientMap::instance()->findUIThreadClient(m_id));
        if (image) {
            image->setData(data);
            image->onClientCallback();
        }
    }

private:
    LInt m_id;
    StringBuilder m_builder;
};

ImageLoaderIOS* ImageLoaderIOS::instance()
{
    static ImageLoaderIOS sImageLoader;
    return &sImageLoader;
}

LVoid ImageLoaderIOS::loadImage(const String& url, LInt clientId)
{
    // 清除HTTP请求头
    AppManager::instance()->network()->clearHeaders();
    // 开始加载图片
    AppManager::instance()->network()->loadUrl(url, new ImageLoaderClient(clientId), LFalse);
}
}
