#include "AutoObject.h"
#include "JNIUtil.h"
#include "PlatformBridge.h"
#include "SalLog.h"
#include "ZipEntry.h"

#if ENABLE(BOYIA_ANDROID)
namespace yanbo {
const char* kZipPassword = "123456";
bool PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    BOYIA_LOG("PlatformBridge---unzip---src: %s, dest: %s", GET_STR(zipFile), GET_STR(dir));

    if (dir.EndWith(_CS("sdk"))) {
        return ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kBoyiaNull);
    }

    return ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kZipPassword);

    // JNIEnv* env = JNIUtil::getEnv();
    // jstring jpath = util::strToJstring(env, GET_STR(zipFile));
    // jstring jdir = util::strToJstring(env, GET_STR(dir));

    // KFORMATLOG("boyia app AppHandler unzip path=%s", GET_STR(zipFile));
    // KFORMATLOG("boyia app AppHandler unzip dir=%s", GET_STR(dir));

    // bool result = JNIUtil::callStaticBooleanMethod(
    //     "com/boyia/app/common/utils/ZipOperation",
    //     "unZipFile",
    //     "(Ljava/lang/String;Ljava/lang/String;)Z",
    //     jpath, jdir);

    // env->DeleteLocalRef(jpath);
    // env->DeleteLocalRef(jdir);

    // BOYIA_LOG("PlatformBridge---unzip---result=%d", (LInt)result);
    // return result;
}

const char* PlatformBridge::getAppPath()
{
    return "/data/data/com.boyia.app/files/apps/";
}

const char* PlatformBridge::getBoyiaJsonPath()
{
    return "/data/data/com.boyia.app/files/boyia.json";
}

const char* PlatformBridge::getSdkPath()
{
    return "/data/data/com.boyia.app/files/sdk/";
}

const char* PlatformBridge::getAppRoot()
{
    return "/data/data/com.boyia.app/files/";
}

const char* PlatformBridge::getInstructionCachePath()
{
    return "/data/data/com.boyia.app/files/instruction_cache.bin";
}

const char* PlatformBridge::getStringTableCachePath()
{
    return "/data/data/com.boyia.app/files/stringtable_cache.bin";
}

const char* PlatformBridge::getInstructionEntryPath()
{
    return "/data/data/com.boyia.app/files/instruction_entry.bin";
}

const char* PlatformBridge::getBoyiaJsonUrl()
{
    return "boyia_assets://boyia.json";
    //return "https://damonyan1985.github.io/app/boyia.json";
}

const LInt PlatformBridge::getTextSize(const String& text)
{
    JNIEnv* env = JNIUtil::getEnv();
    jstring jtext = util::strToJstring(env, GET_STR(text));
    LInt result = JNIUtil::callStaticIntMethod(
        "com/boyia/app/core/BoyiaBridge",
        "getTextSize",
        "(Ljava/lang/String;)I",
        jtext);
    env->DeleteLocalRef(jtext);
    return result;
}
}

#endif