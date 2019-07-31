#ifndef Tiny2Dom_h
#define Tiny2Dom_h

#include "CssParser.h"
#include "HtmlView.h"
#include "TinyXml2.h"

using namespace tinyxml2;

namespace yanbo {

class DOMBuilder {
public:
    DOMBuilder();
    ~DOMBuilder();

    DOMBuilder& add(HtmlDocument* doc);
    DOMBuilder& add(util::CssParser* cssParser);

    virtual LVoid build(const String& buffer);

private:
    HtmlView* createHtmlView(XMLNode* node, XMLNode* parentElem, HtmlView* parent);
    void createRenderTree(XMLNode* elem, XMLNode* parentElem, HtmlView* parent);

private:
    HtmlDocument* m_htmlDoc;
    util::CssParser* m_cssParser;
    XMLDocument* m_xmlDoc;
};
}

#endif