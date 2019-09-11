#ifndef JSONState_h
#define JSONState_h

#include "cJSON.h"

namespace yanbo {
class JSONState {
public:
    cJSON* get(const char* key) const;

private:
    cJSON* m_json;
};
}

#endif