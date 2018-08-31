/*
 * HtmlRenderer.cpp
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#include "HtmlRenderer.h"
#include "ResourceLoader.h"
#include "RenderContext.h"
#include "SalLog.h"
#include "UIView.h"

namespace yanbo
{

HtmlRenderer::HtmlRenderer()
    : m_cssParser(NULL)
    , m_form(NULL)
    , m_htmlDoc(NULL)
    , m_document(NULL)
{	
}

HtmlRenderer::~HtmlRenderer()
{
	if (NULL != m_document)
	{
		delete m_document;
		m_document = NULL;
	}

	if (NULL != m_cssParser)
	{
	    delete m_cssParser;
	    m_cssParser = NULL;
	}
}

void HtmlRenderer::renderHTML(HtmlDocument* doc,
		const String& buffer,
		const LayoutRect& rect,
		ResourceLoader* loader)
{
	PaintThread::instance()->setGC(loader->view()->getGraphicsContext());
	//PaintThread::getInst()->init();

	m_htmlDoc = doc;
	m_htmlDoc->clearHtmlList();
	m_loader = loader;
	m_rect = rect;

	if (NULL == m_cssParser)
	{
	    m_cssParser = new util::CssParser();
	}

	m_document = Document::create(Document::ETiny2Doc);
	m_document->createDocument(buffer, m_htmlDoc, m_cssParser);

    delete m_document;
    m_document = NULL;
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
	if (NULL != cssManager)
	{
	    m_htmlDoc->getRenderTreeRoot()->setStyle(cssManager, NULL);
	}

	m_htmlDoc->getRenderTreeRoot()->layout();
}

void HtmlRenderer::paint(HtmlView* item)
{
	PaintThread::instance()->draw(item ? item : m_htmlDoc->getRenderTreeRoot());
//	LGraphicsContext* gc = m_loader->view()->getGraphicsContext();
//	if (item)
//	{
//		item->paint(*gc);
//	}
//	else
//	{
//		m_htmlDoc->getRenderTreeRoot()->paint(*gc);
//	}
//
//	PaintThread::getInst()->submit();
}

}
