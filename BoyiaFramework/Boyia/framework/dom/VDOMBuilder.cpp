#include "VDOMBuilder.h"
#include "HtmlTags.h"
#include "VDocument.h"

namespace yanbo {

VDOMBuilder::VDOMBuilder()
    : m_xmlDoc(NULL)
    , m_vdom(NULL)
{
}

VDOMBuilder::~VDOMBuilder()
{
    if (m_xmlDoc) {
        delete m_xmlDoc;
    }
}

VDOMBuilder& VDOMBuilder::add(VDocument* doc)
{
    m_vdom = doc;
    return *this;
}

VDOMBuilder& VDOMBuilder::add(const JSONState& state)
{
    return *this;
}

LVoid VDOMBuilder::build(const String& buffer)
{
    if (!m_xmlDoc) {
        m_xmlDoc = new XMLDocument;
        m_xmlDoc->Parse((const LCharA*)buffer.GetBuffer());
    }

    XMLElement* root = m_xmlDoc->RootElement();
    if (root) {
        createVDom(root, NULL, NULL);
    }

    //m_htmlDoc->sortIds();
}

VNode* VDOMBuilder::createVDom(XMLNode* elem, XMLNode* parentElem, VNode* parent)
{
    //KLOG("createRenderTree");
    VNode* item = createVNode(elem, parentElem, parent);

    for (XMLNode* child = elem->FirstChild(); child; child = child->NextSibling()) {
        static_cast<VElement*>(item)->add(createVDom(child, elem, item));
    }

    return item;
}

VNode* VDOMBuilder::createVNode(XMLNode* node, XMLNode* parentElem, VNode* parent)
{
    VNode* item = NULL;
    HtmlTags* htmlTags = HtmlTags::getInstance();

    if (node->ToText() == NULL && node->ToComment() == NULL) {
        XMLElement* elem = node->ToElement();
        if (!elem) {
            return NULL;
        }

        String tagVal = _CS(elem->Value());
        String id = _CS(elem->Attribute("id"));
        //String tagName = _CS(elem->Attribute("name"));
        String className = _CS(elem->Attribute("class"));

        int tag = htmlTags->symbolAsInt(tagVal);

        VElement* vElem = new VElement();
        vElem->setId(id);
        vElem->setClass(className);
        vElem->setTag(tag);

        item = vElem;
    } else if (node->ToText()) {
        XMLText* elem = node->ToText();
        VText* vText = new VText();
        vText->setText(_CS(elem->Value()));
        item = vText;
    }

    return item;
}
}
