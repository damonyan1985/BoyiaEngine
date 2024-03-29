/*
 * UIView.cpp
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */

#include "UIView.h"
#include "AppManager.h"
#include "Application.h"
#include "HtmlView.h"

namespace yanbo {

UIView::UIView(Application* app)
    : m_app(app)
    , m_loader(kBoyiaNull)
    , m_doc(kBoyiaNull)
    , m_controller(kBoyiaNull)
    , m_operation(kBoyiaNull)
    , m_viewIdStart(0)
    , m_editor(kBoyiaNull)
{
    initComponents();
}

UIView::~UIView()
{
    if (m_loader) {
        delete m_loader;
    }

    if (m_doc) {
        delete m_doc;
    }

    if (m_controller) {
        delete m_controller;
    }

    if (m_operation) {
        delete m_operation;
    }
    
    if (m_editor) {
        delete m_editor;
    }
}

Application* UIView::application() const
{
    return m_app;
}

UIView* UIView::current()
{
    return AppManager::instance()->currentApp()->view();
}

const LRect& UIView::getClientRange() const
{
    return AppManager::instance()->getViewport();
}

void UIView::initComponents()
{
    if (!m_loader) {
        m_loader = new ResourceLoader(this);
        m_editor = Editor::get();
        m_doc = new HtmlDocument();
        m_doc->setView(this);
        m_controller = new UIViewController(this);
        m_operation = new UIOperation();
    }
}

LGraphicsContext* UIView::getGraphicsContext() const
{
    return AppManager::instance()->uiThread()->graphics();
}

void UIView::loadPage(const String& url)
{
    if (m_loader) {
        m_loader->load(url, ResourceLoader::kHtmlDoc);
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
    return AppManager::instance()->network();
}

void UIView::loadString(const String& src)
{
    m_loader->loadString(src);
}

bool UIView::canHit() const
{
    return getDocument() && getDocument()->getRenderTreeRoot();
}

UIOperation* UIView::operation() const
{
    return m_operation;
}

void UIView::handleFlingEvent(const LFlingEvent& evt)
{
    m_controller->onFling(evt.pt1, evt.pt2, evt.velocityX, evt.velocityY);
}

LInt UIView::incrementViewId()
{
    return ++m_viewIdStart;
}

Editor* UIView::getEditor() const
{
    return m_editor;
}
}
