#ifndef PlatformBridge_h
#define PlatformBridge_h

#include "UtilString.h"

namespace yanbo {

class PlatformBridge {
public:
    static bool unzip(const String& zipFile, const String& dir);
    static const char* getAppRoot();
    static const char* getAppPath();
    static const char* getBoyiaJsonPath();
    static const char* getSdkPath();
    static const char* getBoyiaJsonUrl();
};
}

#endif