// Author Yanbo
#ifndef JSONParser_h
#define JSONParser_h

#include "UtilString.h"
#include "cJSON.h"

namespace boyia {
class JSONIterator {
public:
    JSONIterator(cJSON* json);
    JSONIterator next();
    cJSON* operator*();
    JSONIterator& operator=(const JSONIterator& iter);
    LBool operator!=(const JSONIterator& iter);

private:
    cJSON* m_json;
};

class JSONParser {
public:
    enum SourceType {
        kSourceJsonFile,
        kSourceJsonText,
        kSourceXmlFile,
        kSourceXmlText // 将xml文本转换成json
    };
    
    JSONParser(const String& str, SourceType type = kSourceJsonFile);
    ~JSONParser();

    const cJSON* json() const;
    cJSON* get(const char* key) const;

    JSONIterator begin() const;
    
    LVoid toJson(String& json) const;

private:
    LVoid parseXml(const String& path);
    
    cJSON* m_json;
};
}
#endif
