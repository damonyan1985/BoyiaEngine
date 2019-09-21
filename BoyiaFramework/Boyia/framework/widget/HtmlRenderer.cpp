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
    : m_styleParser(NULL)
    , m_form(NULL)
    , m_htmlDoc(NULL)
{
}

HtmlRenderer::~HtmlRenderer()
{
    if (m_styleParser) {
        delete m_styleParser;
    }
}

void HtmlRenderer::renderHTML(HtmlDocument* doc,
    const String& buffer,
    ResourceLoader* loader)
{
    //UIThread::instance()->setGC(loader->view()->getGraphicsContext());

    m_htmlDoc = doc;
    m_htmlDoc->clearHtmlList();
    m_loader = loader;

    if (NULL == m_styleParser) {
        m_styleParser = new util::StyleParser();
    }

    DOMBuilder dom;
    dom.add(m_htmlDoc)
        .add(m_styleParser)
        .build(buffer);
    //dom.createDocument(buffer, m_htmlDoc, m_styleParser);
    m_htmlDoc->resetHtmlFocus();
}

util::StyleManager* HtmlRenderer::getStyleManager() const
{
    return m_styleParser->getStyleManager();
}

util::StyleParser* HtmlRenderer::getStyleParser() const
{
    return m_styleParser;
}

void HtmlRenderer::layout()
{
    util::StyleManager* StyleManager = m_styleParser->getStyleManager();
    if (StyleManager) {
        m_htmlDoc->getRenderTreeRoot()->setStyle(StyleManager, NULL);
    }

    m_htmlDoc->getRenderTreeRoot()->layout();
}

void HtmlRenderer::paint(HtmlView* item)
{
    UIThread::instance()->draw(item ? item : m_htmlDoc->getRenderTreeRoot());
}
}
