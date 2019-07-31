/*
 * HtmlRenderer.cpp
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#include "HtmlRenderer.h"
#include "DOMBuilder.h"
#include "RenderContext.h"
#include "ResourceLoader.h"
#include "SalLog.h"
#include "UIThread.h"
#include "UIView.h"

namespace yanbo {

HtmlRenderer::HtmlRenderer()
    : m_cssParser(NULL)
    , m_form(NULL)
    , m_htmlDoc(NULL)
{
}

HtmlRenderer::~HtmlRenderer()
{
    if (NULL != m_cssParser) {
        delete m_cssParser;
        m_cssParser = NULL;
    }
}

void HtmlRenderer::renderHTML(HtmlDocument* doc,
    const String& buffer,
    const LayoutRect& rect,
    ResourceLoader* loader)
{
    UIThread::instance()->setGC(loader->view()->getGraphicsContext());

    m_htmlDoc = doc;
    m_htmlDoc->clearHtmlList();
    m_loader = loader;
    m_rect = rect;

    if (NULL == m_cssParser) {
        m_cssParser = new util::CssParser();
    }

    DOMBuilder dom;
    dom.add(m_htmlDoc)
        .add(m_cssParser)
        .build(buffer);
    //dom.createDocument(buffer, m_htmlDoc, m_cssParser);
    m_htmlDoc->resetHtmlFocus();
}

util::CssManager* HtmlRenderer::getCssManager() const
{
    return m_cssParser->getCssManager();
}

util::CssParser* HtmlRenderer::getCssParser() const
{
    return m_cssParser;
}

void HtmlRenderer::layout()
{
    m_htmlDoc->setViewPort(m_rect);
    util::CssManager* cssManager = m_cssParser->getCssManager();
    if (NULL != cssManager) {
        m_htmlDoc->getRenderTreeRoot()->setStyle(cssManager, NULL);
    }

    m_htmlDoc->getRenderTreeRoot()->layout();
}

void HtmlRenderer::paint(HtmlView* item)
{
    UIThread::instance()->draw(item ? item : m_htmlDoc->getRenderTreeRoot());
}
}