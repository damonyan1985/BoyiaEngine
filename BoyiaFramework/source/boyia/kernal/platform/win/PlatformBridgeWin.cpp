#include "PlatformBridge.h"
#include "SalLog.h"

namespace yanbo {

#if ENABLE(BOYIA_WINDOWS)
bool PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    return false;
}

const char* PlatformBridge::getAppPath()
{
    return "";
}

const char* PlatformBridge::getBoyiaJsonPath()
{
    return "";
}

const char* PlatformBridge::getSdkPath()
{
    return "";
}

const char* PlatformBridge::getAppRoot()
{
    return "";
}

const char* PlatformBridge::getBoyiaJsonUrl()
{
    return "";
}

#endif
}