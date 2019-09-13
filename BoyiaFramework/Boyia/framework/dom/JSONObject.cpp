#include "JSONObject.h"
#include <string.h>

namespace yanbo {
JSONObject::JSONObject(cJSON* json)
    : m_json(json)
{
}
cJSON* JSONObject::get(const char* key) const
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

cJSON* JSONObject::get(const String& key) const
{
    return get(GET_STR(key));
}

JSONObject& JSONObject::operator=(cJSON* json)
{
    m_json = json;
    return *this;
}

cJSON* JSONObject::ptr() const
{
    return m_json;
}
}