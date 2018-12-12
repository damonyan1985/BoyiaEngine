#ifndef AutoObject_h
#define AutoObject_h

#include "UtilString.h"
#include <jni.h>
#include <string.h>

namespace util
{
class AutoJObject {
public:
    AutoJObject(const AutoJObject& other)
        : m_env(other.m_env)
        , m_obj(other.m_obj ? other.m_env->NewLocalRef(other.m_obj) : NULL) {}
    ~AutoJObject() {
        if (m_obj)
            m_env->DeleteLocalRef(m_obj);
    }
    jobject get() const {
        return m_obj;
    }
    // Releases the local reference to the caller. The caller *must* delete the
    // local reference when it is done with it.
    jobject release() {
        jobject obj = m_obj;
        m_obj = 0;
        return obj;
    }
    JNIEnv* env() const {
        return m_env;
    }
private:
    AutoJObject(); // Not permitted.
    AutoJObject(JNIEnv* env, jobject obj)
        : m_env(env)
        , m_obj(obj) {}
    JNIEnv* m_env;
    jobject m_obj;
    friend AutoJObject getRealObject(JNIEnv*, jobject);
};

AutoJObject getRealObject(JNIEnv* env, jobject obj);

jmethodID GetJMethod(JNIEnv* env, jclass clazz, const char name[], const char signature[]);

jstring strToJstring(JNIEnv* env, const char* s);

LVoid jstringTostr(JNIEnv* env, jstring jstr, String& result);
}

#endif
