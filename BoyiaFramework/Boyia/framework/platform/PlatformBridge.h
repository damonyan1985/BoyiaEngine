#ifndef PlatformBridge_h
#define PlatformBridge_h

#include "UtilString.h"

namespace yanbo {

class PlatformBridge {
public:
    static bool unzip(const String& zipFile, const String& dir);
    static const char* getAppPath();
    static const char* getAppJsonPath();
    static const char* getSdkPath();
};
}

#endif