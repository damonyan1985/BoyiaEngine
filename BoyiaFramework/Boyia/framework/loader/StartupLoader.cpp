#include "StartupLoader.h"
#include "AutoObject.h"
#include "BoyiaThread.h"
#include "FileUtil.h"
#include "JNIUtil.h"
#include "JSONParser.h"
#include "PlatformBridge.h"
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_APPS_SIZE 20
#define APP_LOAD_URL "https://raw.githubusercontent.com/damonyan1985/BoyiaApp/master/boyia.json"
//#define APPS_JSON_PATH "/data/data/com.boyia.app/files/apps/boyia.json"
//#define APPS_PATH "/data/data/com.boyia.app/files/apps/"
#define APP_JSON "/app.json"

namespace yanbo {

static LVoid BoyiaThreadLoad(const char* entry)
{
    String url = _CS(entry);
    BoyiaThread::instance()->load(url);
    url.ReleaseBuffer();
}

class AppInfo {
public:
    AppInfo()
        : isEntry(LFalse)
        , versionCode(0)
    {
    }

    LVoid parseApps(cJSON* appsJson)
    {
        cJSON* appJson = appsJson->child;
        while (appJson) {
            cJSON* item = appJson->child;
            while (item) {
                if (0 == strcmp(item->string, "versionCode")) {
                    versionCode = item->valueint;
                    KFORMATLOG("boyia app version=%d", item->valueint);
                } else if (0 == strcmp(item->string, "path")) {
                    path = _CS(item->valuestring);
                    KFORMATLOG("boyia app path=%s", item->valuestring);
                } else if (0 == strcmp(item->string, "url")) {
                    url = _CS(item->valuestring);
                    KFORMATLOG("boyia app url=%s", item->valuestring);
                } else if (0 == strcmp(item->string, "name")) {
                    name = _CS(item->valuestring);
                    KFORMATLOG("boyia app name=%s", item->valuestring);
                } else if (0 == strcmp(item->string, "isEntry")) {
                    isEntry = item->type == cJSON_True ? LTrue : LFalse;
                    KFORMATLOG("boyia app isEntry=%d", item->type);
                }

                item = item->next;
            }

            appJson = appJson->next;
        }
    }

public:
    LInt versionCode; // app version code
    String path; // download in local path
    String name; // app name
    String url; // download url
    LBool isEntry;
};

class AppHandler : public NetworkClient {
public:
    AppHandler(const String& name, LBool launchable)
        : m_launchable(launchable)
    {
        m_appDir = _CS(PlatformBridge::getAppPath()) + name;
        m_appFilePath = m_appDir + _CS("_tmp.zip");

        BOYIA_LOG("boyia app AppHandler name=%s", GET_STR(m_appFilePath));
        m_appFile = fopen(GET_STR(m_appFilePath), "wb+");
    }

    virtual void onDataReceived(const LByte* data, LInt size)
    {
        fwrite(data, size, 1, m_appFile);
    }

    virtual void onStatusCode(LInt statusCode)
    {
    }

    virtual void onRedirectUrl(const String& redirectUrl)
    {
    }

    virtual void onLoadError(LInt error)
    {
        fclose(m_appFile);
        delete this;
    }

    virtual void onLoadFinished()
    {
        // 下载完成
        fclose(m_appFile);
        // 解压到应用程序目录
        if (m_launchable) {
            PlatformBridge::unzip(m_appFilePath, m_appDir);
            FileUtil::printAllFiles("/data/data/com.boyia.app/files/");

            String appJsonPath = m_appDir + _CS(APP_JSON);
            boyia::JSONParser parser(appJsonPath);

            //KFORMATLOG("boyia app AppHandler entry=%s", entry->valuestring);
            BoyiaThreadLoad(parser.get("entry")->valuestring);
            FileUtil::deleteFile(GET_STR(m_appFilePath));
        }

        delete this;
    }

    virtual void onFileLen(LInt len)
    {
        BOYIA_LOG("boyia app AppHandler size=%d", len);
    }

private:
    FILE* m_appFile;
    String m_appDir;
    String m_appFilePath;
    LBool m_launchable;
};

StartupLoader::StartupLoader()
    : m_appInfos(0, MAX_APPS_SIZE)
{
}

LVoid StartupLoader::startLoad()
{
    if (!FileUtil::isExist(PlatformBridge::getAppPath())) {
        mkdir(PlatformBridge::getAppPath(), S_IRWXU);
    }

    m_file = fopen(PlatformBridge::getAppJsonPath(), "wb+");

    m_loader.loadUrl(_CS(APP_LOAD_URL), this);
    BOYIA_LOG("StartupLoader---startLoad m_file=%d", (LIntPtr)m_file);
}

LVoid StartupLoader::loadApp()
{
    if (!FileUtil::isExist(PlatformBridge::getAppJsonPath())) {
        return;
    }
    parseConfig();
    startLoadApp();
}

LVoid StartupLoader::onDataReceived(const LByte* data, LInt size)
{
    if (m_file) {
        fwrite(data, size, 1, m_file);
    }
}

LVoid StartupLoader::onStatusCode(LInt statusCode)
{
}

LVoid StartupLoader::onFileLen(LInt len)
{
}

LVoid StartupLoader::onRedirectUrl(const String& redirectUrl)
{
}

LVoid StartupLoader::onLoadError(LInt error)
{
    if (m_file) {
        fclose(m_file);
    }
}

LVoid StartupLoader::onLoadFinished()
{
    if (!m_file) {
        return;
    }

    fclose(m_file);
    loadApp();
}

LVoid StartupLoader::parseConfig()
{
    String content;

    String path(_CS(PlatformBridge::getAppJsonPath()), LFalse, LStrlen((LUint8*)PlatformBridge::getAppJsonPath()));
    util::FileUtil::readFile(path, content);
    BOYIA_LOG("boyia app content=%s", GET_STR(content));

    cJSON* json = cJSON_Parse(GET_STR(content));
    if (json->type == cJSON_Object) {
        cJSON* child = json->child;
        while (child) {
            if (child->type == cJSON_Array
                && 0 == strcmp(child->string, "apps")) {
                AppInfo* appInfo = new AppInfo;
                appInfo->parseApps(child);
                m_appInfos.addElement(appInfo);
            } else if (child->type == cJSON_String
                && 0 == strcmp(child->string, "retCode")) {
            }

            child = child->next;
        }
    }

    cJSON_Delete(json);
}

LVoid StartupLoader::upgradeApp(const String& name)
{
    String appDir = _CS(PlatformBridge::getAppPath()) + name;
    String appFilePath = appDir + _CS("_tmp.zip");
    if (FileUtil::isExist(GET_STR(appFilePath))) {
        // 删除APP目录
        FileUtil::deleteFile(GET_STR(appDir));
        // 解压到当前目录
        PlatformBridge::unzip(appFilePath, appDir);
        // 删除下载的文件
        FileUtil::deleteFile(GET_STR(appFilePath));
    }
}

LVoid StartupLoader::startLoadApp()
{
    LInt size = m_appInfos.size();
    for (LInt id = 0; id < size; ++id) {
        // 升级App
        upgradeApp(m_appInfos[id]->name);
        String appDir = _CS(PlatformBridge::getAppPath()) + m_appInfos[id]->name;
        LInt versionCode = 0;
        LBool hasApp = FileUtil::isExist(GET_STR(appDir));
        if (hasApp) {
            // Get App Json Info
            String appJsonPath = appDir + _CS(APP_JSON);
            // LOG AppPath
            BOYIA_LOG("StartupLoader.cpp---startLoadApp---appJsonPath=%s", GET_STR(appJsonPath));

            if (!FileUtil::isExist(GET_STR(appJsonPath))) {
                continue;
            }

            boyia::JSONParser parser(appJsonPath);
            // Get App Version
            cJSON* version = parser.get("versionCode");
            if (version) {
                versionCode = version->valueint;
            }

            if (m_appInfos[id]->isEntry) {
                BoyiaThreadLoad(parser.get("entry")->valuestring);
            }
        }

        // If the versionCode boyia.json greater than the version
        // which in local app.json
        if (m_appInfos[id]->versionCode > versionCode) {
            m_loader.loadUrl(m_appInfos[id]->url, new AppHandler(m_appInfos[id]->name, !hasApp && m_appInfos[id]->isEntry));
        }
    }
}
}
