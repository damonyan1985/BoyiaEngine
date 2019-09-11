#ifndef VDOMBuilder_h
#define VDOMBuilder_h

//#include "CssParser.h"
//#include "HtmlView.h"
#include "JSONParser.h"
#include "JSONState.h"
#include "TinyXml2.h"
#include "VDocument.h"

using namespace tinyxml2;

namespace yanbo {

class VDOMBuilder {
public:
    VDOMBuilder();
    ~VDOMBuilder();

    VDOMBuilder& add(VDocument* dom);
    VDOMBuilder& add(const JSONState& state);
    virtual LVoid build(const String& buffer);

private:
    VNode* createVNode(XMLNode* node, XMLNode* parentNode, VNode* parent);
    VNode* createVDom(XMLNode* elem, XMLNode* parentNode, VNode* parent);

private:
    VDocument* m_vdom;
    //util::CssParser* m_cssParser;
    XMLDocument* m_xmlDoc;
};
}

#endif