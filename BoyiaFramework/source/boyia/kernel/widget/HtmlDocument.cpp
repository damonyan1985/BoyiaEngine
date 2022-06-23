/*
 * HtmlDocument.cpp
 *
 *  Created on: 2011-7-9
 *      Author: yanbo
 */
#include "HtmlDocument.h"
#include "StringUtils.h"
#include "UIView.h"
#include "InputView.h"

namespace yanbo {

HtmlDocument::HtmlDocument()
    : m_title(_CS("boyia_ui"))
    , m_url(_CS(""))
    , m_root(kBoyiaNull)
    , m_view(kBoyiaNull)
    , m_focus(kBoyiaNull)
{
}

HtmlDocument::~HtmlDocument()
{
}

LVoid HtmlDocument::setRenderTreeRoot(HtmlView* root)
{
    m_root = root;
}

HtmlView* HtmlDocument::getRenderTreeRoot() const
{
    return m_root;
}

LVoid HtmlDocument::setPageTitle(const String& titleText)
{
    m_title = titleText;
}

const String& HtmlDocument::getPageTitle() const
{
    return m_title;
}

LVoid HtmlDocument::setPageUrl(const String& url)
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
    WeakHtmlViewList::Iterator iter = m_currentItemIter;

    if (--m_currentItemIter == m_itemList.end()) {
        m_currentItemIter = iter;
    }

    return *m_currentItemIter;
}

HtmlView* HtmlDocument::getNextItem()
{
    WeakHtmlViewList::Iterator iter = m_currentItemIter;

    if (++m_currentItemIter == m_itemList.end()) {
        m_currentItemIter = iter;
    }

    return *m_currentItemIter;
}

HtmlView* HtmlDocument::getCurrentItem()
{
    return *m_currentItemIter;
}

LVoid HtmlDocument::addHtmlView(HtmlView* item)
{
    m_itemList.push(item);
}

LVoid HtmlDocument::resetHtmlFocus()
{
    m_currentItemIter = m_itemList.begin();
}

LVoid HtmlDocument::clearHtmlList()
{
    m_itemList.clear();
}

const LayoutRect& HtmlDocument::getViewPort() const
{
    return m_view->getClientRange();
}

LVoid HtmlDocument::putItemID(String& id, HtmlView* item)
{
    m_idMap.put(id, item);
}

HtmlView* HtmlDocument::getItemByID(const String& id)
{
    return (HtmlView*)m_idMap.get(HashString(id, LFalse));
}

LVoid HtmlDocument::setView(UIView* view)
{
    m_view = view;
}

UIView* HtmlDocument::getView() const
{
    return m_view;
}

LVoid HtmlDocument::setFocusImpl(LBool focus, const LPoint& point)
{
    if (m_focus->isEditor()) {
        static_cast<InputView*>(m_focus.get())->setSelectedWithPosition(focus, point);
    } else {
        m_focus->setSelected(focus);
    }
}

LVoid HtmlDocument::setFocusView(HtmlView* view, const LPoint& point)
{   
    if (!view) {
        return;
    }

    if (!view->isSelectable()) {
        return;
    }

    if (m_focus && m_focus != view) {
        setFocusImpl(LFalse, point);
    }
    
    m_focus = view;
    setFocusImpl(LTrue, point);
}
}
