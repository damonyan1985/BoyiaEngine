/*
 * HtmlDocument.cpp
 *
 *  Created on: 2011-7-9
 *      Author: yanbo
 */
#include "HtmlDocument.h"
#include "StringUtils.h"
#include "UIView.h"

namespace yanbo
{

HtmlDocument::HtmlDocument()
    : m_title(_CS("yanbo"))
    , m_url(_CS(""))
    , m_root(NULL)
    , m_view(NULL)
    , m_idMap(20)
{
}

HtmlDocument::~HtmlDocument()
{	
}

void HtmlDocument::setRenderTreeRoot(HtmlView* root)
{
    m_root = root;
}

HtmlView* HtmlDocument::getRenderTreeRoot() const
{
	return m_root;
}

void HtmlDocument::setPageTitle(const String& titleText)
{
	m_title = titleText;
}

const String& HtmlDocument::getPageTitle() const
{
	return m_title;
}

void HtmlDocument::setPageUrl(const String& url)
{
    m_url = url;	
}

const String& HtmlDocument::getPageUrl() const
{
	return m_url;
}

// use linear structure to deal with the up, down event draw can enhance efficiency
HtmlView* HtmlDocument::getPreItem()
{
	HtmlViewList::Iterator iter = m_currentItemIter;
	
	if (--m_currentItemIter == m_itemList.end())
	{
	    m_currentItemIter = iter;
	}
	
	return *m_currentItemIter;
}

HtmlView* HtmlDocument::getNextItem()
{
	HtmlViewList::Iterator iter = m_currentItemIter;
	
	if (++m_currentItemIter == m_itemList.end())
	{
	    m_currentItemIter = iter;
	}
	
	return *m_currentItemIter;
}

void HtmlDocument::addForm(HtmlForm* form)
{
	m_formList.push(form);
}

HtmlView* HtmlDocument::getCurrentItem()
{
    return *m_currentItemIter;
}

void HtmlDocument::addHtmlView(HtmlView* item)
{
	m_itemList.push(item);
}

void HtmlDocument::resetHtmlFocus()
{
	m_currentItemIter = m_itemList.begin();
}
void HtmlDocument::clearHtmlList()
{
	m_itemList.clear();
}

void HtmlDocument::setViewPort(const LayoutRect& viewRect)
{
	m_viewRect = viewRect;
}

const LayoutRect& HtmlDocument::getViewPort() const
{
	return m_viewRect;
}

void HtmlDocument::putItemID(String& id, HtmlView* item)
{
	m_idMap.put(id, (LInt) item);
}

HtmlView* HtmlDocument::getItemByID(const String& id)
{
	return (HtmlView*) m_idMap.get(id);
}

void HtmlDocument::setView(UIView* view)
{
    m_view = view;
}

UIView* HtmlDocument::getView() const
{
    return m_view;
}

void HtmlDocument::sortIds()
{
	m_idMap.sort();
}

}

