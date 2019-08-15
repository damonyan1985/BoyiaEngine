#include "AppLoader.h"
#include "AppManager.h"
#include "AppThread.h"
#include "AutoObject.h"
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
#define APP_JSON "/app.json"

namespace yanbo {

class AppHandler : public NetworkClient {
public:
    AppHandler(AppInfo* info, LBool launchable)
        : m_launchable(launchable)
        , m_info(info)
    {
        m_appDir = _CS(PlatformBridge::getAppPath()) + m_info->name;
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
        PlatformBridge::unzip(m_appFilePath, m_appDir);
        if (m_launchable) {
            FileUtil::printAllFiles("/data/data/com.boyia.app/files/");

            String appJsonPath = m_appDir + _CS(APP_JSON);
            boyia::JSONParser parser(appJsonPath);

            m_info->path = _CS(parser.get("entry")->valuestring);
            AppManager::instance()->launchApp(m_info);
            BOYIA_LOG("AppHandler.js---onLoadFinished %s", GET_STR(m_info->path));
            FileUtil::deleteFile(GET_STR(m_appFilePath));
            BOYIA_LOG("AppHandler.js---onLoadFinished %d", 6);
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
    AppInfo* m_info;
};

AppLoader::AppLoader(AppManager* manager)
    : m_appInfos(0, MAX_APPS_SIZE)
    , m_manager(manager)
    , m_sdk(NULL)
{
}

LVoid AppLoader::startLoad()
{
    if (!FileUtil::isExist(PlatformBridge::getAppPath())) {
        mkdir(PlatformBridge::getAppPath(), S_IRWXU);
    }

    m_file = fopen(PlatformBridge::getAppJsonPath(), "wb+");

    m_loader.loadUrl(_CS(APP_LOAD_URL), this);
    BOYIA_LOG("AppLoader---startLoad m_file=%d", (LIntPtr)m_file);
}

LVoid AppLoader::loadApps()
{
    if (!FileUtil::isExist(PlatformBridge::getAppJsonPath())) {
        return;
    }
    parseConfig();
    startLoadApps();
}

LVoid AppLoader::onDataReceived(const LByte* data, LInt size)
{
    if (m_file) {
        fwrite(data, size, 1, m_file);
    }
}

LVoid AppLoader::onStatusCode(LInt statusCode)
{
}

LVoid AppLoader::onFileLen(LInt len)
{
}

LVoid AppLoader::onRedirectUrl(const String& redirectUrl)
{
}

LVoid AppLoader::onLoadError(LInt error)
{
    if (m_file) {
        fclose(m_file);
    }
}

LVoid AppLoader::onLoadFinished()
{
    if (!m_file) {
        return;
    }

    fclose(m_file);
    loadApps();
}

static LVoid appendAppInfo(KVector<AppInfo*>& appInfos, cJSON* appsJson)
{
    cJSON* appJson = appsJson->child;
    while (appJson) {
        AppInfo* appInfo = new AppInfo;
        appInfo->parseApp(appJson);

        appInfos.addElement(appInfo);
        appJson = appJson->next;
    }
}

LVoid AppLoader::parseConfig()
{
    String content;

    String path(_CS(PlatformBridge::getAppJsonPath()), LFalse, LStrlen((LUint8*)PlatformBridge::getAppJsonPath()));
    util::FileUtil::readFile(path, content);
    BOYIA_LOG("boyia app content=%s", GET_STR(content));

    cJSON* json = cJSON_Parse(GET_STR(content));
    if (json->type == cJSON_Object) {
        cJSON* child = json->child;
        while (child) {
            if (child->type == cJSON_Array && 0 == strcmp(child->string, "apps")) {
                appendAppInfo(m_appInfos, child);
            } else if (child->type == cJSON_String && 0 == strcmp(child->string, "code")) {
            } else if (child->type == cJSON_Object && 0 == strcmp(child->string, "sdk")) {
                m_sdk = new AppInfo;
                m_sdk->parseApp(child);
            }

            child = child->next;
        }
    }

    cJSON_Delete(json);
}

LVoid AppLoader::upgradeApp(const String& name)
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

LVoid AppLoader::loadApp(AppInfo* info)
{
    // 升级App
    upgradeApp(info->name);
    String appDir = _CS(PlatformBridge::getAppPath()) + info->name;
    LInt versionCode = 0;
    LBool hasApp = FileUtil::isExist(GET_STR(appDir));
    if (hasApp) {
        // Get App Json Info
        String appJsonPath = appDir + _CS(APP_JSON);
        // LOG AppPath
        BOYIA_LOG("AppLoader.cpp---startLoadApp---appJsonPath=%s", GET_STR(appJsonPath));

        if (!FileUtil::isExist(GET_STR(appJsonPath))) {
            return;
        }

        boyia::JSONParser parser(appJsonPath);
        // Get App Version
        cJSON* version = parser.get("versionCode");
        if (version) {
            versionCode = version->valueint;
        }

        if (info->isEntry) {
            //BoyiaThreadLoad(parser.get("entry")->valuestring);
            info->path = _CS(parser.get("entry")->valuestring);
            m_manager->launchApp(info);
        }
    }

    // If the versionCode boyia.json greater than the version
    // which in local app.json
    if (info->versionCode > versionCode) {
        m_loader.loadUrl(info->url, new AppHandler(info, !hasApp && info->isEntry));
    }
}

LVoid AppLoader::startLoadApps()
{
    // Download BoyiaSDK
    loadApp(m_sdk);

    // Download Boyia App
    LInt size = m_appInfos.size();
    for (LInt id = 0; id < size; ++id) {
        loadApp(m_appInfos[id]);
    }
}
}
