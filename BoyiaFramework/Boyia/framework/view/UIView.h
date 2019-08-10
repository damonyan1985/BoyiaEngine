/*
 * UIView.h
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */

#ifndef UIView_h
#define UIView_h

#include "BoyiaEventHandler.h"
#include "HtmlDocument.h"
#include "LEvent.h"
#include "LGdi.h"
#include "ResourceLoader.h"
#include "ResourceLoaderClient.h"
#include "UIViewController.h"

namespace yanbo {
class UIView {
public:
    static UIView* getInstance();
    void destroy();

    UIView();
    virtual ~UIView();

public:
    const LRect& getClientRange() const;
    // setting network and graphic components int UIView
    void setComponents(NetworkBase* network, ResourceLoaderClient* client);

    void loadPage(const String& url);
    void loadString(const String& src);
    HtmlDocument* getDocument() const;
    ResourceLoader* getLoader() const;
    NetworkBase* network() const;
    boyia::BoyiaEventHandler* jsHandler() const;
    LGraphicsContext* getGraphicsContext() const;

    void handleMouseEvent(const LMouseEvent& evt);
    void handleKeyEvent(const LKeyEvent& evt);
    void handleTouchEvent(const LTouchEvent& evt);

    bool canHit() const;

private:
    ResourceLoader* m_loader;
    HtmlDocument* m_doc;
    LGraphicsContext* m_gc;
    NetworkBase* m_network;
    LRect m_clientRect;
    UIViewController* m_controller;
    boyia::BoyiaEventHandler* m_jsHandler;

    static UIView* s_instance;
};
}
#endif
