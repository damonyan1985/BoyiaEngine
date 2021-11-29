#include "PlatformBridge.h"
#include "SalLog.h"
#include "ZipEntry.h"

#if ENABLE(BOYIA_IOS)
#include "AppManager.h"
#include "FileUtil.h"
#include "IOSRenderer.h"

namespace yanbo {
// TODO
const char* kZipPassword = "123456";

static String sAppPath((LUint8)0, 256);
static String sBoyiaJsonPath((LUint8)0, 256);
static String sSdkPath((LUint8)0, 256);
static String sAppRootPath((LUint8)0, 256);

bool PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    BOYIA_LOG("PlatformBridge---unzip---src: %s, dest: %s", GET_STR(zipFile), GET_STR(dir));

    if (dir.EndWith(_CS("sdk"))) {
        return ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kBoyiaNull);
    }

    return ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kZipPassword);
}

const char* PlatformBridge::getAppPath()
{
    if (!sAppPath.GetLength()) {
        sAppPath = _CS(getAppRoot());
        sAppPath += _CS("apps/");
    }

    BOYIA_LOG("PlatformBridge---getAppPath: %s", GET_STR(sAppPath));
    return GET_STR(sAppPath);
    //return "/data/data/com.boyia.app/files/apps/";
}

const char* PlatformBridge::getBoyiaJsonPath()
{
    if (!sBoyiaJsonPath.GetLength()) {
        sBoyiaJsonPath = _CS(getAppRoot());
        sBoyiaJsonPath += _CS("boyia.json");
    }

    BOYIA_LOG("PlatformBridge---getBoyiaJsonPath: %s", GET_STR(sBoyiaJsonPath));
    return GET_STR(sBoyiaJsonPath);
    //return "/data/data/com.boyia.app/files/boyia.json";
}

const char* PlatformBridge::getSdkPath()
{
    if (!sSdkPath.GetLength()) {
        sSdkPath = _CS(getAppRoot());
        sSdkPath += _CS("sdk/");
    }

    BOYIA_LOG("PlatformBridge---getSdkPath: %s", GET_STR(sSdkPath));
    return GET_STR(sSdkPath);
    //return "/data/data/com.boyia.app/files/sdk/";
}

const char* PlatformBridge::getAppRoot()
{
    if (sAppRootPath.GetLength()) {
        return GET_STR(sAppRootPath);
    }
    
    NSBundle* coreBundle = [NSBundle bundleWithIdentifier:@"com.boyia.core"];
    NSString* appDir = [coreBundle pathForResource:@"metal" ofType:@"bundle"];
    NSBundle* appBundle = [NSBundle bundleWithPath:appDir];
    //NSString* appDir = [NSBundle bundleWithIdentifier:@"com.boyia.metal"].bundlePath;
//    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
//    NSString* docDir = [paths objectAtIndex:0];
//    NSString* appDir = [docDir stringByAppendingString:@"/boyia/"];
//    NSFileManager* fileManager = [NSFileManager defaultManager];
//    if (![fileManager fileExistsAtPath:appDir]) {
//        [fileManager createDirectoryAtPath:appDir withIntermediateDirectories:YES attributes:nil error:nil];
//    }
    NSString* boyiaAppDir = [appBundle.bundlePath stringByAppendingString:@"/"];
    sAppRootPath = (const LUint8*)([boyiaAppDir UTF8String]);
    return GET_STR(sAppRootPath);
}

const char* PlatformBridge::getInstructionCachePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
    if (!info->appCodePath.instructPath.GetLength()) {
        info->appCodePath.instructPath = _CS(getAppRoot());

        info->appCodePath.instructPath += _CS("dist/");
        if (!FileUtil::isExist(GET_STR(info->appCodePath.instructPath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.instructPath));
        }

        info->appCodePath.instructPath += info->name;
        if (!FileUtil::isExist(GET_STR(info->appCodePath.instructPath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.instructPath));
        }

        info->appCodePath.instructPath += _CS("/instruction_cache.bin");
    }

    return GET_STR(info->appCodePath.instructPath);
    //return "/data/data/com.boyia.app/files/instruction_cache.bin";
}

const char* PlatformBridge::getStringTableCachePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
    if (!info->appCodePath.stringTablePath.GetLength()) {
        info->appCodePath.stringTablePath = _CS(getAppRoot());
        const AppInfo* info = AppManager::instance()->currentApp()->appInfo();

        info->appCodePath.stringTablePath += _CS("dist/");
        if (!FileUtil::isExist(GET_STR(info->appCodePath.stringTablePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.stringTablePath));
        }

        info->appCodePath.stringTablePath += info->name;
        if (!FileUtil::isExist(GET_STR(info->appCodePath.stringTablePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.stringTablePath));
        }

        info->appCodePath.stringTablePath += _CS("/stringtable_cache.bin");
    }

    return GET_STR(info->appCodePath.stringTablePath);
}

const char* PlatformBridge::getInstructionEntryPath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
    if (!info->appCodePath.entryCodePath.GetLength()) {
        info->appCodePath.entryCodePath = _CS(getAppRoot());
        const AppInfo* info = AppManager::instance()->currentApp()->appInfo();

        info->appCodePath.entryCodePath += _CS("dist/");
        if (!FileUtil::isExist(GET_STR(info->appCodePath.entryCodePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.entryCodePath));
        }
        info->appCodePath.entryCodePath += info->name;
        if (!FileUtil::isExist(GET_STR(info->appCodePath.entryCodePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.entryCodePath));
        }

        info->appCodePath.entryCodePath += _CS("/instruction_entry.bin");
    }

    return GET_STR(info->appCodePath.entryCodePath);
}

const char* PlatformBridge::getSymbolTablePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
    if (!info->appCodePath.symbolTablePath.GetLength()) {
        info->appCodePath.symbolTablePath = _CS(getAppRoot());
        const AppInfo* info = AppManager::instance()->currentApp()->appInfo();

        info->appCodePath.symbolTablePath += _CS("dist/");
        if (!FileUtil::isExist(GET_STR(info->appCodePath.symbolTablePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.symbolTablePath));
        }
        info->appCodePath.symbolTablePath += info->name;
        if (!FileUtil::isExist(GET_STR(info->appCodePath.symbolTablePath))) {
            FileUtil::createDir(GET_STR(info->appCodePath.symbolTablePath));
        }

        info->appCodePath.symbolTablePath += _CS("/symbol_table.bin");
    }

    return GET_STR(info->appCodePath.symbolTablePath);
}

const char* PlatformBridge::getBoyiaJsonUrl()
{
    return "boyia://net_boyia.json";
    //return "https://damonyan1985.github.io/app/boyia.json";
}

const LInt PlatformBridge::getTextSize(const String& text)
{
    NSString* str = STR_TO_OCSTR(text);
    return (LInt)str.length;
}

const LReal PlatformBridge::getDisplayDensity()
{
    // TODO
    return 0;
}

PlatformBridge::PlatformType PlatformBridge::getPlatformType()
{
    return kPlatformIos;
}
}

#endif
