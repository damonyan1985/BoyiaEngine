#include "CharConvertor.h"
#include "PlatformBridge.h"
#include "SalLog.h"
#if ENABLE(BOYIA_WINDOWS)
#include "AppManager.h"
#include "FileUtil.h"
#include "ZipEntry.h"
#include <ShlObj.h>
#include <stdlib.h>
#include <windows.h>

#define CSIDL_LOCAL_APPDATA 0x001c
#define MAX_PATH 260

namespace yanbo {
#define BOYIA_WIN_DEBUG

static String sAppPath((LUint8)0, MAX_PATH);
static String sBoyiaJsonPath((LUint8)0, MAX_PATH);
static String sSdkPath((LUint8)0, MAX_PATH);
static String sAppRootPath((LUint8)0, MAX_PATH);

const char* kZipPassword = "123456";

bool PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    if (dir.EndWith(_CS("sdk"))) {
        ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kBoyiaNull);
    }

    return ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kZipPassword);
}

const char* PlatformBridge::getAppPath()
{
    if (!sAppPath.GetLength()) {
        sAppPath = _CS(getAppRoot());
        sAppPath += _CS("apps\\");
    }
    return GET_STR(sAppPath);
}

const char* PlatformBridge::getBoyiaJsonPath()
{
    if (!sBoyiaJsonPath.GetLength()) {
        sBoyiaJsonPath = _CS(getAppRoot());
        sBoyiaJsonPath += _CS("boyia.json");
    }

    BOYIA_LOG("PlatformBridge---getBoyiaJsonPath: %s", GET_STR(sBoyiaJsonPath));
    return GET_STR(sBoyiaJsonPath);
}

const char* PlatformBridge::getSdkPath()
{
    if (!sSdkPath.GetLength()) {
        sSdkPath = _CS(getAppRoot());
        sSdkPath += _CS("sdk\\");
    }
    return GET_STR(sSdkPath);
}

const char* PlatformBridge::getAppRoot()
{
    if (!sAppRootPath.GetLength()) {
#ifdef BOYIA_WIN_DEBUG
        sAppRootPath = _CS(getenv("BOYIA_UI_ROOT"));
        sAppRootPath += _CS("\\BoyiaTools\\test\\project\\");
#else
        ::SHGetSpecialFolderPathA(NULL, (char*)sAppRootPath.GetBuffer(), CSIDL_LOCAL_APPDATA, 0);
        BOYIA_LOG("PlatformBridge---getAppRoot: %s BoyiaUI: %s", GET_STR(sAppRootPath), getenv("BOYIA_UI_ROOT"));
        LUint8* buffer = sAppRootPath.GetBuffer();
        LInt len = LStrlen(buffer);
        //sAppRootPath += _CS("\\boyia_app\\");
        sAppRootPath.Copy(buffer, LTrue, len);
        sAppRootPath += _CS("\\boyia_app\\");
#endif
    }
    return GET_STR(sAppRootPath);
}

const char* PlatformBridge::getBoyiaJsonUrl()
{
#ifdef BOYIA_WIN_DEBUG
    return "boyia://net_boyia.json";
#else
    return "https://damonyan1985.github.io/app/boyia.json";
#endif
}

const LInt PlatformBridge::getTextSize(const String& text)
{
    wstring wtext = CharConvertor::CharToWchar(GET_STR(text));
    return wtext.length();
}

const char* PlatformBridge::getInstructionCachePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
    if (!info->appCodePath.instructPath.GetLength()) {
        info->appCodePath.instructPath = _CS(getAppRoot());

        info->appCodePath.instructPath += _CS("dist\\");
        if (!FileUtil::isExist(GET_STR(info->appCodePath.instructPath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.instructPath));
        }

        info->appCodePath.instructPath += info->name;
        if (!FileUtil::isExist(GET_STR(info->appCodePath.instructPath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.instructPath));
        }

        info->appCodePath.instructPath += _CS("\\instruction_cache.bin");
    }

    return GET_STR(info->appCodePath.instructPath);
}

const char* PlatformBridge::getStringTableCachePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
    if (!info->appCodePath.stringTablePath.GetLength()) {
        info->appCodePath.stringTablePath = _CS(getAppRoot());
        const AppInfo* info = AppManager::instance()->currentApp()->appInfo();

        info->appCodePath.stringTablePath += _CS("dist\\");
        if (!FileUtil::isExist(GET_STR(info->appCodePath.stringTablePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.stringTablePath));
        }

        info->appCodePath.stringTablePath += info->name;
        if (!FileUtil::isExist(GET_STR(info->appCodePath.stringTablePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.stringTablePath));
        }

        info->appCodePath.stringTablePath += _CS("\\stringtable_cache.bin");
    }

    return GET_STR(info->appCodePath.stringTablePath);
}

const char* PlatformBridge::getInstructionEntryPath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
    if (!info->appCodePath.entryCodePath.GetLength()) {
        info->appCodePath.entryCodePath = _CS(getAppRoot());
        const AppInfo* info = AppManager::instance()->currentApp()->appInfo();

        info->appCodePath.entryCodePath += _CS("dist\\");
        if (!FileUtil::isExist(GET_STR(info->appCodePath.entryCodePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.entryCodePath));
        }
        info->appCodePath.entryCodePath += info->name;
        if (!FileUtil::isExist(GET_STR(info->appCodePath.entryCodePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.entryCodePath));
        }

        info->appCodePath.entryCodePath += _CS("\\instruction_entry.bin");
    }

    return GET_STR(info->appCodePath.entryCodePath);
}

const char* PlatformBridge::getSymbolTablePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
    if (!info->appCodePath.symbolTablePath.GetLength()) {
        info->appCodePath.symbolTablePath = _CS(getAppRoot());
        const AppInfo* info = AppManager::instance()->currentApp()->appInfo();

        info->appCodePath.symbolTablePath += _CS("dist\\");
        if (!FileUtil::isExist(GET_STR(info->appCodePath.symbolTablePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.symbolTablePath));
        }
        info->appCodePath.symbolTablePath += info->name;
        if (!FileUtil::isExist(GET_STR(info->appCodePath.symbolTablePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.symbolTablePath));
        }

        info->appCodePath.symbolTablePath += _CS("\\symbol_table.bin");
    }

    return GET_STR(info->appCodePath.symbolTablePath);
}

const LReal PlatformBridge::getDisplayDensity()
{
    return 10.0f;
}

PlatformBridge::PlatformType PlatformBridge::getPlatformType()
{
    return kPlatformWindows;
}
}

#endif