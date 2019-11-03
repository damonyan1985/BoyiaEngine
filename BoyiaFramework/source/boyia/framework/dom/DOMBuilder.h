#ifndef Tiny2Dom_h
#define Tiny2Dom_h

#include "HtmlView.h"
#include "StyleParser.h"
#include "TinyXml2.h"

using namespace tinyxml2;

namespace yanbo {

class DOMBuilder {
public:
    DOMBuilder();
    ~DOMBuilder();

    DOMBuilder& with(HtmlDocument* doc);
    DOMBuilder& with(util::StyleParser* styleParser);

    virtual LVoid build(const String& buffer);

private:
    HtmlView* createHtmlView(XMLNode* node, XMLNode* parentElem, HtmlView* parent);
    void createRenderTree(XMLNode* elem, XMLNode* parentElem, HtmlView* parent);

private:
    HtmlDocument* m_htmlDoc;
    util::StyleParser* m_styleParser;
    XMLDocument* m_xmlDoc;
};
}

#endif
