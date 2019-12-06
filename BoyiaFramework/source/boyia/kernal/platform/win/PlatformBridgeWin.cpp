#include "PlatformBridge.h"
#include "SalLog.h"
#include "CharConvertor.h"
#if ENABLE(BOYIA_WINDOWS)
#include "ZipEntry.h"
#include <windows.h>
#include <ShlObj.h>

#define CSIDL_LOCAL_APPDATA             0x001c
#define MAX_PATH          260

namespace yanbo {
static String sAppPath((LUint8)0, MAX_PATH);
static String sBoyiaJsonPath((LUint8)0, MAX_PATH);
static String sSdkPath((LUint8)0, MAX_PATH);
static String sAppRootPath((LUint8)0, MAX_PATH);
const char* kZipPassword = "123456";

bool PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    if (dir.EndWith(_CS("sdk"))) {
        ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kBoyiaNull);
    } else {
        ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kZipPassword);
    }
    return false;
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
        ::SHGetSpecialFolderPathA(NULL, (char*)sAppRootPath.GetBuffer(), CSIDL_LOCAL_APPDATA, 0);
        BOYIA_LOG("PlatformBridge---getAppRoot: %s\n", GET_STR(sAppRootPath));

        LUint8* buffer = sAppRootPath.GetBuffer();
        LInt len = LStrlen(buffer);
        //sAppRootPath += _CS("\\boyia_app\\");
        sAppRootPath.Copy(buffer, LTrue, len);
        sAppRootPath += _CS("\\boyia_app\\");
    }
    return GET_STR(sAppRootPath);
}

const char* PlatformBridge::getBoyiaJsonUrl()
{
    return "https://damonyan1985.github.io/app/boyia.json";
}

const LInt PlatformBridge::getTextSize(const String& text)
{
    wstring wtext = CharConvertor::CharToWchar(GET_STR(text));
    return wtext.length();
}
}

#endif