#include "PlatformBridge.h"
#include "SalLog.h"
#if ENABLE(BOYIA_WINDOWS)
#include <windows.h>
#include <ShlObj.h>

#define CSIDL_LOCAL_APPDATA             0x001c
#define MAX_PATH          260

namespace yanbo {
static String sAppPath((LUint8)0, MAX_PATH);
static String sBoyiaJsonPath((LUint8)0, MAX_PATH);
static String sSdkPath((LUint8)0, MAX_PATH);
static String sAppRootPath((LUint8)0, MAX_PATH);

bool PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    return false;
}

const char* PlatformBridge::getAppPath()
{
    if (!sAppPath.GetLength()) {
        sAppPath = _CS(getAppRoot());
        sAppPath += _CS("\\apps\\");
    }
    return GET_STR(sAppPath);
}

const char* PlatformBridge::getBoyiaJsonPath()
{
    if (!sBoyiaJsonPath.GetLength()) {
        sBoyiaJsonPath = _CS(getAppRoot());
        sBoyiaJsonPath += _CS("\\boyia.json");
    }
    return GET_STR(sBoyiaJsonPath);
}

const char* PlatformBridge::getSdkPath()
{
    if (!sSdkPath.GetLength()) {
        sSdkPath = _CS(getAppRoot());
        sSdkPath += _CS("\\sdk");
    }
    return GET_STR(sSdkPath);
}

const char* PlatformBridge::getAppRoot()
{
    if (!sAppRootPath.GetLength()) {
        ::SHGetSpecialFolderPathA(NULL, (char*)sAppRootPath.GetBuffer(), CSIDL_LOCAL_APPDATA, 0);
    }
    return GET_STR(sAppRootPath);
}

const char* PlatformBridge::getBoyiaJsonUrl()
{
    return "";
}
}

#endif