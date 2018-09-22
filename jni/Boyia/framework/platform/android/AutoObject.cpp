#include "AutoObject.h"
#include "JNIUtil.h"
namespace util
{
AutoJObject getRealObject(JNIEnv* env, jobject obj)
{
	jobject real = obj;
	if (obj)
	{
		real = env->NewLocalRef(obj);
	}

    return AutoJObject(env, real);
}

jmethodID GetJMethod(JNIEnv* env, jclass clazz, const char name[], const char signature[])
{
    jmethodID m = env->GetMethodID(clazz, name, signature);
    return m;
}

// C字符串转java字符串
jstring strToJstring(JNIEnv* env, const char* s)
{
    int        strLen    = strlen(s);
    jclass     jstrObj   = yanbo::JNIUtil::getJavaClassID("java/lang/String");//env->FindClass("java/lang/String");
    jmethodID  methodId  = env->GetMethodID(jstrObj, "<init>", "([BLjava/lang/String;)V");
    jbyteArray byteArray = env->NewByteArray(strLen);
    jstring    encode    = env->NewStringUTF("utf-8");

    env->SetByteArrayRegion(byteArray, 0, strLen, (jbyte*)s);

    jstring str = (jstring)(env->NewObject(jstrObj, methodId, byteArray, encode));

    env->DeleteLocalRef(byteArray);
    env->DeleteLocalRef(encode);
    env->DeleteLocalRef(jstrObj);
    return str;
}

LVoid jstringTostr(JNIEnv* env, jstring jstr, String& result)
{
    char* pStr = NULL;

    jclass     jstrObj   = yanbo::JNIUtil::getJavaClassID("java/lang/String");//env->FindClass("java/lang/String");
    jstring    encode    = env->NewStringUTF("utf-8");
    jmethodID  methodId  = env->GetMethodID(jstrObj, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray byteArray = (jbyteArray)env->CallObjectMethod(jstr, methodId, encode);
    jsize      strLen    = env->GetArrayLength(byteArray);
    jbyte      *jBuf     = env->GetByteArrayElements(byteArray, JNI_FALSE);

    if (jBuf > 0)
    {
        pStr = new char[strLen + 1];//(char*)malloc(strLen + 1);
        if (!pStr)
        {
            return;
        }

        memcpy(pStr, jBuf, strLen);
        pStr[strLen] = 0;
    }

    env->ReleaseByteArrayElements(byteArray, jBuf, 0);
    env->DeleteLocalRef(encode);
    env->DeleteLocalRef(jstrObj);
    // 接受字符串
    result.Copy(_CS(pStr), LTrue, strLen);
}

}
