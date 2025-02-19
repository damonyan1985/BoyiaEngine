/*
 * ResourceLoader.cpp
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */
#include "ResourceLoader.h"
#include "AppManager.h"
#include "Application.h"
#include "FileUtil.h"
#include "PlatformBridge.h"
#include "SalLog.h"
#include "StringBuilder.h"
#include "StringUtils.h"
#include "UIThread.h"
#include "UIView.h"
//#include <android/log.h>

namespace yanbo {

class ResourceHandle LFinal : public NetworkClient, public UIEvent {
public:
    ResourceHandle(ResourceLoader* loader, LInt type, const String& url)
        : m_loader(loader)
        , m_result(NetworkClient::kNetworkSuccess)
        , m_resType(type)
        , m_url(url)
    {
    }

    virtual LVoid onDataReceived(const LByte* data, LInt size) LOverride
    {
        LByte* destData = new LByte[size];
        util::LMemcpy(destData, data, size);
        m_builder.append(destData, 0, size, LFalse);
    }

    virtual LVoid onStatusCode(LInt statusCode) LOverride
    {
    }

    virtual LVoid onRedirectUrl(const String& redirectUrl) LOverride
    {
    }

    virtual LVoid onLoadError(LInt error) LOverride
    {
        //m_loader->onLoadError(error);
        m_result = error;
        UIThread::instance()->sendUIEvent(this);
    }

    virtual LVoid onLoadFinished() LOverride
    {
        UIThread::instance()->sendUIEvent(this);
    }

    // Run In UIThread
    virtual LVoid run() LOverride
    {
        KLOG("ResourceEvent::run");

        BOYIA_LOG("ResourceHandle---run---url: %s", GET_STR(m_url));
        if (m_result == NetworkClient::kNetworkSuccess) {
            KLOG("ResourceEvent::run kNetworkSuccess");
            OwnerPtr<String> data = m_builder.toString();
            BOYIA_LOG("Parse script=%s", (const char*)data->GetBuffer());
            m_loader->onLoadFinished(*data.get(), m_resType);
        } else {
            m_loader->onLoadError(m_result);
        }
    }

    virtual LVoid onFileLen(LInt len) LOverride
    {
        //m_loader->onFileLen(len);
    }

private:
    String m_url;
    ResourceLoader* m_loader;
    LInt m_resType;
    StringBuilder m_builder;
    LInt m_result;
};

ResourceLoader::ResourceLoader(UIView* view)
    : m_render(kBoyiaNull)
    , m_view(view)
    , m_cssSize(0)
{
}

ResourceLoader::~ResourceLoader()
{
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
    if (type == kCacheScript && FileUtil::isExist(PlatformBridge::getInstructionEntryPath())) {
        return;
    }

    if (type == kCacheStyleSheet) {
        ++m_cssSize;
    }

    m_view->network()->loadUrl(url, new ResourceHandle(this, type, url));
}

UIView* ResourceLoader::view() const
{
    return m_view;
}

LVoid ResourceLoader::startupPage()
{
    m_render->layout();
    m_render->paint(kBoyiaNull);

    if (m_view->application()->runtime()->isLoadExeFile()) {
        m_view->application()->runtime()->runExeFile();
    }
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

    KLOG("m_render->layout()");
    startupPage();
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
        startupPage();
    }
}

// Execute external script
LVoid ResourceLoader::executeScript(const String& data)
{
    m_view->application()->runtime()->compile(data);
    //SystemGC(m_view->application()->runtime()->vm());
}

LVoid ResourceLoader::repaint(HtmlView* item)
{
    KLOG("ResourceLoader::repaint()");
    if (item) {
        m_render->paint(item);
        return;
    }

    m_render->paint(kBoyiaNull);
}

LVoid ResourceLoader::loadString(const String& src)
{
    onLoadFinished(src, kHtmlDoc);
}
}
