#include "AppInfo.h"
#include "SalLog.h"
#include <string.h>

namespace yanbo {
AppCodePath::AppCodePath()
    : instructPath((LUint8)0, 256)
    , entryCodePath((LUint8)0, 256)
    , stringTablePath((LUint8)0, 256)
    , symbolTablePath((LUint8)0, 256)
{
}

AppInfo::AppInfo()
    : isEntry(LFalse)
    , versionCode(0)
    , id(0)
{
}

AppInfo::~AppInfo()
{
}

LVoid AppInfo::parseApp(cJSON* appJson)
{
    cJSON* item = appJson->child;
    while (item) {
        if (0 == strcmp(item->string, "versionCode")) {
            versionCode = item->valueint;
            BOYIA_LOG("boyia app version=%d", item->valueint);
        } else if (0 == strcmp(item->string, "path")) {
            path = _CS(item->valuestring);
            BOYIA_LOG("boyia app path=%s", item->valuestring);
        } else if (0 == strcmp(item->string, "url")) {
            url = _CS(item->valuestring);
            BOYIA_LOG("boyia app url=%s", item->valuestring);
        } else if (0 == strcmp(item->string, "name")) {
            name = _CS(item->valuestring);
            BOYIA_LOG("boyia app name=%s", item->valuestring);
        } else if (0 == strcmp(item->string, "isEntry")) {
            isEntry = item->type == cJSON_True ? LTrue : LFalse;
            BOYIA_LOG("boyia app isEntry=%d", item->type);
        } else if (0 == strcmp(item->string, "id")) {
            id = item->valueint;
            BOYIA_LOG("boyia app id=%d", item->valueint);
        }

        item = item->next;
    }
}
}