#include "MediaPlayerAndroid.h"
#include "AutoObject.h"
#include "JNIUtil.h"
#include "UIThread.h"
#include "VideoView.h"
// #include <GLES2/gl2.h>
// #include <GLES2/gl2ext.h>
#include <jni.h>

namespace util {

struct JMediaPlayer {
    jweak m_obj;
    jmethodID m_start;
    jmethodID m_pause;
    jmethodID m_stop;
    jmethodID m_seek;
    jmethodID m_setNativePtr;
    jmethodID m_getPlayerId;
    AutoJObject object(JNIEnv* env)
    {
        return getRealObject(env, m_obj);
    }
};

MediaPlayerAndroid::MediaPlayerAndroid(LVoid* view)
    : m_view(view)
{
    JNIEnv* env = JNIUtil::getEnv();
    jclass clazz = JNIUtil::getJavaClassID("com/boyia/app/core/BoyiaPlayer");
    jmethodID constructMethod = env->GetMethodID(clazz, "<init>",
        "()V");
    jobject obj = env->NewObject(clazz, constructMethod);

    m_player = new JMediaPlayer;
    m_player->m_obj = env->NewGlobalRef(obj);
    m_player->m_start = GetJMethod(env, clazz, "start", "(Ljava/lang/String;)V");
    m_player->m_pause = GetJMethod(env, clazz, "pause", "()V");
    m_player->m_stop = GetJMethod(env, clazz, "stop", "()V");
    m_player->m_seek = GetJMethod(env, clazz, "seek", "(I)V");
    m_player->m_setNativePtr = GetJMethod(env, clazz, "setNativePtr", "(J)V");
    m_player->m_getPlayerId = GetJMethod(env, clazz, "getPlayerId", "()J");

    // env->CallVoidMethod(obj, m_player->m_setNativePtr,
    //     (jlong)view);
    // 视频相关回调信息，比如进度，缓冲，开始，结束
    yanbo::UIThreadClientMap::instance()->registerClient(this);
    env->CallVoidMethod(obj, m_player->m_setNativePtr,
        (jlong)getClientId());

    // 获取textureId；
    LIntPtr playId = env->CallLongMethod(obj, m_player->m_getPlayerId);
    setPlayerId(playId);

    env->DeleteLocalRef(obj);
    env->DeleteLocalRef(clazz);
}

MediaPlayerAndroid::~MediaPlayerAndroid()
{
    JNIUtil::getEnv()->DeleteGlobalRef(m_player->m_obj);
    delete m_player;
}

LVoid MediaPlayerAndroid::start(const String& url)
{
    JNIEnv* env = JNIUtil::getEnv();
    AutoJObject javaObject = m_player->object(env);
    if (!javaObject.get())
        return;

    jstring strPath = strToJstring(env, url);
    BOYIA_LOG("MediaPlayerAndroid::start path=%s", (const char*)url.GetBuffer());
    env->CallVoidMethod(javaObject.get(), m_player->m_start,
        strPath);

    env->DeleteLocalRef(strPath);
}

LVoid MediaPlayerAndroid::seek(int progress)
{
    JNIEnv* env = JNIUtil::getEnv();
    AutoJObject javaObject = m_player->object(env);
    if (!javaObject.get())
        return;

    env->CallVoidMethod(javaObject.get(), m_player->m_seek,
        progress);
}

LVoid MediaPlayerAndroid::pause()
{
    JNIEnv* env = JNIUtil::getEnv();
    AutoJObject javaObject = m_player->object(env);
    if (!javaObject.get())
        return;

    env->CallVoidMethod(javaObject.get(), m_player->m_pause);
}

LVoid MediaPlayerAndroid::stop()
{
    JNIEnv* env = JNIUtil::getEnv();
    AutoJObject javaObject = m_player->object(env);
    if (!javaObject.get())
        return;

    env->CallVoidMethod(javaObject.get(), m_player->m_stop);
}

// Now in ui thread, can use ui thread method to paint
LVoid MediaPlayerAndroid::onClientCallback()
{
    yanbo::UIThread::instance()->drawUI(m_view);
}

LMediaPlayer* LMediaPlayer::create(LVoid* view)
{
    return new MediaPlayerAndroid(view);
}
}
