#include "ImageLoader.h"
#include "AppManager.h"
#include "StringBuilder.h"
#include "UIThreadClientMap.h"
#include "ImageWin.h"

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
        util::ImageWin* image = static_cast<util::ImageWin*>(UIThreadClientMap::instance()->getUIThreadClient(m_id));
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
    AppManager::instance()->network()->loadUrl(url, new ImageLoaderClient(clientId));
}
}