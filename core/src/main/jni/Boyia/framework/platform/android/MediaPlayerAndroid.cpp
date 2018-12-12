#include "MediaPlayerAndroid.h"
#include "JNIUtil.h"
#include "AutoObject.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>

namespace util
{
struct JMediaPlayer
{
	jweak       m_obj;
	jmethodID   m_start;
	jmethodID   m_pause;
	jmethodID   m_stop;
	jmethodID   m_seek;
	//jmethodID   m_texId;
	jmethodID   m_setNativePtr;
	jmethodID   m_updateTexture;
	jmethodID   m_setTextureId;
	jmethodID   m_canDraw;
	AutoJObject object(JNIEnv* env) {
	    return getRealObject(env, m_obj);
	}
};

MediaPlayerAndroid::MediaPlayerAndroid(LVoid* view)
    : m_view(view)
    , m_texID(0)
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
	//m_player->m_texId = GetJMethod(env, clazz, "getTextureId", "()I");
	m_player->m_setNativePtr = GetJMethod(env, clazz, "setNativePtr", "(J)V");
	m_player->m_updateTexture = GetJMethod(env, clazz, "updateTexture", "()[F");
	m_player->m_setTextureId = GetJMethod(env, clazz, "setTextureId", "(I)V");
	m_player->m_canDraw = GetJMethod(env, clazz, "canDraw", "()Z");


	env->CallVoidMethod(obj, m_player->m_setNativePtr,
		    		(jlong)view);

	env->DeleteLocalRef(obj);
	env->DeleteLocalRef(clazz);

}

bool MediaPlayerAndroid::canDraw()
{
	JNIEnv* env = JNIUtil::getEnv();
	AutoJObject javaObject = m_player->object(env);
	if (!javaObject.get())
		return false;

	return env->CallBooleanMethod(javaObject.get(), m_player->m_canDraw);
}

void MediaPlayerAndroid::createTextureId()
{
	glGenTextures(1, &m_texID);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_texID);

	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 将textureID传给java层
	JNIEnv* env = JNIUtil::getEnv();
    AutoJObject javaObject = m_player->object(env);
	if (!javaObject.get())
		return;

	env->CallVoidMethod(javaObject.get(), m_player->m_setTextureId, m_texID);
}

MediaPlayerAndroid::~MediaPlayerAndroid()
{
	JNIUtil::getEnv()->DeleteGlobalRef(m_player->m_obj);
	delete m_player;
}

void MediaPlayerAndroid::start(const String& url)
{
	if (!m_texID)
	{
		createTextureId();
	}
	JNIEnv* env = JNIUtil::getEnv();
	AutoJObject javaObject = m_player->object(env);
	if (!javaObject.get())
	    return;

	jstring strPath = strToJstring(env, (const char*)url.GetBuffer());
	KFORMATLOG("MediaPlayerAndroid::start path=%s", (const char*)url.GetBuffer());
    env->CallVoidMethod(javaObject.get(), m_player->m_start,
	    		strPath);

	env->DeleteLocalRef(strPath);
}

void MediaPlayerAndroid::updateTexture(float* matrix)
{
	JNIEnv* env = JNIUtil::getEnv();
	AutoJObject javaObject = m_player->object(env);
	if (!javaObject.get())
		return;
	if (!matrix)
		return;

	jfloatArray arr = (jfloatArray) env->CallObjectMethod(javaObject.get(), m_player->m_updateTexture);

	int count = env->GetArrayLength(arr);
	jfloat* ptr = env->GetFloatArrayElements(arr, JNI_FALSE);

	LMemcpy(matrix, ptr, sizeof(float) * count);
    env->ReleaseFloatArrayElements(arr, ptr, 0);
    env->DeleteLocalRef(arr);
}

void MediaPlayerAndroid::seek(int progress)
{
	JNIEnv* env = JNIUtil::getEnv();
	AutoJObject javaObject = m_player->object(env);
	if (!javaObject.get())
		return;

	env->CallVoidMethod(javaObject.get(), m_player->m_seek,
			progress);
}

void MediaPlayerAndroid::pause()
{
	JNIEnv* env = JNIUtil::getEnv();
    AutoJObject javaObject = m_player->object(env);
	if (!javaObject.get())
		return;

	env->CallVoidMethod(javaObject.get(), m_player->m_pause);
}

void MediaPlayerAndroid::stop()
{
	JNIEnv* env = JNIUtil::getEnv();
    AutoJObject javaObject = m_player->object(env);
	if (!javaObject.get())
		return;

	env->CallVoidMethod(javaObject.get(), m_player->m_stop);
}

int MediaPlayerAndroid::texId()
{
	return m_texID;
}

LMediaPlayer* LMediaPlayer::create(LVoid* view)
{
    return new MediaPlayerAndroid(view);
}

}
