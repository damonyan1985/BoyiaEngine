#include "PlatformBridge.h"
#include "AutoObject.h"
#include "JNIUtil.h"
#include "SalLog.h"

namespace yanbo {

#if ENABLE(BOYIA_ANDROID)
bool PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    JNIEnv* env = JNIUtil::getEnv();
    jstring jpath = util::strToJstring(env, GET_STR(zipFile));
    jstring jdir = util::strToJstring(env, GET_STR(dir));

    KFORMATLOG("boyia app AppHandler unzip path=%s", GET_STR(zipFile));
    KFORMATLOG("boyia app AppHandler unzip dir=%s", GET_STR(dir));

    bool result = JNIUtil::callStaticBooleanMethod(
        "com/boyia/app/common/utils/ZipOperation",
        "unZipFile",
        "(Ljava/lang/String;Ljava/lang/String;)Z",
        jpath, jdir);

    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(jdir);

    BOYIA_LOG("PlatformBridge---unzip---result=%d", (LInt)result);
    return result;
}

const char* PlatformBridge::getAppPath()
{
    return "/data/data/com.boyia.app/files/apps/";
}

const char* PlatformBridge::getAppJsonPath()
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

const char* PlatformBridge::getBoyiaJsonUrl()
{
    return "https://damonyan1985.github.io/app/boyia.json";
}

#endif
}