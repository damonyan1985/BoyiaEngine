/*
 * UIView.cpp
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */

#include "UIView.h"
#include "AppManager.h"
#include "HtmlView.h"

namespace yanbo {

UIView* UIView::s_instance = NULL;
UIView::UIView()
    : m_loader(NULL)
    , m_doc(NULL)
    , m_controller(NULL)
    , m_network(NULL)
    , m_jsHandler(NULL)
{
    setComponents();
}

UIView::~UIView()
{
    if (m_loader) {
        delete m_loader;
    }

    if (m_doc) {
        delete m_doc;
    }

    if (m_network) {
        delete m_network;
    }

    if (m_jsHandler) {
        delete m_jsHandler;
    }
}

UIView* UIView::getInstance()
{
    return AppManager::instance()->currentApp()->view();
}

const LRect& UIView::getClientRange() const
{
    return AppManager::instance()->getViewport();
}

void UIView::setComponents()
{
    if (!m_loader) {
        m_loader = new ResourceLoader(NULL);
        m_network = NetworkBase::create();

        m_doc = new HtmlDocument();
        m_loader->setView(this);
        m_doc->setView(this);
        m_jsHandler = new boyia::BoyiaEventHandler();

        m_controller = new UIViewController(this);
    }
}

LGraphicsContext* UIView::getGraphicsContext() const
{
    return AppManager::instance()->uiThread()->graphics();
}

void UIView::loadPage(const String& url)
{
    if (m_loader) {
        m_loader->load(url, ResourceLoader::HTMLDOC);
    }
}

HtmlDocument* UIView::getDocument() const
{
    return m_doc;
}

void UIView::handleMouseEvent(const util::LMouseEvent& evt)
{
    switch (evt.getType()) {
    case util::LMouseEvent::MOUSE_PRESS: {
        m_controller->onMousePress(evt.getPosition());
    } break;
    case util::LMouseEvent::MOUSE_UP: {
        m_controller->onMouseUp(evt.getPosition());
    } break;
    case util::LMouseEvent::MOUSE_LONG_PRESS: {
    } break;
    case util::LMouseEvent::MOUSE_SCROLL: {
    } break;
    }
}

void UIView::handleKeyEvent(const util::LKeyEvent& evt)
{
    switch (evt.getType()) {
    case util::LKeyEvent::KEY_ARROW_DOWN: {
        m_controller->onDownKey(LFalse);
    } break;
    case util::LKeyEvent::KEY_ARROW_UP: {
        m_controller->onUpKey(LFalse);
    } break;
    case util::LKeyEvent::KEY_ENTER: {
        m_controller->onEnterKey(LFalse);
    } break;
    }
}

void UIView::handleTouchEvent(const util::LTouchEvent& evt)
{
    //	m_jsHandlerm_jsHandler->handleTouch(evt);

    switch (evt.getType()) {
    case LTouchEvent::ETOUCH_DOWN:
        m_controller->onTouchDown(evt.getPosition());
        break;
    case LTouchEvent::ETOUCH_MOVE:
        m_controller->onTouchMove(evt.getPosition());
        break;
    case LTouchEvent::ETOUCH_UP:
        m_controller->onTouchUp(evt.getPosition());
        break;
    }
}

ResourceLoader* UIView::getLoader() const
{
    return m_loader;
}

NetworkBase* UIView::network() const
{
    return m_network;
}

void UIView::loadString(const String& src)
{
    m_loader->loadString(src);
}

boyia::BoyiaEventHandler* UIView::jsHandler() const
{
    return m_jsHandler;
}

bool UIView::canHit() const
{
    return getDocument() && getDocument()->getRenderTreeRoot();
}
}
