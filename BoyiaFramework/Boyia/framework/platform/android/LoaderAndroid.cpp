/*
 * LoaderAndroid.cpp
 *
 *  Created on: 2014-2-23
 *      Author: yanbo
 */

#include "LoaderAndroid.h"
#include "AutoObject.h"
#include "JNIUtil.h"
#include "SalLog.h"
#include "StringUtils.h"

namespace yanbo {

struct JLoader {
    jweak m_obj;
    jmethodID m_beginRequest;
    jmethodID m_endRequest;
    jmethodID m_putParam;
    jmethodID m_putHeader;
    util::AutoJObject object(JNIEnv* env)
    {
        return util::getRealObject(env, m_obj);
    }
};

LoaderAndroid::LoaderAndroid()
    : m_privateLoader(NULL)
{
}

LoaderAndroid::~LoaderAndroid()
{
    if (m_privateLoader != NULL) {
        JNIEnv* env = JNIUtil::getEnv();
        env->DeleteGlobalRef(m_privateLoader->m_obj);
        delete m_privateLoader;
    }
}

LVoid LoaderAndroid::initLoader()
{
    JNIEnv* env = JNIUtil::getEnv();
    jclass clazz = env->FindClass("com/boyia/app/core/ResourceLoader");
    jmethodID constructMethod = env->GetMethodID(clazz, "<init>",
        "()V");
    jobject obj = env->NewObject(clazz, constructMethod);

    m_privateLoader = new JLoader;
    m_privateLoader->m_obj = env->NewGlobalRef(obj);
    m_privateLoader->m_beginRequest = util::GetJMethod(env, clazz, "beginRequest", "(I)V");
    m_privateLoader->m_endRequest = util::GetJMethod(env, clazz, "endRequest", "(Ljava/lang/String;ZJ)V");
    m_privateLoader->m_putParam = util::GetJMethod(env, clazz, "putParam", "(Ljava/lang/String;Ljava/lang/String;)V");
    m_privateLoader->m_putHeader = util::GetJMethod(env, clazz, "putHeader", "(Ljava/lang/String;Ljava/lang/String;)V");
    env->DeleteLocalRef(obj);
    env->DeleteLocalRef(clazz);
}

LVoid LoaderAndroid::loadUrl(const String& url, NetworkClient* client, LBool isWait)
{
    request(url, client, isWait, NetworkBase::GET);
}

LVoid LoaderAndroid::loadUrl(const String& url, NetworkClient* client)
{
    loadUrl(url, client, LTrue);
}

LVoid LoaderAndroid::postData(const String& url, NetworkClient* client)
{
    postData(url, client, LTrue);
}

LVoid LoaderAndroid::postData(const String& url, NetworkClient* client, LBool isWait)
{
    request(url, client, isWait, NetworkBase::POST);
}

LVoid LoaderAndroid::request(const String& url, NetworkClient* client, LBool isWait, LInt method)
{
    JNIEnv* env = JNIUtil::getEnv();
    util::AutoJObject javaObject = m_privateLoader->object(env);

    KFORMATLOG("url=%s", (const char*)url.GetBuffer());
    if (!javaObject.get())
        return;

    jstring strUrl = util::strToJstring(env, (const char*)url.GetBuffer());
    // beginRequest
    env->CallVoidMethod(javaObject.get(), m_privateLoader->m_beginRequest, (jint)method);

    if (m_headers.size()) {
        NetworkMap::Iterator iter = m_headers.begin();
        NetworkMap::Iterator iterEnd = m_headers.end();
        for (; iter != iterEnd; ++iter) {
            const String& key = (*iter)->getKey();
            const String& value = (*iter)->getValue();
            env->CallVoidMethod(
                javaObject.get(),
                m_privateLoader->m_putHeader,
                util::strToJstring(env, (const char*)key.GetBuffer()),
                util::strToJstring(env, (const char*)value.GetBuffer()));
        }
    }

    if (m_params.size()) {
        NetworkMap::Iterator iter = m_params.begin();
        NetworkMap::Iterator iterEnd = m_params.end();
        for (; iter != iterEnd; ++iter) {
            const String& key = (*iter)->getKey();
            const String& value = (*iter)->getValue();
            env->CallVoidMethod(
                javaObject.get(),
                m_privateLoader->m_putHeader,
                util::strToJstring(env, (const char*)key.GetBuffer()),
                util::strToJstring(env, (const char*)value.GetBuffer()));
        }
    }

    // endRequest
    env->CallVoidMethod(javaObject.get(), m_privateLoader->m_endRequest,
        strUrl, isWait, (jlong)client);
    env->DeleteLocalRef(strUrl);

    m_headers.clear();
    m_params.clear();
}

LVoid LoaderAndroid::cancel()
{
}
}
