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
#include <shlwapi.h>

#define CSIDL_LOCAL_APPDATA 0x001c

namespace yanbo {
#define BOYIA_WIN_DEBUG

static String sAppPath((LUint8)0, MAX_PATH_SIZE);
static String sBoyiaJsonPath((LUint8)0, MAX_PATH_SIZE);
static String sSdkPath((LUint8)0, MAX_PATH_SIZE);
static String sAppRootPath((LUint8)0, MAX_PATH_SIZE);

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

const LReal PlatformBridge::getDisplayDensity()
{
    return 10.0f;
}

PlatformBridge::PlatformType PlatformBridge::getPlatformType()
{
    return kPlatformWindows;
}

void PlatformBridge::handleApi(const String& params, LIntPtr callback)
{    
}

bool PlatformBridge::isAbsolutePath(const String& path)
{
    return !PathIsRelativeA(GET_STR(path)) || PathIsUNCA(GET_STR(path));
}
}

#endif