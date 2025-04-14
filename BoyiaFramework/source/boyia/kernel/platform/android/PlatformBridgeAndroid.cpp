#include "AutoObject.h"
#include "JNIUtil.h"
#include "PlatformBridge.h"
#include "SalLog.h"
#include "ZipEntry.h"

#if ENABLE(BOYIA_ANDROID)
#include "AppManager.h"
#include "FileUtil.h"

namespace yanbo {
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
    if (!sAppRootPath.GetLength()) {
        JNIEnv* env = JNIUtil::getEnv();
        jstring path = JNIUtil::callStaticStringMethod(
            "com/boyia/app/core/BoyiaBridge",
            "getAppRoot",
            "()Ljava/lang/String;");

        util::jstringTostr(env, path, sAppRootPath);
        env->DeleteLocalRef(path);
    }

    BOYIA_LOG("PlatformBridge---getAppRoot: %s", GET_STR(sAppRootPath));
    return GET_STR(sAppRootPath);
    //"/data/data/com.boyia.app/files/";
}

const char* PlatformBridge::getBoyiaJsonUrl()
{
    return "boyia_assets://boyia.json";
    //return "https://damonyan1985.github.io/app/boyia.json";
}

const LInt PlatformBridge::getTextSize(const String& text)
{
    JNIEnv* env = JNIUtil::getEnv();
    jstring jtext = util::strToJstring(env, text);
    LInt result = JNIUtil::callStaticIntMethod(
        "com/boyia/app/core/BoyiaBridge",
        "getTextSize",
        "(Ljava/lang/String;)I",
        jtext);
    env->DeleteLocalRef(jtext);
    return result;
}

const LReal PlatformBridge::getDisplayDensity()
{
    return JNIUtil::callStaticFloatMethod(
        "com/boyia/app/core/BoyiaBridge",
        "getDisplayDensity",
        "()F");
}

PlatformBridge::PlatformType PlatformBridge::getPlatformType()
{
    return kPlatformAndroid;
}

void PlatformBridge::handleApi(const String& params, LIntPtr callback)
{    
}

bool PlatformBridge::isAbsolutePath(const String& path)
{
    return path.GetLength() > 0 && path.StartWith(_CS("/"));
}
}

#endif