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
    static const LInt getTextSize(const String& text);
    static const char* getInstructionCachePath();
    static const char* getStringTableCachePath();
    static const char* getInstructionEntryPath();
    static const char* getSymbolTablePath();
    static const LReal getDisplayDensity();
};
}

#endif