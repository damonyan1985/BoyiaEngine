#include "AppInfo.h"
#include "SalLog.h"
#include "PlatformBridge.h"
#include "FileUtil.h"
#include <string.h>

#define APP_JSON "/app.json"

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

LBool AppInfo::isKey(cJSON* item, const char* key)
{
    return 0 == strcmp(item->string, key);
}

LVoid AppInfo::parseApp(cJSON* appJson)
{
    cJSON* item = appJson->child;
    while (item) {
        if (isKey(item, "versionCode")) {
            versionCode = item->valueint;
            BOYIA_LOG("boyia app version=%d", item->valueint);
        } else if (isKey(item, "path")) {
            path = _CS(item->valuestring);
            BOYIA_LOG("boyia app path=%s", item->valuestring);
        } else if (isKey(item, "url")) {
            url = _CS(item->valuestring);
            BOYIA_LOG("boyia app url=%s", item->valuestring);
        } else if (isKey(item, "name")) {
            name = _CS(item->valuestring);
            BOYIA_LOG("boyia app name=%s", item->valuestring);
        } else if (isKey(item, "isEntry")) {
            isEntry = item->type == cJSON_True ? LTrue : LFalse;
            BOYIA_LOG("boyia app isEntry=%d", item->type);
        } else if (isKey(item, "id")) {
            id = item->valueint;
            BOYIA_LOG("boyia app id=%d", item->valueint);
        } else if (isKey(item, "cover")) {
            cover = _CS(item->valuestring);
        }

        item = item->next;
    }
}

LVoid AppInfo::parse()
{
    String appDir = _CS(PlatformBridge::getAppPath()) + name;
    if (FileUtil::isExist(GET_STR(appDir))) {
        // Get App Json Info
        String appJsonPath = appDir + _CS(APP_JSON);
        // LOG AppPath
        BOYIA_LOG("AppInfo.cpp---parse()---appJsonPath=%s", GET_STR(appJsonPath));

        if (!FileUtil::isExist(GET_STR(appJsonPath))) {
            return;
        }

        boyia::JSONParser parser(appJsonPath);
        // Get App Version
        cJSON* version = parser.get("versionCode");
        if (version) {
            versionCode = version->valueint;
        }

        cJSON* entry = parser.get("entry");
        if (entry) {
            path = _CS(entry->valuestring);
        }
    }
}
}
