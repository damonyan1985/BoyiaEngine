#include "VDOMBuilder.h"
#include "HtmlTags.h"
#include "OwnerPtr.h"
#include "StringBuilder.h"
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

typedef union DOMValueImpl {
    cJSON* json;
    LInt loopIndex;
} DOMValueImpl;

class DOMValue {
public:
    DOMValue()
    {
        value.json = NULL;
    }
    LInt type;
    DOMValueImpl value;
};

class LoopItemData {
public:
    LoopItemData() {}
    LoopItemData(
        const String& argItemName,
        const String& argIndexName,
        cJSON* argData,
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
};

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

VDOMBuilder& VDOMBuilder::add(JSONObject* state)
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
        Stack<LoopItemData> stack;
        createVDom(root, stack);
    }
}

// for special attribute
LBool VDOMBuilder::findSpecialAttribute(XMLNode* node, VNode* parent, Stack<LoopItemData>& stack)
{
    if (node->ToText() == NULL && node->ToComment() == NULL) {
        XMLElement* elem = node->ToElement();
        if (!elem) {
            return LTrue;
        }

        DOMValue outValue;
        // 优先判断if是否为真，否则一切都别整了
        String ifValue = _CS(elem->Attribute(kBoyiaIfProperty));
        if (ifValue.GetLength()) {
            fetchValue(ifValue, stack, outValue);
            if (!outValue.value.json->valueint) {
                return LTrue;
            }
        }

        String loopValue = _CS(elem->Attribute(kBoyiaLoopProperty));
        // such as <div boyia-loop="{items:item:index}">, index is option
        // loopValue length must be larger then 2.
        if (loopValue.GetLength() >= 2) {
            OwnerPtr<KVector<String>> args = StringUtils::split(loopValue, _CS(":"));

            fetchValue(args->elementAt(0), stack, outValue);
            //cJSON* list = m_state->get(GET_STR(args->elementAt(0)));
            cJSON* list = outValue.value.json;
            if (list->type == cJSON_Array) {
                cJSON* nodeData = list->child;
                LInt index = 0;
                while (nodeData) {
                    LoopItemData itemData(
                        args->elementAt(1),
                        args->size() >= 3 ? args->elementAt(2) : String(_CS(NULL)),
                        nodeData,
                        index);

                    stack.push(itemData);
                    static_cast<VElement*>(parent)->add(createVDom(elem, stack));
                    nodeData = nodeData->next;
                    index++;
                    stack.pop();
                }
            }

            return LTrue;
        }
    }

    return LFalse;
}

VNode* VDOMBuilder::createVDom(XMLNode* elem, Stack<LoopItemData>& stack)
{
    // root has no special attributes
    VNode* item = createVNode(elem, stack);
    for (XMLNode* child = elem->FirstChild(); child; child = child->NextSibling()) {
        if (!findSpecialAttribute(child, item, stack) && item->isElement()) {
            static_cast<VElement*>(item)->add(createVDom(child, stack));
        }
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
            OwnerPtr<KVector<String>> values = StringUtils::split(value, _CS("."));
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

            outValue.value.json = data.ptr();
            outValue.type = kJsonValue;
        }
    }
}

LVoid VDOMBuilder::fetchTextValue(const String& value, Stack<LoopItemData>& stack, String& outValue)
{
    OwnerPtr<String> val = new String(value);
    do {
        LInt begin = val->Find(_CS("{"));
        LInt end = val->Find(_CS("}"));

        StringBuilder builder;
        builder.append(val->Mid(0, begin));

        // if the value is a variable
        if (begin > 0 && end > begin) {
            String strVal = val->Mid(begin, end - begin);
            LInt pos = strVal.Find(_CS("."));

            if (pos < 0) {
                // value is only support string
                // because ui display is also string
                for (LInt i = stack.size() - 1; i >= 0; ++i) {
                    if (stack.elementAt(i).itemName.CompareCase(strVal)) {
                        //outValue.value.json = stack.elementAt(i).data;
                        //outValue.type = kJsonValue;
                        builder.append(_CS(stack.elementAt(i).data->valuestring));
                        break;
                    }

                    if (stack.elementAt(i).indexName.CompareCase(strVal)) {
                        //outValue.value.loopIndex = stack.elementAt(i).loopIndex;
                        //outValue.type = kIntValue;
                        String number('\0', 20);
                        LInt2Str(stack.elementAt(i).loopIndex, number.GetBuffer(), 10);
                        builder.append(number);
                        break;
                    }
                }
            } else {
                // such as obj.name.firstName
                OwnerPtr<KVector<String>> values = StringUtils::split(value, _CS("."));
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

                //outValue.value.json = data.ptr();
                //outValue.type = kJsonValue;
                if (data.ptr()->type == cJSON_String) {
                    builder.append((LByte*)data.ptr()->valuestring);
                }
            }
        }

        val = new String(val->Mid(end));
    } while (val->GetLength());
}

VNode* VDOMBuilder::createVNode(XMLNode* node, Stack<LoopItemData>& stack)
{
    VNode* item = NULL;
    HtmlTags* htmlTags = HtmlTags::getInstance();

    if (!node->ToText() && !node->ToComment()) {
        XMLElement* elem = node->ToElement();
        if (!elem) {
            return NULL;
        }

        String tagVal = _CS(elem->Value());
        String id = _CS(elem->Attribute("id"));
        //String tagName = _CS(elem->Attribute("name"));
        String className = _CS(elem->Attribute("class"));
        DOMValue outValue;
        fetchValue(className, stack, outValue);
        if (outValue.value.json) {
            className = _CS(outValue.value.json->valuestring);
        }

        LInt tag = htmlTags->symbolAsInt(tagVal);

        VElement* vElem = new VElement();
        vElem->setId(id);
        vElem->setClass(className);
        vElem->setTag(tag);
        item = vElem;

    } else if (node->ToText()) {
        XMLText* elem = node->ToText();
        VText* vText = new VText();
        String value;
        fetchTextValue(_CS(elem->Value()), stack, value);
        vText->setText(value);

        item = vText;
    }

    return item;
}
}
