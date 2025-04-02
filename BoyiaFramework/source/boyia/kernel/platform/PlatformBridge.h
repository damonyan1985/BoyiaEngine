#ifndef PlatformBridge_h
#define PlatformBridge_h

#include "UtilString.h"
#include <functional>

namespace yanbo {
class PlatformBridge {
public:
    enum PlatformType {
        kPlatformAndroid = 0,
        kPlatformIos,
        kPlatformWindows
    };
    
    static bool unzip(const String& zipFile, const String& dir);
    static const char* getCachePath(String& cachePath, const String& binName);
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
    static const char* getDebugInfoPath();
    static const LReal getDisplayDensity();
    static void handleApi(const String& params, LIntPtr callback);
    
    static PlatformType getPlatformType();
};
}

#endif
