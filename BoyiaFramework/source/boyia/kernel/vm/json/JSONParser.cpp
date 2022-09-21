#include "JSONParser.h"
#include "FileUtil.h"
#include "TinyXml2.h"
#include "StringUtils.h"
#include <string.h>

using namespace tinyxml2;

namespace boyia {
JSONIterator::JSONIterator(cJSON* json)
    : m_json(json)
{
}

JSONIterator JSONIterator::next()
{
    return JSONIterator(m_json->next);
}

cJSON* JSONIterator::operator*()
{
    return m_json;
}

LBool JSONIterator::operator!=(const JSONIterator& iter)
{
    return iter.m_json == m_json;
}

JSONIterator& JSONIterator::operator=(const JSONIterator& iter)
{
    m_json = iter.m_json;
    return *this;
}

JSONParser::JSONParser(const String& str, SourceType type)
{
    if (type == kSourceJsonFile) {
        String content;
        FileUtil::readFile(str, content);
        BOYIA_LOG("JSONParser---JSONParser() content=%s", GET_STR(content));
        m_json = cJSON_Parse(GET_STR(content));
    } else {
        m_json = cJSON_Parse(GET_STR(str));
    }
    
    switch (type) {
        case kSourceJsonFile: {
            String content;
            FileUtil::readFile(str, content);
            BOYIA_LOG("JSONParser---JSONParser() content=%s", GET_STR(content));
            m_json = cJSON_Parse(GET_STR(content));
        } break;
        case kSourceJsonText: {
            m_json = cJSON_Parse(GET_STR(str));
        } break;
        case kSourceXmlFile: {
            String content;
            FileUtil::readFile(str, content);
            
            parseXml(content);
        } break;
        case kSourceXmlText: {
            parseXml(str);
        } break;
    }
}

JSONParser::~JSONParser()
{
    cJSON_Delete(m_json);
}

const cJSON* JSONParser::json() const
{
    return m_json;
}

cJSON* JSONParser::get(const char* key) const
{
    cJSON* child = m_json->child;
    while (child) {
        if (0 == strcmp(child->string, key)) {
            return child;
        }

        child = child->next;
    }

    return NULL;
}

JSONIterator JSONParser::begin() const
{
    return JSONIterator(m_json->child);
}

static cJSON* createJsonObject(XMLNode* node)
{
    if (node->ToText()) {
        XMLText* elem = node->ToText();
        cJSON* json = cJSON_CreateObject();
        
        cJSON_AddStringToObject(json, "tag", "#text");
        cJSON_AddStringToObject(json, "text", elem->Value());
        
        return json;
    }
    
    if (!node->ToElement()) {
        return kBoyiaNull;
    }

    cJSON* json = cJSON_CreateObject();
    // 添加tag name
    cJSON_AddStringToObject(json, "tag", node->Value());
    
    // 添加属性
    XMLElement* elem = static_cast<XMLElement*>(node);
    if (!elem->FirstAttribute()) {
        return json;
    }
    
    cJSON* props = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "props", props);
    
    for (const XMLAttribute* attr = elem->FirstAttribute(); attr; attr = attr->Next()) {
        // TODO 暂时默认属性值都是字符串
        cJSON_AddStringToObject(props, attr->Name(), attr->Value());
    }
    
    return json;
}

static cJSON* traverseXmlNodeToJson(XMLNode* node)
{
    cJSON* json = createJsonObject(node);
    // 如果不是element，则不可能有子元素
    if (!json) {
        return json;
    }
    
    LBool hasChild = node->FirstChild() != kBoyiaNull;
    if (!hasChild) {
        return json;
    }
    
    cJSON* children = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "children", children);
    
    for (XMLNode* child = node->FirstChild(); child; child = child->NextSibling()) {
        cJSON_AddItemToArray(children, traverseXmlNodeToJson(child));
    }
    
    return json;
}

LVoid JSONParser::parseXml(const String& str)
{
    XMLDocument dom;
    dom.Parse(GET_STR(str));
    
    XMLElement* root = dom.RootElement();
    if (!root) {
        return;
    }
    
    m_json = traverseXmlNodeToJson(root);
}

LVoid JSONParser::toJson(String& json) const
{
    json = _CS(cJSON_Print(m_json));
}
}
