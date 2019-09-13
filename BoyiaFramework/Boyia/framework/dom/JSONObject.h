#ifndef JSONObject_h
#define JSONObject_h

#include "UtilString.h"
#include "cJSON.h"

namespace yanbo {
class JSONObject {
public:
    JSONObject(cJSON* json = nullptr);
    JSONObject& operator=(cJSON* json);
    cJSON* get(const char* key) const;
    cJSON* get(const String& key) const;
    cJSON* ptr() const;

private:
    cJSON* m_json;
};
}

#endif