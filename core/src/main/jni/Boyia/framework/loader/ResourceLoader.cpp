/*
 * ResourceLoader.cpp
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */
#include "ResourceLoader.h"
#include "BoyiaThread.h"
#include "SalLog.h"
#include "StringBuilder.h"
#include "StringUtils.h"
#include "UIView.h"
#include <android/log.h>

extern LVoid CompileScript(char* code);
namespace yanbo {

class ResourceHandle : public NetworkClient, public BoyiaEvent {
public:
    ResourceHandle(ResourceLoader* loader, LInt type)
        : m_loader(loader)
        , m_result(NetworkClient::NETWORK_SUCCESS)
        , m_resType(type)
    {
    }

    virtual void onDataReceived(const LByte* data, LInt size)
    {
        m_builder.append(data, 0, size, LFalse);
    }

    virtual void onStatusCode(LInt statusCode)
    {
    }

    virtual void onRedirectUrl(const String& redirectUrl)
    {
    }

    virtual void onLoadError(LInt error)
    {
        //m_loader->onLoadError(error);
        m_result = error;
        BoyiaThread::instance()->sendEvent(this);
    }

    virtual void onLoadFinished()
    {
        m_data = m_builder.toString();
        BoyiaThread::instance()->sendEvent(this);
    }

    virtual LVoid run()
    {
        __android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "ResourceEvent::run");
        if (m_result == NetworkClient::NETWORK_SUCCESS) {
            //BoyiaPtr<String> sptr = m_builder.toString();
            __android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "ResourceEvent::run NETWORK_SUCCESS");
            __android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "Parse script=%s", (const char*)m_data->GetBuffer());
            m_loader->onLoadFinished(*m_data.get(), m_resType);
        } else {
            m_loader->onLoadError(m_result);
        }
    }

    virtual void onFileLen(LInt len)
    {
        //m_loader->onFileLen(len);
    }

private:
    ResourceLoader* m_loader;
    LInt m_resType;
    StringBuilder m_builder;
    LInt m_result;
    BoyiaPtr<String> m_data;
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
        m_render = NULL;
    }
}

void ResourceLoader::onFileLen(LInt len)
{
    KLOG("ResourceLoader::onFileLen");
    KDESLOG(len);
    YanLog("dataLen=%d", len);
}

void ResourceLoader::onLoadError(LInt error)
{
}

void ResourceLoader::onLoadFinished(const String& data, LInt resType)
{
    KLOG("ResourceLoader::onLoadFinished()");
    switch (resType) {
    case HTMLDOC: {
        executeDocument(data);
    } break;
    case CACHEJS: {
        executeScript(data);
    } break;
    case CACHECSS: {
        executeCss(data);
    } break;
    default:
        break;
    }
}

void ResourceLoader::load(const String& url, LoadType type)
{
    if (type == CACHECSS) {
        ++m_cssSize;
    }
    ResourceHandle* handle = new ResourceHandle(this, type);
    m_view->network()->loadUrl(url, handle);
}

void ResourceLoader::setView(UIView* view)
{
    m_view = view;
}

UIView* ResourceLoader::view() const
{
    return m_view;
}

void ResourceLoader::executeDocument(const String& data)
{
    KLOG("ResourceLoader::executeDocument()");
    KLOG((const char*)data.GetBuffer());
    //KSTRLOG8(m_data);
    if (!m_render) {
        m_render = new HtmlRenderer();
    }

    m_render->renderHTML(m_view->getDocument(),
        data,
        m_view->getClientRange(),
        this);

    if (m_cssSize) {
        return;
    }

    m_render->layout();
    KLOG("m_render->layout()");

    if (m_client) {
        m_client->onHtmlRenderFinished();
    }

    LGraphicsContext* gc = m_view->getGraphicsContext();
    if (!gc) {
        return;
    }
    gc->reset();

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
void ResourceLoader::executeCss(const String& data)
{
    KFORMATLOG("ResourceLoader::executeCss data=%s", GET_STR(data));
    util::InputStream is(data);
    m_render->getCssParser()->parseCss(is);

    if (--m_cssSize <= 0) {
        m_render->layout();
        m_render->paint(NULL);
    }
}

// Execute external script
void ResourceLoader::executeScript(const String& data)
{
    CompileScript((char*)data.GetBuffer());
}

void ResourceLoader::repaint(HtmlView* item)
{
    KLOG("ResourceLoader::repaint()");
    LGraphicsContext* gc = m_view->getGraphicsContext();
    if (!gc) {
        return;
    }

    if (item) {
        m_render->paint(item);
        return;
    }

    gc->reset();
    m_render->paint(NULL);

    if (m_client) {
        m_client->onViewNeedToDraw();
    }
}

void ResourceLoader::loadString(const String& src)
{
    onLoadFinished(src, HTMLDOC);
}
}
