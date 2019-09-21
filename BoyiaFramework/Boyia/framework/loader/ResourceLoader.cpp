/*
 * ResourceLoader.cpp
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */
#include "ResourceLoader.h"
#include "AppThread.h"
#include "SalLog.h"
#include "StringBuilder.h"
#include "StringUtils.h"
#include "UIView.h"
#include <android/log.h>

extern LVoid CompileScript(char* code);
namespace yanbo {

class ResourceHandle : public NetworkClient, public AppEvent {
public:
    ResourceHandle(ResourceLoader* loader, LInt type, const String& url)
        : m_loader(loader)
        , m_result(NetworkClient::kNetworkSuccess)
        , m_resType(type)
        , m_url(url)
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

    virtual LVoid onRedirectUrl(const String& redirectUrl)
    {
    }

    virtual LVoid onLoadError(LInt error)
    {
        //m_loader->onLoadError(error);
        m_result = error;
        AppThread::instance()->sendEvent(this);
    }

    virtual LVoid onLoadFinished()
    {
        m_data = m_builder.toString();
        AppThread::instance()->sendEvent(this);
    }

    virtual LVoid run()
    {
        __android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "ResourceEvent::run");

        BOYIA_LOG("ResourceHandle---run---url: %s", GET_STR(m_url));
        if (m_result == NetworkClient::kNetworkSuccess) {
            //BoyiaPtr<String> sptr = m_builder.toString();
            __android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "ResourceEvent::run kNetworkSuccess");
            __android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "Parse script=%s", (const char*)m_data->GetBuffer());
            m_loader->onLoadFinished(*m_data.get(), m_resType);
        } else {
            m_loader->onLoadError(m_result);
        }
    }

    virtual LVoid onFileLen(LInt len)
    {
        //m_loader->onFileLen(len);
    }

private:
    String m_url;
    ResourceLoader* m_loader;
    LInt m_resType;
    StringBuilder m_builder;
    LInt m_result;
    OwnerPtr<String> m_data;
};

ResourceLoader::ResourceLoader(ResourceLoaderClient* client)
    : m_render(NULL)
    , m_client(client)
    , m_cssSize(0)
{
}

ResourceLoader::~ResourceLoader()
{
    if (m_render) {
        delete m_render;
    }
}

LVoid ResourceLoader::onFileLen(LInt len)
{
    KLOG("ResourceLoader::onFileLen");
    KDESLOG(len);
    BOYIA_LOG("dataLen=%d", len);
}

LVoid ResourceLoader::onLoadError(LInt error)
{
}

LVoid ResourceLoader::onLoadFinished(const String& data, LInt resType)
{
    KLOG("ResourceLoader::onLoadFinished()");
    switch (resType) {
    case kHtmlDoc: {
        executeDocument(data);
    } break;
    case kCacheScript: {
        executeScript(data);
    } break;
    case kCacheStyleSheet: {
        executeStyleSheet(data);
    } break;
    default:
        break;
    }
}

LVoid ResourceLoader::load(const String& url, LoadType type)
{
    if (type == kCacheStyleSheet) {
        ++m_cssSize;
    }

    m_view->network()->loadUrl(url, new ResourceHandle(this, type, url));
}

LVoid ResourceLoader::setView(UIView* view)
{
    m_view = view;
}

UIView* ResourceLoader::view() const
{
    return m_view;
}

LVoid ResourceLoader::executeDocument(const String& data)
{
    KLOG("ResourceLoader::executeDocument()");
    KLOG((const char*)data.GetBuffer());
    //KSTRLOG8(m_data);
    if (!m_render) {
        m_render = new HtmlRenderer();
    }

    m_render->renderHTML(m_view->getDocument(),
        data,
        this);

    if (m_cssSize) {
        return;
    }

    m_render->layout();
    KLOG("m_render->layout()");

    if (m_client) {
        m_client->onHtmlRenderFinished();
    }

    KLOG("paint");
    m_render->paint(NULL);

    if (m_client) {
        KLOG("NULL != m_client");
        m_client->onViewNeedToDraw();
    }
}

HtmlRenderer* ResourceLoader::render() const
{
    return m_render;
}

// Execute external css
LVoid ResourceLoader::executeStyleSheet(const String& data)
{
    KFORMATLOG("ResourceLoader::executeCss data=%s", GET_STR(data));
    util::InputStream is(data);
    m_render->getStyleParser()->parseCss(is);

    if (--m_cssSize <= 0) {
        m_render->layout();
        m_render->paint(NULL);
    }
}

// Execute external script
LVoid ResourceLoader::executeScript(const String& data)
{
    CompileScript((char*)data.GetBuffer());
}

LVoid ResourceLoader::repaint(HtmlView* item)
{
    KLOG("ResourceLoader::repaint()");
    if (item) {
        m_render->paint(item);
        return;
    }

    m_render->paint(NULL);

    if (m_client) {
        m_client->onViewNeedToDraw();
    }
}

LVoid ResourceLoader::loadString(const String& src)
{
    onLoadFinished(src, kHtmlDoc);
}
}
