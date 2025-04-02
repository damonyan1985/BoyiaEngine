#include "PlatformBridge.h"
#include "FileUtil.h"
#include "AppManager.h"

namespace yanbo {
const char* PlatformBridge::getCachePath(String& cachePath, const String& binName)
{
    if (!cachePath.GetLength()) {
        cachePath = _CS(getAppRoot());
#if ENABLE(BOYIA_WINDOWS)
        cachePath += _CS("dist\\");
#else
        cachePath += _CS("dist/");
#endif
        if (!FileUtil::isExist(GET_STR(cachePath))) {
            FileUtil::createDir(GET_STR(cachePath));
        }
        cachePath += AppManager::instance()->currentApp()->appInfo()->name;
        if (!FileUtil::isExist(GET_STR(cachePath))) {
            FileUtil::createDir(GET_STR(cachePath));
        }

        cachePath += binName;
    }

    return GET_STR(cachePath);
}

const char* PlatformBridge::getInstructionCachePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
#if ENABLE(BOYIA_WINDOWS)    
    return getCachePath(info->appCodePath.instructPath, _CS("\\instruction_cache.bin"));
#else
    // For Android, Path is "/data/data/com.boyia.app/files/instruction_cache.bin"
    return getCachePath(info->appCodePath.instructPath, _CS("/instruction_cache.bin"));
#endif    
}

const char* PlatformBridge::getStringTableCachePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
#if ENABLE(BOYIA_WINDOWS)    
    return getCachePath(info->appCodePath.instructPath, _CS("\\stringtable_cache.bin"));
#else
    // For Android, Path is "/data/data/com.boyia.app/files/stringtable_cache.bin"
    return getCachePath(info->appCodePath.instructPath, _CS("/stringtable_cache.bin"));
#endif      
}

const char* PlatformBridge::getInstructionEntryPath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
#if ENABLE(BOYIA_WINDOWS)    
    return getCachePath(info->appCodePath.instructPath, _CS("\\instruction_entry.bin"));
#else
    // For Android, Path is "/data/data/com.boyia.app/files/instruction_entry.bin"
    return getCachePath(info->appCodePath.instructPath, _CS("/instruction_entry.bin"));
#endif    
}

const char* PlatformBridge::getSymbolTablePath()
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
#if ENABLE(BOYIA_WINDOWS)    
    return getCachePath(info->appCodePath.instructPath, _CS("\\symbol_table.bin"));
#else
    // For Android, Path is "/data/data/com.boyia.app/files/symbol_table.bin"
    return getCachePath(info->appCodePath.instructPath, _CS("/symbol_table.bin"));
#endif    
}

const char* PlatformBridge::getDebugInfoPath() 
{
    const AppInfo* info = AppManager::instance()->currentApp()->appInfo();
#if ENABLE(BOYIA_WINDOWS)    
    return getCachePath(info->appCodePath.instructPath, _CS("\\debug_info.bin"));
#else
    // For Android, Path is "/data/data/com.boyia.app/files/debug_info.bin"
    return getCachePath(info->appCodePath.instructPath, _CS("/debug_info.bin"));
#endif    
}

}