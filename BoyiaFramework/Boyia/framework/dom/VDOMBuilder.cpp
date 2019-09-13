#include "VDOMBuilder.h"
#include "HtmlTags.h"
#include "OwnerPtr.h"
#include "VDocument.h"

namespace yanbo {
// Boyia引擎使代码变得简洁,
// 将使UIXML标签只保留几个属性, 保留属性为:
// id, class, boyia-loop, boyia-if
const char* kBoyiaLoopProperty = "boyia-loop";
const char* kBoyiaIfProperty = "boyia-if";

enum DOMValueType {
    kJsonValue,
    kIntValue
};

typedef struct DOMValue {
    LInt type;
    union Value {
        cJSON* json;
        LInt loopIndex;
    } value;
} DOMValue;

class LoopItemData {
public:
    LoopItemData() {}
    LoopItemData(
        const String& argItemName,
        const String& argIndexName,
        const cJSON* argData,
        LInt argLoopIndex)
    {
        itemName = argItemName;
        indexName = argIndexName;
        data = argData;
        loopIndex = argLoopIndex;
    }

    LoopItemData(LoopItemData& item)
    {
        itemName = item.itemName;
        indexName = item.indexName;

        data = item.data;
        loopIndex = item.loopIndex;
    }

    String itemName;
    String indexName;
    cJSON* data;
    LInt loopIndex;
}

VDOMBuilder::VDOMBuilder()
    : m_xmlDoc(NULL)
    , m_vdom(NULL)
    , m_state(NULL)
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

VDOMBuilder& VDOMBuilder::add(JSONState* state)
{
    m_state = state;
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
        Stack<String> stack;
        createVDom(root, stack);
    }
}

// for special attribute
LVoid VDOMBuilder::findNodeAttribute(XMLNode* elem, VNode* parent, Stack<LoopItemData>& stack)
{
    if (node->ToText() == NULL && node->ToComment() == NULL) {
        XMLElement* elem = node->ToElement();
        if (!elem) {
            return;
        }

        String loopValue = _CS(elem->Attribute(kBoyiaLoopProperty));
        // such as <div boyia-loop="{items:item:index}">, index is option
        // loopValue length must be larger then 2.
        if (loopValue.GetLength() >= 2) {
            OwnerPtr<KVector<String>> args = StringUtils::split(loopValue, _CS(":"));
            DOMValue outValue;
            fetchValue(args->elementAt(0), stack, outValue);
            //cJSON* list = m_state->get(GET_STR(args->elementAt(0)));
            cJSON* list = outValue.value.json;
            if (list->type == cJSON_Array) {
                cJSON* nodeData = list->child;
                LInt index = 0;
                while (nodeData) {
                    LoopItemData itemData(
                        nodeData,
                        index,
                        args->size() >= 3 ? args->elementAt(2) : _CS(""),
                        args->elementAt(1));

                    stack.push(itemData);
                    static_cast<VElement*>(parent)->add(createVDom(elem, stack, args->elementAt(1)));
                    nodeData = nodeData->next;
                    index++;
                }
            }

            return;
        }

        String ifValue = _CS(elem->Attribute(kBoyiaIfProperty));
        if (ifValue.GetLength()) {
            m_state->get(ifValue);
        }
    }
}

VNode* VDOMBuilder::createVDom(XMLNode* elem, Stack<LoopItemData>& stack)
{
    VNode* item = createVNode(elem, stack);
    for (XMLNode* child = elem->FirstChild(); child; child = child->NextSibling()) {
        findNodeAttribute(child, item);
    }

    return item;
}

LVoid VDOMBuilder::fetchValue(const String& value, Stack<LoopItemData>& stack, DOMValue& outValue)
{
    LInt begin = value.Find(_CS("{"));
    LInt end = value.Find(_CS("}"));

    // if the value is a variable
    if (begin > 0 && end > begin) {
        String strVal = value.Mid(begin, end - begin);
        LInt pos = strVal.Find(_CS("."));

        if (pos < 0) {
            // value is only support string
            // because ui display is also string
            for (LInt i = stack.size() - 1; i >= 0; ++i) {
                if (stack.elementAt(i).itemName.CompareCase(strVal)) {
                    outValue.value.json = stack.elementAt(i).data;
                    outValue.type = kJsonValue;
                    break;
                }

                if (stack.elementAt(i).indexName.CompareCase(strVal)) {
                    outValue.value.loopIndex = stack.elementAt(i).loopIndex;
                    outValue.type = kIntValue;
                    break;
                }
            }
        } else {
            // such as obj.name.firstName
            OwnerPtr<KVector<String>> values = StringUtils::split(_CS("."));
            JSONObject data;
            // if value is in stack
            for (LInt j = stack.size() - 1; j >= 0; ++j) {
                if (stack.elementAt(j).itemName.CompareCase(values->elementAt(0))) {
                    data = stack.elementAt(j).data;
                    break;
                }
            }

            // if value is not in stack, now we may find value in m_state
            if (!data.ptr()) {
                data = m_state->get(values->elementAt(0));
            }

            // such as obj.props1.props2...
            for (LInt i = 1; i < values->size(); ++i) {
                data = data.get(values->elementAt(i));
            }

            outValue.value.json = data;
            outValue.type = kJsonValue;
        }
    }
}

VNode* VDOMBuilder::createVNode(XMLNode* node, LVoid* data, Stack<LoopItemData>& stack)
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

        // LInt begin = className.Find("{");
        // LInt end = className.Find("{");
        // // 如果存在类似{obj.name}
        // if (begin > 0 && end > begin) {
        //     //static_cast<cJSON*>(data)->
        //     String value = className.Mid(begin, end - begin);
        //     KVector<String> values = StringUtils::split(value, _CS("."));
        //     m_state->get(GET_STR(value));
        // }

        LInt tag = htmlTags->symbolAsInt(tagVal);

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
