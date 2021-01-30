#include "JNIUtil.h"
#include "AutoObject.h"
#include "BoyiaLib.h"
#include "BoyiaView.h"
#include "KVector.h"
#include "SalLog.h"
#include <pthread.h>
//#include <stdarg.h>

namespace yanbo {

static pthread_key_t g_key;

JavaVM* JNIUtil::sJavaVM = nullptr;
jmethodID JNIUtil::sLoadClassMethodID = nullptr;
jobject JNIUtil::sClassLoader = nullptr;

jclass JNIUtil::getJavaClassID(const char* className)
{
    if (nullptr == className) {
        return nullptr;
    }

    JNIEnv* env = JNIUtil::getEnv();
    jstring jstrClassName = env->NewStringUTF(className);
    jclass clazz = (jclass)env->CallObjectMethod(JNIUtil::sClassLoader,
        JNIUtil::sLoadClassMethodID,
        jstrClassName);

    if (nullptr == clazz) {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(jstrClassName);
    return clazz;
}

void JNIUtil::callStaticMethod(
    const char* className,
    const char* method,
    const char* signature,
    LInt signLen,
    const char retType,
    BoyiaValue* stack,
    LInt argsLen,
    LVoid* vm,
    BoyiaValue* result)
{
    JNIEnv* env = getEnv();
    jvalue* args = NULL;
    KVector<jstring> strVector(0, 10);
    if (argsLen > 0) {
        args = new jvalue[argsLen];
        LInt idx = 0;
        for (; idx < argsLen; idx++) {
            BoyiaValue* val = stack + idx;
            if (val->mValueType == BY_CLASS) {
                //char* pStr = convertMStr2Str(&val->mValue.mStrVal);
                BoyiaStr* pStr = GetStringBuffer(val); //&val->mValue.mStrVal;
                jstring str = util::strToJstring(env, pStr->mPtr, pStr->mLen);
                args[idx].l = str;
                //delete[] pStr;
                strVector.addElement(str);
            } else if (val->mValueType == BY_INT) {
                args[idx].i = val->mValue.mIntVal;
            } else if (val->mValueType == BY_NAVCLASS) {
                boyia::BoyiaView* view = (boyia::BoyiaView*)val->mValue.mIntVal;
                args[idx].i = (LIntPtr)view->item();
            }
        }
    }
    BOYIA_LOG("JNIUtil---callStaticMethod---className=%s and retType=%c", className, retType);
    JniMethodInfo methodInfo;
    switch (retType) {
    case 'V': {

        if (getStaticMethodInfo(methodInfo, className, method, signature)) {

            methodInfo.env->CallStaticVoidMethodA(
                methodInfo.classID,
                methodInfo.methodID,
                args);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    }

    break;
    case 'S': {
        CString strSign(signature, LFalse, signLen);
        strSign += "Ljava/lang/String;";

        if (getStaticMethodInfo(methodInfo, className, method, strSign.GetBuffer())) {
            jstring str = (jstring)methodInfo.env->CallStaticObjectMethodA(
                methodInfo.classID,
                methodInfo.methodID,
                args);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);

            String strResult;
            util::jstringTostr(methodInfo.env, str, strResult);
            CreateNativeString(result, (LInt8*)strResult.GetBuffer(),
                strResult.GetLength(), vm);
            strResult.ReleaseBuffer();
        }
    } break;
    case 'I': {
        if (getStaticMethodInfo(methodInfo, className, method, signature)) {
            result->mValueType = BY_INT;
            result->mValue.mIntVal = methodInfo.env->CallStaticIntMethodA(
                methodInfo.classID,
                methodInfo.methodID,
                args);
            methodInfo.env->DeleteLocalRef(methodInfo.classID);
        }
    } break;
    }

    // 你可能不知道的localRef泄露
    LInt size = strVector.size();
    while (size) {
        env->DeleteLocalRef(strVector[--size]);
    }
}

// signature代码参数
void JNIUtil::callStaticVoidMethod(const char* className,
    const char* method,
    const char* signature,
    ...)
{
    JniMethodInfo methodInfo;
    if (getStaticMethodInfo(methodInfo, className, method, signature)) {
        va_list args;
        va_start(args, signature);
        methodInfo.env->CallStaticVoidMethodV(
            methodInfo.classID,
            methodInfo.methodID,
            args);
        va_end(args);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
}

jint JNIUtil::callStaticIntMethod(const char* className,
    const char* method,
    const char* signature,
    ...)
{
    JniMethodInfo methodInfo;
    if (getStaticMethodInfo(methodInfo, className, method, signature)) {
        va_list args;
        va_start(args, signature);
        jint result = methodInfo.env->CallStaticIntMethodV(
            methodInfo.classID,
            methodInfo.methodID,
            args);
        va_end(args);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
        return result;
    }

    return 0;
}

bool JNIUtil::callStaticBooleanMethod(const char* className,
    const char* method,
    const char* signature,
    ...)
{
    JniMethodInfo methodInfo;
    if (getStaticMethodInfo(methodInfo, className, method, signature)) {
        va_list args;
        va_start(args, signature);
        bool result = methodInfo.env->CallStaticBooleanMethodV(
            methodInfo.classID,
            methodInfo.methodID,
            args);
        va_end(args);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);

        return result;
    }

    return false;
}

jstring JNIUtil::callStaticStringMethod(const char* className,
    const char* method,
    const char* signature,
    ...)
{
    JniMethodInfo methodInfo;
    if (getStaticMethodInfo(methodInfo, className, method, signature)) {
        va_list args;
        va_start(args, signature);
        jstring str = (jstring)methodInfo.env->CallStaticObjectMethodV(
            methodInfo.classID,
            methodInfo.methodID,
            args);
        va_end(args);

        methodInfo.env->DeleteLocalRef(methodInfo.classID);
        return str;
    }

    return nullptr;
}

void JNIUtil::loadHTML(const String& url, String& stream)
{
    JNIEnv* env = getEnv();
    // 将地址转为jstring
    jstring strUrl = util::strToJstring(env, url);
    jstring text = callStaticStringMethod(
        "com/boyia/app/core/BoyiaResLoader",
        "syncLoadResource",
        "(Ljava/lang/String;)Ljava/lang/String;",
        strUrl);

    util::jstringTostr(env, text, stream);

    env->DeleteLocalRef(strUrl);
    env->DeleteLocalRef(text);
}

bool JNIUtil::getMethodInfoFromClassLoader(JniMethodInfo& methodinfo,
    const char* className,
    const char* methodName,
    const char* paramCode)
{
    if ((nullptr == className) || (nullptr == methodName) || (nullptr == paramCode)) {
        return false;
    }

    JNIEnv* env = getEnv();
    if (!env) {
        return false;
    }

    jclass classID = env->FindClass(className);
    if (!classID) {
        env->ExceptionClear();
        return false;
    }

    jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
    if (!methodID) {
        env->ExceptionClear();
        return false;
    }

    env->DeleteLocalRef(classID);

    //methodinfo.classID = classID;
    methodinfo.env = env;
    methodinfo.methodID = methodID;

    return true;
}

bool JNIUtil::setClassLoaderFrom(jobject activityInstance)
{
    JniMethodInfo getclassloaderMethod;
    if (!getMethodInfoFromClassLoader(getclassloaderMethod,
            "android/content/Context",
            "getClassLoader",
            "()Ljava/lang/ClassLoader;")) {
        return false;
    }

    jobject clzz = getEnv()->CallObjectMethod(activityInstance,
        getclassloaderMethod.methodID);

    if (nullptr == clzz) {
        return false;
    }

    JniMethodInfo methodInfo;
    if (!getMethodInfoFromClassLoader(methodInfo,
            "java/lang/ClassLoader",
            "loadClass",
            "(Ljava/lang/String;)Ljava/lang/Class;")) {
        return false;
    }

    sClassLoader = getEnv()->NewGlobalRef(clzz);
    sLoadClassMethodID = methodInfo.methodID;
    return true;
}

JNIEnv* JNIUtil::cacheEnv(JavaVM* jvm)
{
    JNIEnv* env = nullptr;
    // get jni environment
    jint ret = jvm->GetEnv((void**)&env, JNI_VERSION_1_4);

    switch (ret) {
    case JNI_OK:
        // Success!
        pthread_setspecific(g_key, env);
        return env;

    case JNI_EDETACHED:
        // Thread not attached

        // TODO : If calling AttachCurrentThread() on a native thread
        // must call DetachCurrentThread() in future.
        // see: http://developer.android.com/guide/practices/design/jni.html

        if (jvm->AttachCurrentThread(&env, nullptr) < 0) {
            return nullptr;
        } else {
            pthread_setspecific(g_key, env);
            return env;
        }

    case JNI_EVERSION:
    default:
        return nullptr;
    }
}

JNIEnv* JNIUtil::getEnv()
{
    JNIEnv* env = (JNIEnv*)pthread_getspecific(g_key);
    if (env == nullptr)
        env = cacheEnv(sJavaVM);
    return env;
}

bool JNIUtil::getStaticMethodInfo(JniMethodInfo& methodinfo,
    const char* className,
    const char* methodName,
    const char* paramCode)
{
    if ((nullptr == className) || (nullptr == methodName) || (nullptr == paramCode)) {
        return false;
    }

    JNIEnv* env = getEnv();
    if (!env) {
        return false;
    }

    jclass classID = getJavaClassID(className);
    if (!classID) {
        env->ExceptionClear();
        return false;
    }

    jmethodID methodID = env->GetStaticMethodID(classID, methodName, paramCode);
    if (!methodID) {
        env->ExceptionClear();
        return false;
    }

    methodinfo.classID = classID;
    methodinfo.env = env;
    methodinfo.methodID = methodID;
    return true;
}

bool JNIUtil::getMethodInfo(JniMethodInfo& methodinfo,
    const char* className,
    const char* methodName,
    const char* paramCode)
{
    if ((nullptr == className) || (nullptr == methodName) || (nullptr == paramCode)) {
        return false;
    }

    JNIEnv* env = getEnv();
    if (!env) {
        return false;
    }

    jclass classID = getJavaClassID(className);
    if (!classID) {
        env->ExceptionClear();
        return false;
    }

    jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
    if (!methodID) {
        env->ExceptionClear();
        return false;
    }

    methodinfo.classID = classID;
    methodinfo.env = env;
    methodinfo.methodID = methodID;

    return true;
}

JavaVM* JNIUtil::getJavaVM()
{
    pthread_t thisthread = pthread_self();
    return sJavaVM;
}

void JNIUtil::setJavaVM(JavaVM* javaVM)
{
    pthread_t thisthread = pthread_self();
    sJavaVM = javaVM;

    pthread_key_create(&g_key, nullptr);
}
}
