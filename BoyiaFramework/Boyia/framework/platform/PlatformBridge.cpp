#include "PlatformBridge.h"
#include "AutoObject.h"
#include "JNIUtil.h"

namespace yanbo {

#if ENABLE(ANDROID_PORTING)
LVoid PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    JNIEnv* env = JNIUtil::getEnv();
    jstring jpath = util::strToJstring(env, GET_STR(zipFile));
    jstring jdir = util::strToJstring(env, GET_STR(dir));

    KFORMATLOG("boyia app AppHandler unzip path=%s", GET_STR(zipFile));
    KFORMATLOG("boyia app AppHandler unzip dir=%s", GET_STR(dir));

    JNIUtil::callStaticVoidMethod(
        "com/boyia/app/common/utils/ZipOperation",
        "unZipFile",
        "(Ljava/lang/String;Ljava/lang/String;)V",
        jpath, jdir);

    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(jdir);
}

const char* PlatformBridge::getAppPath()
{
    return "/data/data/com.boyia.app/files/apps/";
}

const char* PlatformBridge::getAppJsonPath()
{
    return "/data/data/com.boyia.app/files/apps/boyia.json";
}

#endif
}