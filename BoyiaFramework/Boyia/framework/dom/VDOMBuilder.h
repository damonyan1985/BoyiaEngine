#ifndef VDOMBuilder_h
#define VDOMBuilder_h

#include "JSONObject.h"
#include "JSONParser.h"
#include "Stack.h"
#include "TinyXml2.h"
#include "VDocument.h"

using namespace tinyxml2;

namespace yanbo {

struct DOMValue;
class LoopItemData;
class VDOMBuilder {
public:
    VDOMBuilder();
    ~VDOMBuilder();

    VDOMBuilder& add(VDocument* dom);
    VDOMBuilder& add(JSONObject* state);
    virtual LVoid build(const String& buffer);

private:
    VNode* createVNode(XMLNode* node, Stack<LoopItemData>& stack);
    VNode* createVDom(XMLNode* elem, Stack<LoopItemData>& stack);
    LVoid findNodeAttribute(XMLNode* elem, VNode* parent, Stack<LoopItemData>& stack);
    LVoid fetchValue(const String& value, Stack<LoopItemData>& stack, DOMValue& outValue);

private:
    VDocument* m_vdom;
    JSONObject* m_state;
    XMLDocument* m_xmlDoc;
};
}

#endif