#include "JSONState.h"
#include <string.h>

namespace yanbo {
cJSON* JSONState::get(const char* key) const
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
}