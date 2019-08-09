#include "AppInfo.h"
#include "SalLog.h"
#include <string.h>

namespace yanbo {
AppInfo::AppInfo()
    : isEntry(LFalse)
    , versionCode(0)
{
}

LVoid AppInfo::parseApps(cJSON* appsJson)
{
    cJSON* appJson = appsJson->child;
    while (appJson) {
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
            }

            item = item->next;
        }

        appJson = appJson->next;
    }
}
}