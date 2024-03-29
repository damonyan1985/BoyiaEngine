#ifndef JNIUtil_h
#define JNIUtil_h

#include "BoyiaCore.h"
#include "UtilString.h"
#include <jni.h>

namespace yanbo {

typedef struct JniMethodInfo {
    JNIEnv* env;
    jclass classID;
    jmethodID methodID;
} JniMethodInfo;

class JNIUtil {
public:
    static void callStaticMethod(
        const char* className,
        const char* method,
        const char* sign,
        LInt signLen,
        const char retType,
        BoyiaValue* args,
        LInt argsLen,
        LVoid* vm,
        BoyiaValue* result);

    static void callStaticVoidMethod(const char* className,
        const char* method,
        const char* signature,
        ...);

    static bool callStaticBooleanMethod(const char* className,
        const char* method,
        const char* signature,
        ...);

    static jstring callStaticStringMethod(const char* className,
        const char* method,
        const char* signature,
        ...);

    static jint callStaticIntMethod(const char* className,
        const char* method,
        const char* signature,
        ...);

    static jfloat callStaticFloatMethod(const char* className,
        const char* method,
        const char* signature,
        ...);

    static jobject callStaticObjectMethod(const char* className,
        const char* method,
        const char* signature,
        ...);

    static void loadHTML(const String& url, String& stream);

    static void setJavaVM(JavaVM* javaVM);
    static JavaVM* getJavaVM();
    static JNIEnv* getEnv();
    static bool setClassLoaderFrom(jobject activityInstance);

    static bool getStaticMethodInfo(JniMethodInfo& methodinfo,
        const char* className,
        const char* methodName,
        const char* paramCode);
    static bool getMethodInfo(JniMethodInfo& methodinfo,
        const char* className,
        const char* methodName,
        const char* paramCode);

    static jclass getJavaClassID(const char* className);

private:
    static bool getMethodInfoFromClassLoader(JniMethodInfo& methodinfo,
        const char* className,
        const char* methodName,
        const char* paramCode);

    static JNIEnv* cacheEnv(JavaVM* jvm);

    static jmethodID sLoadClassMethodID;
    static jobject sClassLoader;
    static JavaVM* sJavaVM;
};
}

using yanbo::JNIUtil;
#endif
