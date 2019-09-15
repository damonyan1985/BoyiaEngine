#include "ImageLoader.h"
#include "StringBuilder.h"

namespace yanbo {

class ImageLoaderClient : public NetworkClient {
public:
    ImageLoaderClient(ImageClient* client)
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
        m_client->setData(*data.get());
        delete this;
    }

private:
    ImageClient* m_client;
    StringBuilder m_builder;
};

ImageLoader* ImageLoader::instance()
{
    static ImageLoader sImageLoader;
    return &sImageLoader;
}

LVoid ImageLoader::loadImage(const String& url, ImageClient* client)
{
    m_loader.loadUrl(url, new ImageLoaderClient(client));
}
}
