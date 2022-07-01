/*
 * UIView.h
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */

#ifndef UIView_h
#define UIView_h

#include "HtmlDocument.h"
#include "LEvent.h"
#include "LGdi.h"
#include "ResourceLoader.h"
#include "UIOperation.h"
#include "UIViewController.h"

namespace yanbo {
class Application;
}

namespace yanbo {
class UIView {
public:
    static UIView* current();

    UIView(Application* app);
    virtual ~UIView();

public:
    const LRect& getClientRange() const;
    // setting network and graphic components int UIView
    void initComponents();

    void loadPage(const String& url);
    void loadString(const String& src);
    HtmlDocument* getDocument() const;
    ResourceLoader* getLoader() const;
    NetworkBase* network() const;
    LGraphicsContext* getGraphicsContext() const;
    Application* application() const;
    UIOperation* operation() const;

    void handleMouseEvent(const LMouseEvent& evt);
    void handleKeyEvent(const LKeyEvent& evt);
    void handleTouchEvent(const LTouchEvent& evt);
    void handleFlingEvent(const LFlingEvent& evt);

    bool canHit() const;
    LInt incrementViewId();
    
    Editor* getEditor() const;

private:
    ResourceLoader* m_loader;
    HtmlDocument* m_doc;
    LGraphicsContext* m_gc;
    LRect m_clientRect;
    UIViewController* m_controller;
    Application* m_app;
    UIOperation* m_operation;
    LInt m_viewIdStart;
    Editor* m_editor;
};
}
#endif
